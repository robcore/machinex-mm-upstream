/* Copyright (c) 2007 Coraid, Inc.  See COPYING for GPL terms. */
/*
 * aoecmd.c
 * Filesystem request handling methods
 */

#include <linux/ata.h>
#include <linux/slab.h>
#include <linux/hdreg.h>
#include <linux/blkdev.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/genhd.h>
#include <linux/moduleparam.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <net/net_namespace.h>
#include <asm/unaligned.h>
#include <linux/uio.h>
#include "aoe.h"

#define MAXIOC (8192)	/* default meant to avoid most soft lockups */

static void ktcomplete(struct frame *, struct sk_buff *);

static int aoe_deadsecs = 60 * 3;
module_param(aoe_deadsecs, int, 0644);
MODULE_PARM_DESC(aoe_deadsecs, "After aoe_deadsecs seconds, give up and fail dev.");

static int aoe_maxout = 16;
module_param(aoe_maxout, int, 0644);
MODULE_PARM_DESC(aoe_maxout,
	"Only aoe_maxout outstanding packets for every MAC on eX.Y.");

static wait_queue_head_t ktiowq;
static struct ktstate kts;

/* io completion queue */
static struct {
	struct list_head head;
	spinlock_t lock;
} iocq;

static struct sk_buff *
new_skb(ulong len)
{
	struct sk_buff *skb;

	skb = alloc_skb(len + MAX_HEADER, GFP_ATOMIC);
	if (skb) {
		skb_reserve(skb, MAX_HEADER);
		skb_reset_mac_header(skb);
		skb_reset_network_header(skb);
		skb->protocol = __constant_htons(ETH_P_AOE);
		skb_checksum_none_assert(skb);
	}
	return skb;
}

static struct frame *
getframe(struct aoetgt *t, u32 tag)
{
	struct frame *f;
	struct list_head *head, *pos, *nx;
	u32 n;

	n = tag % NFACTIVE;
	head = &t->factive[n];
	list_for_each_safe(pos, nx, head) {
		f = list_entry(pos, struct frame, head);
		if (f->tag == tag) {
			list_del(pos);
			return f;
		}
	}
	return NULL;
}

/*
 * Leave the top bit clear so we have tagspace for userland.
 * The bottom 16 bits are the xmit tick for rexmit/rttavg processing.
 * This driver reserves tag -1 to mean "unused frame."
 */
static int
newtag(struct aoetgt *t)
{
	register ulong n;

	n = jiffies & 0xffff;
	return n |= (++t->lasttag & 0x7fff) << 16;
}

static u32
aoehdr_atainit(struct aoedev *d, struct aoetgt *t, struct aoe_hdr *h)
{
	u32 host_tag = newtag(t);

	memcpy(h->src, t->ifp->nd->dev_addr, sizeof h->src);
	memcpy(h->dst, t->addr, sizeof h->dst);
	h->type = __constant_cpu_to_be16(ETH_P_AOE);
	h->verfl = AOE_HVER;
	h->major = cpu_to_be16(d->aoemajor);
	h->minor = d->aoeminor;
	h->cmd = AOECMD_ATA;
	h->tag = cpu_to_be32(host_tag);

	return host_tag;
}

static inline void
put_lba(struct aoe_atahdr *ah, sector_t lba)
{
	ah->lba0 = lba;
	ah->lba1 = lba >>= 8;
	ah->lba2 = lba >>= 8;
	ah->lba3 = lba >>= 8;
	ah->lba4 = lba >>= 8;
	ah->lba5 = lba >>= 8;
}

static void
ifrotate(struct aoetgt *t)
{
	t->ifp++;
	if (t->ifp >= &t->ifs[NAOEIFS] || t->ifp->nd == NULL)
		t->ifp = t->ifs;
	if (t->ifp->nd == NULL) {
		printk(KERN_INFO "aoe: no interface to rotate to\n");
		BUG();
	}
}

static void
skb_pool_put(struct aoedev *d, struct sk_buff *skb)
{
	__skb_queue_tail(&d->skbpool, skb);
}

static struct sk_buff *
skb_pool_get(struct aoedev *d)
{
	struct sk_buff *skb = skb_peek(&d->skbpool);

	if (skb && atomic_read(&skb_shinfo(skb)->dataref) == 1) {
		__skb_unlink(skb, &d->skbpool);
		return skb;
	}
	if (skb_queue_len(&d->skbpool) < NSKBPOOLMAX &&
	    (skb = new_skb(ETH_ZLEN)))
		return skb;

	return NULL;
}

void
aoe_freetframe(struct frame *f)
{
	struct aoetgt *t;

	t = f->t;
	f->buf = NULL;
	f->bv = NULL;
	f->r_skb = NULL;
	list_add(&f->head, &t->ffree);
}

static struct frame *
newtframe(struct aoedev *d, struct aoetgt *t)
{
	struct frame *f;
	struct sk_buff *skb;
	struct list_head *pos;

	if (list_empty(&t->ffree)) {
		if (t->falloc >= NSKBPOOLMAX*2)
			return NULL;
		f = kcalloc(1, sizeof(*f), GFP_ATOMIC);
		if (f == NULL)
			return NULL;
		t->falloc++;
		f->t = t;
	} else {
		pos = t->ffree.next;
		list_del(pos);
		f = list_entry(pos, struct frame, head);
	}

	skb = f->skb;
	if (skb == NULL) {
		f->skb = skb = new_skb(ETH_ZLEN);
		if (!skb) {
bail:			aoe_freetframe(f);
			return NULL;
		}
	}

	if (atomic_read(&skb_shinfo(skb)->dataref) != 1) {
		skb = skb_pool_get(d);
		if (skb == NULL)
			goto bail;
		skb_pool_put(d, f->skb);
		f->skb = skb;
	}

	skb->truesize -= skb->data_len;
	skb_shinfo(skb)->nr_frags = skb->data_len = 0;
	skb_trim(skb, 0);
	return f;
}

static struct frame *
newframe(struct aoedev *d)
{
	struct frame *f;
	struct aoetgt *t, **tt;
	int totout = 0;

	if (d->targets[0] == NULL) {	/* shouldn't happen, but I'm paranoid */
		printk(KERN_ERR "aoe: NULL TARGETS!\n");
		return NULL;
	}
	tt = d->tgt;	/* last used target */
	for (;;) {
		tt++;
		if (tt >= &d->targets[NTARGETS] || !*tt)
			tt = d->targets;
		t = *tt;
		totout += t->nout;
		if (t->nout < t->maxout
		&& t != d->htgt
		&& t->ifp->nd) {
			f = newtframe(d, t);
			if (f) {
				d->tgt = tt;
				ifrotate(t);
				return f;
			}
		}
		if (tt == d->tgt)	/* we've looped and found nada */
			break;
	}
	if (totout == 0) {
		d->kicked++;
		d->flags |= DEVFL_KICKME;
	}
	return NULL;
}

static void
skb_fillup(struct sk_buff *skb, struct bio_vec *bv, ulong off, ulong cnt)
{
	int frag = 0;
	ulong fcnt;
loop:
	fcnt = bv->bv_len - (off - bv->bv_offset);
	if (fcnt > cnt)
		fcnt = cnt;
	skb_fill_page_desc(skb, frag++, bv->bv_page, off, fcnt);
	cnt -= fcnt;
	if (cnt <= 0)
		return;
	bv++;
	off = bv->bv_offset;
	goto loop;
}

static void
fhash(struct frame *f)
{
	struct aoetgt *t = f->t;
	u32 n;

	n = f->tag % NFACTIVE;
	list_add_tail(&f->head, &t->factive[n]);
}

static int
aoecmd_ata_rw(struct aoedev *d)
{
	struct frame *f;
	struct aoe_hdr *h;
	struct aoe_atahdr *ah;
	struct buf *buf;
	struct bio_vec *bv;
	struct aoetgt *t;
	struct sk_buff *skb;
	ulong bcnt, fbcnt;
	char writebit, extbit;

	writebit = 0x10;
	extbit = 0x4;

	f = newframe(d);
	if (f == NULL)
		return 0;
	t = *d->tgt;
	buf = d->inprocess;
	bv = buf->bv;
	bcnt = t->ifp->maxbcnt;
	if (bcnt == 0)
		bcnt = DEFAULTBCNT;
	if (bcnt > buf->resid)
		bcnt = buf->resid;
	fbcnt = bcnt;
	f->bv = buf->bv;
	f->bv_off = f->bv->bv_offset + (f->bv->bv_len - buf->bv_resid);
	do {
		if (fbcnt < buf->bv_resid) {
			buf->bv_resid -= fbcnt;
			buf->resid -= fbcnt;
			break;
		}
		fbcnt -= buf->bv_resid;
		buf->resid -= buf->bv_resid;
		if (buf->resid == 0) {
			d->inprocess = NULL;
			break;
		}
		buf->bv++;
		buf->bv_resid = buf->bv->bv_len;
		WARN_ON(buf->bv_resid == 0);
	} while (fbcnt);

	/* initialize the headers & frame */
	skb = f->skb;
	h = (struct aoe_hdr *) skb_mac_header(skb);
	ah = (struct aoe_atahdr *) (h+1);
	skb_put(skb, sizeof *h + sizeof *ah);
	memset(h, 0, skb->len);
	f->tag = aoehdr_atainit(d, t, h);
	fhash(f);
	t->nout++;
	f->waited = 0;
	f->buf = buf;
	f->bcnt = bcnt;
	f->lba = buf->sector;

	/* set up ata header */
	ah->scnt = bcnt >> 9;
	put_lba(ah, buf->sector);
	if (d->flags & DEVFL_EXT) {
		ah->aflags |= AOEAFL_EXT;
	} else {
		extbit = 0;
		ah->lba3 &= 0x0f;
		ah->lba3 |= 0xe0;	/* LBA bit + obsolete 0xa0 */
	}
	if (bio_data_dir(buf->bio) == WRITE) {
		skb_fillup(skb, f->bv, f->bv_off, bcnt);
		ah->aflags |= AOEAFL_WRITE;
		skb->len += bcnt;
		skb->data_len = bcnt;
		skb->truesize += bcnt;
		t->wpkts++;
	} else {
		t->rpkts++;
		writebit = 0;
	}

	ah->cmdstat = ATA_CMD_PIO_READ | writebit | extbit;

	/* mark all tracking fields and load out */
	buf->nframesout += 1;
	buf->sector += bcnt >> 9;

	skb->dev = t->ifp->nd;
	skb = skb_clone(skb, GFP_ATOMIC);
	if (skb)
		__skb_queue_tail(&d->sendq, skb);
	return 1;
}

/* some callers cannot sleep, and they can call this function,
 * transmitting the packets later, when interrupts are on
 */
static void
aoecmd_cfg_pkts(ushort aoemajor, unsigned char aoeminor, struct sk_buff_head *queue)
{
	struct aoe_hdr *h;
	struct aoe_cfghdr *ch;
	struct sk_buff *skb;
	struct net_device *ifp;

	rcu_read_lock();
	for_each_netdev_rcu(&init_net, ifp) {
		dev_hold(ifp);
		if (!is_aoe_netif(ifp))
			goto cont;

		skb = new_skb(sizeof *h + sizeof *ch);
		if (skb == NULL) {
			printk(KERN_INFO "aoe: skb alloc failure\n");
			goto cont;
		}
		skb_put(skb, sizeof *h + sizeof *ch);
		skb->dev = ifp;
		__skb_queue_tail(queue, skb);
		h = (struct aoe_hdr *) skb_mac_header(skb);
		memset(h, 0, sizeof *h + sizeof *ch);

		memset(h->dst, 0xff, sizeof h->dst);
		memcpy(h->src, ifp->dev_addr, sizeof h->src);
		h->type = __constant_cpu_to_be16(ETH_P_AOE);
		h->verfl = AOE_HVER;
		h->major = cpu_to_be16(aoemajor);
		h->minor = aoeminor;
		h->cmd = AOECMD_CFG;

cont:
		dev_put(ifp);
	}
	rcu_read_unlock();
}

static void
resend(struct aoedev *d, struct frame *f)
{
	struct sk_buff *skb;
	struct aoe_hdr *h;
	struct aoe_atahdr *ah;
	struct aoetgt *t;
	char buf[128];
	u32 n;

	t = f->t;
	ifrotate(t);
	n = newtag(t);
	skb = f->skb;
	h = (struct aoe_hdr *) skb_mac_header(skb);
	ah = (struct aoe_atahdr *) (h+1);

	snprintf(buf, sizeof buf,
		"%15s e%ld.%d oldtag=%08x@%08lx newtag=%08x s=%pm d=%pm nout=%d\n",
		"retransmit", d->aoemajor, d->aoeminor, f->tag, jiffies, n,
		h->src, h->dst, t->nout);
	aoechr_error(buf);

	f->tag = n;
	fhash(f);
	h->tag = cpu_to_be32(n);
	memcpy(h->dst, t->addr, sizeof h->dst);
	memcpy(h->src, t->ifp->nd->dev_addr, sizeof h->src);

	skb->dev = t->ifp->nd;
	skb = skb_clone(skb, GFP_ATOMIC);
	if (skb == NULL)
		return;
	__skb_queue_tail(&d->sendq, skb);
}

static int
tsince(u32 tag)
{
	int n;

	n = jiffies & 0xffff;
	n -= tag & 0xffff;
	if (n < 0)
		n += 1<<16;
	return n;
}

static struct aoeif *
getif(struct aoetgt *t, struct net_device *nd)
{
	struct aoeif *p, *e;

	p = t->ifs;
	e = p + NAOEIFS;
	for (; p < e; p++)
		if (p->nd == nd)
			return p;
	return NULL;
}

static struct aoeif *
addif(struct aoetgt *t, struct net_device *nd)
{
	struct aoeif *p;

	p = getif(t, NULL);
	if (!p)
		return NULL;
	p->nd = nd;
	p->maxbcnt = DEFAULTBCNT;
	p->lost = 0;
	p->lostjumbo = 0;
	return p;
}

static void
ejectif(struct aoetgt *t, struct aoeif *ifp)
{
	struct aoeif *e;
	ulong n;

	e = t->ifs + NAOEIFS - 1;
	n = (e - ifp) * sizeof *ifp;
	memmove(ifp, ifp+1, n);
	e->nd = NULL;
}

static int
sthtith(struct aoedev *d)
{
	struct frame *f, *nf;
	struct list_head *nx, *pos, *head;
	struct sk_buff *skb;
	struct aoetgt *ht = d->htgt;
	int i;

	for (i = 0; i < NFACTIVE; i++) {
		head = &ht->factive[i];
		list_for_each_safe(pos, nx, head) {
			f = list_entry(pos, struct frame, head);
			nf = newframe(d);
			if (!nf)
				return 0;

			/* remove frame from active list */
			list_del(pos);

			/* reassign all pertinent bits to new outbound frame */
			skb = nf->skb;
			nf->skb = f->skb;
			nf->buf = f->buf;
			nf->bcnt = f->bcnt;
			nf->lba = f->lba;
			nf->bv = f->bv;
			nf->bv_off = f->bv_off;
			nf->waited = 0;
			f->skb = skb;
			aoe_freetframe(f);
			ht->nout--;
			nf->t->nout++;
			resend(d, nf);
		}
	}
	/* he's clean, he's useless.  take away his interfaces */
	memset(ht->ifs, 0, sizeof ht->ifs);
	d->htgt = NULL;
	return 1;
}

static inline unsigned char
ata_scnt(unsigned char *packet) {
	struct aoe_hdr *h;
	struct aoe_atahdr *ah;

	h = (struct aoe_hdr *) packet;
	ah = (struct aoe_atahdr *) (h+1);
	return ah->scnt;
}

static void
rexmit_timer(ulong vp)
{
	struct sk_buff_head queue;
	struct aoedev *d;
	struct aoetgt *t, **tt, **te;
	struct aoeif *ifp;
	struct frame *f;
	struct list_head *head, *pos, *nx;
	LIST_HEAD(flist);
	register long timeout;
	ulong flags, n;
	int i;

	d = (struct aoedev *) vp;

	/* timeout is always ~150% of the moving average */
	timeout = d->rttavg;
	timeout += timeout >> 1;

	spin_lock_irqsave(&d->lock, flags);

	if (d->flags & DEVFL_TKILL) {
		spin_unlock_irqrestore(&d->lock, flags);
		return;
	}

	/* collect all frames to rexmit into flist */
	tt = d->targets;
	te = tt + NTARGETS;
	for (; tt < te && *tt; tt++) {
		t = *tt;
		for (i = 0; i < NFACTIVE; i++) {
			head = &t->factive[i];
			list_for_each_safe(pos, nx, head) {
				f = list_entry(pos, struct frame, head);
				if (tsince(f->tag) < timeout)
					continue;
				/* move to flist for later processing */
				list_move_tail(pos, &flist);
			}
		}

		/* window check */
		if (t->nout == t->maxout
		&& t->maxout < t->nframes
		&& (jiffies - t->lastwadj)/HZ > 10) {
			t->maxout++;
			t->lastwadj = jiffies;
		}
	}

	/* process expired frames */
	while (!list_empty(&flist)) {
		pos = flist.next;
		f = list_entry(pos, struct frame, head);
		n = f->waited += timeout;
		n /= HZ;
		if (n > aoe_deadsecs) {
			/* Waited too long.  Device failure.
			 * Hang all frames on first hash bucket for downdev
			 * to clean up.
			 */
			list_splice(&flist, &f->t->factive[0]);
			aoedev_downdev(d);
			break;
		}
		list_del(pos);

		t = f->t;
		if (n > HELPWAIT) {
			/* see if another target can help */
			if (d->ntargets > 1)
				d->htgt = t;
		}
		if (t->nout == t->maxout) {
			if (t->maxout > 1)
				t->maxout--;
			t->lastwadj = jiffies;
		}

		ifp = getif(t, f->skb->dev);
		if (ifp && ++ifp->lost > (t->nframes << 1)
		&& (ifp != t->ifs || t->ifs[1].nd)) {
			ejectif(t, ifp);
			ifp = NULL;
		}
		resend(d, f);
	}

	if (!skb_queue_empty(&d->sendq)) {
		n = d->rttavg <<= 1;
		if (n > MAXTIMER)
			d->rttavg = MAXTIMER;
	}

	if (d->flags & DEVFL_KICKME || d->htgt) {
		d->flags &= ~DEVFL_KICKME;
		aoecmd_work(d);
	}

	__skb_queue_head_init(&queue);
	skb_queue_splice_init(&d->sendq, &queue);

	d->timer.expires = jiffies + TIMERTICK;
	add_timer(&d->timer);

	spin_unlock_irqrestore(&d->lock, flags);

	aoenet_xmit(&queue);
}

/* enters with d->lock held */
void
aoecmd_work(struct aoedev *d)
{
	struct buf *buf;
loop:
	if (d->htgt && !sthtith(d))
		return;
	if (d->inprocess == NULL) {
		if (list_empty(&d->bufq))
			return;
		buf = container_of(d->bufq.next, struct buf, bufs);
		list_del(d->bufq.next);
		d->inprocess = buf;
	}
	if (aoecmd_ata_rw(d))
		goto loop;
}

/* this function performs work that has been deferred until sleeping is OK
 */
void
aoecmd_sleepwork(struct work_struct *work)
{
	struct aoedev *d = container_of(work, struct aoedev, work);

	if (d->flags & DEVFL_GDALLOC)
		aoeblk_gdalloc(d);

	if (d->flags & DEVFL_NEWSIZE) {
		struct block_device *bd;
		unsigned long flags;
		u64 ssize;

		ssize = get_capacity(d->gd);
		bd = bdget_disk(d->gd, 0);

		if (bd) {
			mutex_lock(&bd->bd_inode->i_mutex);
			i_size_write(bd->bd_inode, (loff_t)ssize<<9);
			mutex_unlock(&bd->bd_inode->i_mutex);
			bdput(bd);
		}
		spin_lock_irqsave(&d->lock, flags);
		d->flags |= DEVFL_UP;
		d->flags &= ~DEVFL_NEWSIZE;
		spin_unlock_irqrestore(&d->lock, flags);
	}
}

static void
ataid_complete(struct aoedev *d, struct aoetgt *t, unsigned char *id)
{
	u64 ssize;
	u16 n;

	/* word 83: command set supported */
	n = get_unaligned_le16(&id[83 << 1]);

	/* word 86: command set/feature enabled */
	n |= get_unaligned_le16(&id[86 << 1]);

	if (n & (1<<10)) {	/* bit 10: LBA 48 */
		d->flags |= DEVFL_EXT;

		/* word 100: number lba48 sectors */
		ssize = get_unaligned_le64(&id[100 << 1]);

		/* set as in ide-disk.c:init_idedisk_capacity */
		d->geo.cylinders = ssize;
		d->geo.cylinders /= (255 * 63);
		d->geo.heads = 255;
		d->geo.sectors = 63;
	} else {
		d->flags &= ~DEVFL_EXT;

		/* number lba28 sectors */
		ssize = get_unaligned_le32(&id[60 << 1]);

		/* NOTE: obsolete in ATA 6 */
		d->geo.cylinders = get_unaligned_le16(&id[54 << 1]);
		d->geo.heads = get_unaligned_le16(&id[55 << 1]);
		d->geo.sectors = get_unaligned_le16(&id[56 << 1]);
	}

	if (d->ssize != ssize)
		printk(KERN_INFO
			"aoe: %pm e%ld.%d v%04x has %llu sectors\n",
			t->addr,
			d->aoemajor, d->aoeminor,
			d->fw_ver, (long long)ssize);
	d->ssize = ssize;
	d->geo.start = 0;
	if (d->flags & (DEVFL_GDALLOC|DEVFL_NEWSIZE))
		return;
	if (d->gd != NULL) {
		set_capacity(d->gd, ssize);
		d->flags |= DEVFL_NEWSIZE;
	} else
		d->flags |= DEVFL_GDALLOC;
	schedule_work(&d->work);
}

static void
calc_rttavg(struct aoedev *d, int rtt)
{
	register long n;

	n = rtt;
	if (n < 0) {
		n = -rtt;
		if (n < MINTIMER)
			n = MINTIMER;
		else if (n > MAXTIMER)
			n = MAXTIMER;
		d->mintimer += (n - d->mintimer) >> 1;
	} else if (n < d->mintimer)
		n = d->mintimer;
	else if (n > MAXTIMER)
		n = MAXTIMER;

	/* g == .25; cf. Congestion Avoidance and Control, Jacobson & Karels; 1988 */
	n -= d->rttavg;
	d->rttavg += n >> 2;
}

static struct aoetgt *
gettgt(struct aoedev *d, char *addr)
{
	struct aoetgt **t, **e;

	t = d->targets;
	e = t + NTARGETS;
	for (; t < e && *t; t++)
		if (memcmp((*t)->addr, addr, sizeof((*t)->addr)) == 0)
			return *t;
	return NULL;
}

static inline void
diskstats(struct gendisk *disk, struct bio *bio, ulong duration, sector_t sector)
{
	unsigned long n_sect = bio->bi_size >> 9;
	const int rw = bio_data_dir(bio);
	struct hd_struct *part;
	int cpu;

	cpu = part_stat_lock();
	part = disk_map_sector_rcu(disk, sector);

	part_stat_inc(cpu, part, ios[rw]);
	part_stat_add(cpu, part, ticks[rw], duration);
	part_stat_add(cpu, part, sectors[rw], n_sect);
	part_stat_add(cpu, part, io_ticks, duration);

	part_stat_unlock();
}

static void
bvcpy(struct bio_vec *bv, ulong off, struct sk_buff *skb, long cnt)
{
	ulong fcnt;
	char *p;
	int soff = 0;
loop:
	fcnt = bv->bv_len - (off - bv->bv_offset);
	if (fcnt > cnt)
		fcnt = cnt;
	p = page_address(bv->bv_page) + off;
	skb_copy_bits(skb, soff, p, fcnt);
	soff += fcnt;
	cnt -= fcnt;
	if (cnt <= 0)
		return;
	bv++;
	off = bv->bv_offset;
	goto loop;
}

static void
ktiocomplete(struct frame *f)
{
	struct aoe_hdr *hin, *hout;
	struct aoe_atahdr *ahin, *ahout;
	struct buf *buf;
	struct sk_buff *skb;
	struct aoetgt *t;
	struct aoeif *ifp;
	struct aoedev *d;
	long n;

	if (f == NULL)
		return;

	t = f->t;
	d = t->d;

	hout = (struct aoe_hdr *) skb_mac_header(f->skb);
	ahout = (struct aoe_atahdr *) (hout+1);
	buf = f->buf;
	skb = f->r_skb;
	if (skb == NULL)
		goto noskb;	/* just fail the buf. */

	hin = (struct aoe_hdr *) skb->data;
	skb_pull(skb, sizeof(*hin));
	ahin = (struct aoe_atahdr *) skb->data;
	skb_pull(skb, sizeof(*ahin));
	if (ahin->cmdstat & 0xa9) {	/* these bits cleared on success */
		pr_err("aoe: ata error cmd=%2.2Xh stat=%2.2Xh from e%ld.%d\n",
			ahout->cmdstat, ahin->cmdstat,
			d->aoemajor, d->aoeminor);
noskb:	if (buf)
			buf->flags |= BUFFL_FAIL;
		goto badrsp;
	}

	n = ahout->scnt << 9;
	switch (ahout->cmdstat) {
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_READ_EXT:
		if (skb->len < n) {
			pr_err("aoe: runt data size in read.  skb->len=%d need=%ld\n",
				skb->len, n);
			buf->flags |= BUFFL_FAIL;
			break;
		}
		bvcpy(f->bv, f->bv_off, skb, n);
	case ATA_CMD_PIO_WRITE:
	case ATA_CMD_PIO_WRITE_EXT:
		spin_lock_irq(&d->lock);
		ifp = getif(t, skb->dev);
		if (ifp) {
			ifp->lost = 0;
			if (n > DEFAULTBCNT)
				ifp->lostjumbo = 0;
		}
		if (d->htgt == t) /* I'll help myself, thank you. */
			d->htgt = NULL;
		spin_unlock_irq(&d->lock);
		break;
	case ATA_CMD_ID_ATA:
		if (skb->len < 512) {
			pr_info("aoe: runt data size in ataid.  skb->len=%d\n",
				skb->len);
			break;
		}
		if (skb_linearize(skb))
			break;
		spin_lock_irq(&d->lock);
		ataid_complete(d, t, skb->data);
		spin_unlock_irq(&d->lock);
		break;
	default:
		pr_info("aoe: unrecognized ata command %2.2Xh for %d.%d\n",
			ahout->cmdstat,
			be16_to_cpu(get_unaligned(&hin->major)),
			hin->minor);
	}
badrsp:
	spin_lock_irq(&d->lock);

	aoe_freetframe(f);

	if (buf && --buf->nframesout == 0 && buf->resid == 0) {
		struct bio *bio = buf->bio;

		diskstats(d->gd, bio, jiffies - buf->stime, buf->sector);
		n = (buf->flags & BUFFL_FAIL) ? -EIO : 0;
		mempool_free(buf, d->bufpool);
		spin_unlock_irq(&d->lock);
		if (n != -EIO)
			bio_flush_dcache_pages(buf->bio);
		bio_endio(bio, n);
	} else
		spin_unlock_irq(&d->lock);
	dev_kfree_skb(skb);
}

/* Enters with iocq.lock held.
 * Returns true iff responses needing processing remain.
 */
static int
ktio(void)
{
	struct frame *f;
	struct list_head *pos;
	int i;

	for (i = 0; ; ++i) {
		if (i == MAXIOC)
			return 1;
		if (list_empty(&iocq.head))
			return 0;
		pos = iocq.head.next;
		list_del(pos);
		spin_unlock_irq(&iocq.lock);
		f = list_entry(pos, struct frame, head);
		ktiocomplete(f);
		spin_lock_irq(&iocq.lock);
	}
}

static int
kthread(void *vp)
{
	struct ktstate *k;
	DECLARE_WAITQUEUE(wait, current);
	int more;

	k = vp;
	current->flags |= PF_NOFREEZE;
	set_user_nice(current, -10);
	complete(&k->rendez);	/* tell spawner we're running */
	do {
		spin_lock_irq(k->lock);
		more = k->fn();
		if (!more) {
			add_wait_queue(k->waitq, &wait);
			__set_current_state(TASK_INTERRUPTIBLE);
		}
		spin_unlock_irq(k->lock);
		if (!more) {
			schedule();
			remove_wait_queue(k->waitq, &wait);
		} else
			cond_resched();
	} while (!kthread_should_stop());
	complete(&k->rendez);	/* tell spawner we're stopping */
	return 0;
}

static void
aoe_ktstop(struct ktstate *k)
{
	kthread_stop(k->task);
	wait_for_completion(&k->rendez);
}

static int
aoe_ktstart(struct ktstate *k)
{
	struct task_struct *task;

	init_completion(&k->rendez);
	task = kthread_run(kthread, k, k->name);
	if (task == NULL || IS_ERR(task))
		return -ENOMEM;
	k->task = task;
	wait_for_completion(&k->rendez); /* allow kthread to start */
	init_completion(&k->rendez);	/* for waiting for exit later */
	return 0;
}

/* pass it off to kthreads for processing */
static void
ktcomplete(struct frame *f, struct sk_buff *skb)
{
	ulong flags;

	f->r_skb = skb;
	spin_lock_irqsave(&iocq.lock, flags);
	list_add_tail(&f->head, &iocq.head);
	spin_unlock_irqrestore(&iocq.lock, flags);
	wake_up(&ktiowq);
}

struct sk_buff *
aoecmd_ata_rsp(struct sk_buff *skb)
{
	struct aoedev *d;
	struct aoe_hdr *h;
	struct frame *f;
	struct aoetgt *t;
	u32 n;
	ulong flags;
	char ebuf[128];
	u16 aoemajor;

	h = (struct aoe_hdr *) skb->data;
	aoemajor = be16_to_cpu(get_unaligned(&h->major));
	d = aoedev_by_aoeaddr(aoemajor, h->minor);
	if (d == NULL) {
		snprintf(ebuf, sizeof ebuf, "aoecmd_ata_rsp: ata response "
			"for unknown device %d.%d\n",
			aoemajor, h->minor);
		aoechr_error(ebuf);
		return skb;
	}

	spin_lock_irqsave(&d->lock, flags);

	n = be32_to_cpu(get_unaligned(&h->tag));
	t = gettgt(d, h->src);
	if (t == NULL) {
		printk(KERN_INFO "aoe: can't find target e%ld.%d:%pm\n",
		       d->aoemajor, d->aoeminor, h->src);
		spin_unlock_irqrestore(&d->lock, flags);
		return skb;
	}
	f = getframe(t, n);
	if (f == NULL) {
		calc_rttavg(d, -tsince(n));
		spin_unlock_irqrestore(&d->lock, flags);
		snprintf(ebuf, sizeof ebuf,
			"%15s e%d.%d    tag=%08x@%08lx\n",
			"unexpected rsp",
			get_unaligned_be16(&h->major),
			h->minor,
			get_unaligned_be32(&h->tag),
			jiffies);
		aoechr_error(ebuf);
		return skb;
	}
	calc_rttavg(d, tsince(f->tag));
	t->nout--;
	aoecmd_work(d);

	spin_unlock_irqrestore(&d->lock, flags);

	ktcomplete(f, skb);

	/*
	 * Note here that we do not perform an aoedev_put, as we are
	 * leaving this reference for the ktio to release.
	 */
	return NULL;
}

void
aoecmd_cfg(ushort aoemajor, unsigned char aoeminor)
{
	struct sk_buff_head queue;

	__skb_queue_head_init(&queue);
	aoecmd_cfg_pkts(aoemajor, aoeminor, &queue);
	aoenet_xmit(&queue);
}
 
struct sk_buff *
aoecmd_ata_id(struct aoedev *d)
{
	struct aoe_hdr *h;
	struct aoe_atahdr *ah;
	struct frame *f;
	struct sk_buff *skb;
	struct aoetgt *t;

	f = newframe(d);
	if (f == NULL)
		return NULL;

	t = *d->tgt;

	/* initialize the headers & frame */
	skb = f->skb;
	h = (struct aoe_hdr *) skb_mac_header(skb);
	ah = (struct aoe_atahdr *) (h+1);
	skb_put(skb, sizeof *h + sizeof *ah);
	memset(h, 0, skb->len);
	f->tag = aoehdr_atainit(d, t, h);
	fhash(f);
	t->nout++;
	f->waited = 0;

	/* set up ata header */
	ah->scnt = 1;
	ah->cmdstat = ATA_CMD_ID_ATA;
	ah->lba3 = 0xa0;

	skb->dev = t->ifp->nd;

	d->rttavg = MAXTIMER;
	d->timer.function = rexmit_timer;

	return skb_clone(skb, GFP_ATOMIC);
}
 
static struct aoetgt *
addtgt(struct aoedev *d, char *addr, ulong nframes)
{
	struct aoetgt *t, **tt, **te;
	int i;

	tt = d->targets;
	te = tt + NTARGETS;
	for (; tt < te && *tt; tt++)
		;

	if (tt == te) {
		printk(KERN_INFO
			"aoe: device addtgt failure; too many targets\n");
		return NULL;
	}
	t = kzalloc(sizeof(*t), GFP_ATOMIC);
	if (!t) {
		printk(KERN_INFO "aoe: cannot allocate memory to add target\n");
		return NULL;
	}

	d->ntargets++;
	t->nframes = nframes;
	t->d = d;
	memcpy(t->addr, addr, sizeof t->addr);
	t->ifp = t->ifs;
	t->maxout = t->nframes;
	INIT_LIST_HEAD(&t->ffree);
	for (i = 0; i < NFACTIVE; ++i)
		INIT_LIST_HEAD(&t->factive[i]);
	return *tt = t;
}

void
aoecmd_cfg_rsp(struct sk_buff *skb)
{
	struct aoedev *d;
	struct aoe_hdr *h;
	struct aoe_cfghdr *ch;
	struct aoetgt *t;
	struct aoeif *ifp;
	ulong flags, sysminor, aoemajor;
	struct sk_buff *sl;
	u16 n;

	h = (struct aoe_hdr *) skb_mac_header(skb);
	ch = (struct aoe_cfghdr *) (h+1);

	/*
	 * Enough people have their dip switches set backwards to
	 * warrant a loud message for this special case.
	 */
	aoemajor = get_unaligned_be16(&h->major);
	if (aoemajor == 0xfff) {
		printk(KERN_ERR "aoe: Warning: shelf address is all ones.  "
			"Check shelf dip switches.\n");
		return;
	}

	sysminor = SYSMINOR(aoemajor, h->minor);
	if (sysminor * AOE_PARTITIONS + AOE_PARTITIONS > MINORMASK) {
		printk(KERN_INFO "aoe: e%ld.%d: minor number too large\n",
			aoemajor, (int) h->minor);
		return;
	}

	n = be16_to_cpu(ch->bufcnt);
	if (n > aoe_maxout)	/* keep it reasonable */
		n = aoe_maxout;

	d = aoedev_by_sysminor_m(sysminor);
	if (d == NULL) {
		printk(KERN_INFO "aoe: device sysminor_m failure\n");
		return;
	}

	spin_lock_irqsave(&d->lock, flags);

	t = gettgt(d, h->src);
	if (!t) {
		t = addtgt(d, h->src, n);
		if (!t) {
			spin_unlock_irqrestore(&d->lock, flags);
			return;
		}
	}
	ifp = getif(t, skb->dev);
	if (!ifp) {
		ifp = addif(t, skb->dev);
		if (!ifp) {
			printk(KERN_INFO
				"aoe: device addif failure; "
				"too many interfaces?\n");
			spin_unlock_irqrestore(&d->lock, flags);
			return;
		}
	}
	if (ifp->maxbcnt) {
		n = ifp->nd->mtu;
		n -= sizeof (struct aoe_hdr) + sizeof (struct aoe_atahdr);
		n /= 512;
		if (n > ch->scnt)
			n = ch->scnt;
		n = n ? n * 512 : DEFAULTBCNT;
		if (n != ifp->maxbcnt) {
			printk(KERN_INFO
				"aoe: e%ld.%d: setting %d%s%s:%pm\n",
				d->aoemajor, d->aoeminor, n,
				" byte data frames on ", ifp->nd->name,
				t->addr);
			ifp->maxbcnt = n;
		}
	}

	/* don't change users' perspective */
	if (d->nopen) {
		spin_unlock_irqrestore(&d->lock, flags);
		return;
	}
	d->fw_ver = be16_to_cpu(ch->fwver);

	sl = aoecmd_ata_id(d);

	spin_unlock_irqrestore(&d->lock, flags);

	if (sl) {
		struct sk_buff_head queue;
		__skb_queue_head_init(&queue);
		__skb_queue_tail(&queue, sl);
		aoenet_xmit(&queue);
	}
}

void
aoecmd_cleanslate(struct aoedev *d)
{
	struct aoetgt **t, **te;
	struct aoeif *p, *e;

	d->mintimer = MINTIMER;

	t = d->targets;
	te = t + NTARGETS;
	for (; t < te && *t; t++) {
		(*t)->maxout = (*t)->nframes;
		p = (*t)->ifs;
		e = p + NAOEIFS;
		for (; p < e; p++) {
			p->lostjumbo = 0;
			p->lost = 0;
			p->maxbcnt = DEFAULTBCNT;
		}
	}
}

static void
flush_iocq(void)
{
	struct frame *f;
	struct aoedev *d;
	LIST_HEAD(flist);
	struct list_head *pos;
	struct sk_buff *skb;
	ulong flags;

	spin_lock_irqsave(&iocq.lock, flags);
	list_splice_init(&iocq.head, &flist);
	spin_unlock_irqrestore(&iocq.lock, flags);
	while (!list_empty(&flist)) {
		pos = flist.next;
		list_del(pos);
		f = list_entry(pos, struct frame, head);
		d = f->t->d;
		skb = f->r_skb;
		spin_lock_irqsave(&d->lock, flags);
		if (f->buf) {
			f->buf->nframesout--;
			aoe_failbuf(d, f->buf);
		}
		aoe_freetframe(f);
		spin_unlock_irqrestore(&d->lock, flags);
		dev_kfree_skb(skb);
	}
}

int __init
aoecmd_init(void)
{
	INIT_LIST_HEAD(&iocq.head);
	spin_lock_init(&iocq.lock);
	init_waitqueue_head(&ktiowq);
	kts.name = "aoe_ktio";
	kts.fn = ktio;
	kts.waitq = &ktiowq;
	kts.lock = &iocq.lock;
	return aoe_ktstart(&kts);
}

void
aoecmd_exit(void)
{
	aoe_ktstop(&kts);
	flush_iocq();
}
