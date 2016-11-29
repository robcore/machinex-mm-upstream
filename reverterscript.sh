#!/bin/bash

patch -p1 -R < "patches/0400-fixup-exec.c-I-think.patch"
patch -p1 -R < "patches/0399-fair-sched-fixup.patch"
patch -p1 -R < "patches/0398-that-doesnt-belong-there-yet.patch"
patch -p1 -R < "patches/0397-fixup-arm-kernel-signal.c.patch"
patch -p1 -R < "patches/0396-REVERT-fix-some-build-warnings.patch"
patch -p1 -R < "patches/0395-some-fixes.patch"
patch -p1 -R < "patches/0394-added-missing-uidgid.h-header.patch"
patch -p1 -R < "patches/0393-bug.h-need-linux-kernel.h-for-TAINT_WARN.BAHAHAHAHAH.patch"
patch -p1 -R < "patches/0392-sched-fix-some-kernel-doc-warnings.patch"
patch -p1 -R < "patches/0391-bs.patch"
patch -p1 -R < "patches/0390-bs.patch"
patch -p1 -R < "patches/0389-dma-dmaengine-add-slave-req-id-in-slave_config.patch"
patch -p1 -R < "patches/0388-pc-shm-restore-MADV_REMOVE-functionality-on-shared.patch"
patch -p1 -R < "patches/0387-c-r-prctl-drop-VMA-flags-test-on-PR_SET_MM.patch"
patch -p1 -R < "patches/0386-seccomp-squash-part-2.patch"
patch -p1 -R < "patches/0385-seccomp-bring-squash.patch"
patch -p1 -R < "patches/0384-no-new-privs-bringup.patch"
patch -p1 -R < "patches/0383-c-r-prctl-add-ability-to-get-clear_tid_address.patch"
patch -p1 -R < "patches/0382-hmem-replace_page-must-flush_dcache-and-others.patch"
patch -p1 -R < "patches/0381-dox.patch"
patch -p1 -R < "patches/0380-ubi-ubifs-fix-systems-without-debugfs.patch"
patch -p1 -R < "patches/0379-dox-update-wireless-urls.patch"
patch -p1 -R < "patches/0378-drivers-net-Remove-casts-to-same-type.patch"
patch -p1 -R < "patches/0377-perf-Limit-callchains-to-127.patch"
patch -p1 -R < "patches/0376-sched-Validate-assumptions-in-sched_init_numa.patch"
patch -p1 -R < "patches/0375-sched-Always-initialize-cpu-power.patch"
patch -p1 -R < "patches/0374-sched-Fix-domain-iteration.patch"
patch -p1 -R < "patches/0373-sched-numa-Load-balance-between-remote-nodes.patch"
patch -p1 -R < "patches/0372-regmap-Implement-support-for-wake-IRQs.patch"
patch -p1 -R < "patches/0371-regmap-Don-t-try-to-map-non-existant-IRQs.patch"
patch -p1 -R < "patches/0370-net-Remove-casts-to-same-type.patch"
patch -p1 -R < "patches/0369-i2c-Add-generic-I2C-multiplexer-using-pinctrl-API.patch"
patch -p1 -R < "patches/0368-REVERT-mm-compaction-handle-incorrect-MIGRATE_UNMOVA.patch"
patch -p1 -R < "patches/0367-vfs-do_last-make-exit-RCU-safe.patch"
patch -p1 -R < "patches/0366-unexport-do_munmap.patch"
patch -p1 -R < "patches/0365-switch-aio-and-shm-to-do_mmap_pgoff-make-do_mmap.patch"
patch -p1 -R < "patches/0364-take-calculation-of-final-prot-in-security_mmap_file.patch"
patch -p1 -R < "patches/0363-move-security_mmap_addr-to-saner-place.patch"
patch -p1 -R < "patches/0362-take-security_mmap_file-outside-of-mmap_sem.patch"
patch -p1 -R < "patches/0361-ext4-hole-punch-use-truncate_pagecache_range.patch"
patch -p1 -R < "patches/0360-jbd2-use-kmem_cache_zalloc-wrapper-instead-of-flag.patch"
patch -p1 -R < "patches/0359-ext4-let-getattr-report-the-right-blocks-in-delalloc.patch"
patch -p1 -R < "patches/0358-kconfig-update-compression-algorithm-info.patch"
patch -p1 -R < "patches/0357-c-r-prctl-add-ability-to-set-new-mm_struct-exe_file.patch"
patch -p1 -R < "patches/0356-r-prctl-extend-PR_SET_MM-to-set-up-more-mm_structs.patch"
patch -p1 -R < "patches/0355-c-r-procfs-add-arg_start-end-env_start-end-and-exit.patch"
patch -p1 -R < "patches/0354-syscalls-x86-add-__NR_kcmp-syscall.patch"
patch -p1 -R < "patches/0353-fs-proc-introduce-proc-pid-task-tid-children-entry.patch"
patch -p1 -R < "patches/0352-sysctl-make-kernel.ns_last_pid-control-dependent-on-.patch"
patch -p1 -R < "patches/0351-fs-compat-fixup.patch"
patch -p1 -R < "patches/0350-eventfd-change-int-to-__u64-in-eventfd_signal.patch"
patch -p1 -R < "patches/0349-add-some-bs-apple-shitfucking-poop.patch"
patch -p1 -R < "patches/0348-pidns-make-killed-children-autoreap.patch"
patch -p1 -R < "patches/0347-big-ol-ipc-mqueue-squash.and-some-random-driver-addi.patch"
patch -p1 -R < "patches/0346-kexec-export-kexec.h-to-user-space.patch"
patch -p1 -R < "patches/0345-arm-use-clear_tasks_mm_cpumask.patch"
patch -p1 -R < "patches/0344-fork-call-complete_vfork_done-after-clearing-child.patch"
patch -p1 -R < "patches/0343-proc-smaps-show-amount-of-nonlinear-ptes-in-vma.patch"
patch -p1 -R < "patches/0342-proc-smaps-carefully-handle-migration-entries.patch"
patch -p1 -R < "patches/0341-proc-report-file-anon-bit-in-proc-pid-pagemap.patch"
patch -p1 -R < "patches/0340-procfs-use-more-apprioriate-types-when-dumping-proc.patch"
patch -p1 -R < "patches/0339-proc-remove-mm_for_maps.patch"
patch -p1 -R < "patches/0338-stack-usage-add-pid-to-warning-printk-in-check_stack.patch"
patch -p1 -R < "patches/0337-cred-fix.patch"
patch -p1 -R < "patches/0336-kmod-move-call_usermodehelper_fns-to-.c-file.patch"
patch -p1 -R < "patches/0335-kmod-unexport-call_usermodehelper_freeinfo.patch"
patch -p1 -R < "patches/0334-some-fs-squashes.patch"
patch -p1 -R < "patches/0333-init-disable-sparse-checking-of-the-mount.o-source.patch"
patch -p1 -R < "patches/0332-checkpatch-updates.patch"
patch -p1 -R < "patches/0331-cpu_pm-typos.patch"
patch -p1 -R < "patches/0330-sethost-domain-name-notify-only-on-uts-success.patch"
patch -p1 -R < "patches/0329-file_mmap-squash.patch"
patch -p1 -R < "patches/0328-perf-squash.patch"
patch -p1 -R < "patches/0327-perf-Remove-duplicate-invocation-on-perf_event.patch"
patch -p1 -R < "patches/0326-dox.patch"
patch -p1 -R < "patches/0325-pktgen-Use-pr_debug.patch"
patch -p1 -R < "patches/0324-target-Handle-ATA_16-passthrough-for-pSCSI-backend.patch"
patch -p1 -R < "patches/0323-target-Add-MI_REPORT_TARGET_PGS-ext.-header-implicit.patch"
patch -p1 -R < "patches/0322-fs-squASH.patch"
patch -p1 -R < "patches/0321-ext4-Remove-i_mutex-use-from-ext4_quota_write.patch"
patch -p1 -R < "patches/0320-userns-Convert-stat-to-return-values-mapped-from-kui.patch"
patch -p1 -R < "patches/0319-target-add-support-for-the-WRITE_VERIFY-command.patch"
patch -p1 -R < "patches/0318-Smack-allow-for-significantly-longer-Smack-labels-v4.patch"
patch -p1 -R < "patches/0317-gfp-flags-for-security_inode_alloc.patch"
patch -p1 -R < "patches/0316-dmaengine-squash.patch"
patch -p1 -R < "patches/0315-fs-small-rbd-squash.patch"
patch -p1 -R < "patches/0314-FunctionFS-enable-multiple-functions.patch"
patch -p1 -R < "patches/0313-fuse-fix-blksize-calculation.patch"
patch -p1 -R < "patches/0312-small-jffs-squash.patch"
patch -p1 -R < "patches/0311-regmap-Convert-regmap_irq-to-use-irq_domain.patch"
patch -p1 -R < "patches/0310-squashed-dmaengine-hid-logitech-mdiobus-and-target-c.patch"
patch -p1 -R < "patches/0309-dmaengine-dw-add-clk-unprepare-and-DT-probing-caps.patch"
patch -p1 -R < "patches/0308-codel-Controlled-Delay-AQM.patch"
patch -p1 -R < "patches/0307-vfs-make-it-possible-to-access-the-dentry-hash-len.patch"
patch -p1 -R < "patches/0306-tty-move-global-ldisc-idle-waitqueue-to-the-individu.patch"
patch -p1 -R < "patches/0305-clk-mux-assign-init-data-and-add-a-fixed-factor-cloc.patch"
patch -p1 -R < "patches/0304-clk-remove-COMMON_CLK_DISABLE_UNUSED.patch"
patch -p1 -R < "patches/0303-netfilder-mdio-squash.patch"
patch -p1 -R < "patches/0302-driver-core-extend-dev_printk-to-pass-structured-dat.patch"
patch -p1 -R < "patches/0301-vfs-stop-d_splice_alias-creating-directory-aliases.patch"
patch -p1 -R < "patches/0300-bs.patch"
patch -p1 -R < "patches/0299-fsnotify-remove-unused-parameter-from-send_to_group.patch"
patch -p1 -R < "patches/0298-fsnotify-handle-subfiles-perm-events.patch"
patch -p1 -R < "patches/0297-fs-Move-bh_cachep-to-the-__read_mostly-section.patch"
patch -p1 -R < "patches/0296-fs-move-file_remove_suid-to-fs-inode.c.patch"
patch -p1 -R < "patches/0295-bury-__kernel_nlink_t.patch"
patch -p1 -R < "patches/0294-sched-Remove-NULL-assignment-of-dattr_current.patch"
patch -p1 -R < "patches/0293-sched-Remove-the-last-NULL-entry-from-sched_feat_nam.patch"
patch -p1 -R < "patches/0292-sched-Make-sched_feat_names-const.patch"
patch -p1 -R < "patches/0291-sched-Don-t-try-allocating-memory-from-offline-nodes.patch"
patch -p1 -R < "patches/0290-one-last-lglock-cleanup.patch"
patch -p1 -R < "patches/0289-squashed-fs-updates.patch"
patch -p1 -R < "patches/0288-selinuxfs-snprintf-misuses.patch"
patch -p1 -R < "patches/0287-ceph-move-encode_fh-to-new-API.patch"
patch -p1 -R < "patches/0286-encode_fh-API-change.patch"
patch -p1 -R < "patches/0285-some-fput-fget-fd-file-squashes.patch"
patch -p1 -R < "patches/0284-perf-config-Allow-_-in-config-file-variable-names.patch"
patch -p1 -R < "patches/0283-rtc-add-ioctl-to-get-clear-battery-low-voltage-statu.patch"
patch -p1 -R < "patches/0282-radix-tree-fix-preload-vector-size.patch"
patch -p1 -R < "patches/0281-lib-bitmap.c-fix-documentation-for-scnprintf-funcs.patch"
patch -p1 -R < "patches/0280-lib-string_helpers.c-make-arrays-static.patch"
patch -p1 -R < "patches/0279-lib-test-kstrtox.c-mark-const-init-data-with-__initc.patch"
patch -p1 -R < "patches/0278-leds-Use-kcalloc-instead-of-kzalloc-to-allocate-arra.patch"
patch -p1 -R < "patches/0277-leds-simple_strtoul-cleanup.patch"
patch -p1 -R < "patches/0276-squash-remaining-backlight-updates.patch"
patch -p1 -R < "patches/0275-lcd-add-callbacks-for-early-fb-event-blank-support.patch"
patch -p1 -R < "patches/0274-bdev-add-events-for-early-fb-event-support.patch"
patch -p1 -R < "patches/0273-remaining-mm-squashes.patch"
patch -p1 -R < "patches/0272-mm-memcg-keep-ratelimit-counter-separate-from-event.patch"
patch -p1 -R < "patches/0271-mm-memcg-print-statistics-directly-to-seq_file.patch"
patch -p1 -R < "patches/0270-mm-memcg-convert-numa-stat-to-read_seq_string-interf.patch"
patch -p1 -R < "patches/0269-mm-memcg-remove-obsolete-statistics-array-boundary.patch"
patch -p1 -R < "patches/0268-rescounter-remove-__must_check-from-res_counter.patch"
patch -p1 -R < "patches/0267-memcg-don-t-uncharge-in-mem_cgroup_move_account.patch"
patch -p1 -R < "patches/0266-memcg-move-charges-to-root-cgroup-if-use_hierarchy-0.patch"
patch -p1 -R < "patches/0265-memcg-use-res_counter_uncharge_until-in-move_parent.patch"
patch -p1 -R < "patches/0264-rescounters-add-res_counter_uncharge.patch"
patch -p1 -R < "patches/0263-mm-vmscan-kill-struct-mem_cgroup_zone.patch"
patch -p1 -R < "patches/0262-mm-vmscan-push-lruvec-pointer-into-should_continue.patch"
patch -p1 -R < "patches/0261-mm-vmscan-push-lruvec-pointer-into-shrink_list.patch"
patch -p1 -R < "patches/0260-mm-vmscan-push-lruvec-pointer-into-inactive_list_is_.patch"
patch -p1 -R < "patches/0259-mm-vmscan-replace-zone_nr_lru_pages-with-get_lruvec_.patch"
patch -p1 -R < "patches/0258-mm-vmscan-push-lruvec-pointer-into-putback_inactive.patch"
patch -p1 -R < "patches/0257-mm-add-link-from-struct-lruvec-to-struct-zone.patch"
patch -p1 -R < "patches/0256-mm-memcg-move-reclaim_stat-into-lruvec.patch"
patch -p1 -R < "patches/0255-memcg-make-threshold-index-in-the-right-position.patch"
patch -p1 -R < "patches/0254-memcg-remove-redundant-parentheses.patch"
patch -p1 -R < "patches/0253-memcg-mark-stat-field-of-mem_cgroup-struct.patch"
patch -p1 -R < "patches/0252-memcg-remove-unused-variable.patch"
patch -p1 -R < "patches/0251-memcg-mark-more-functions-variables-as-static.patch"
patch -p1 -R < "patches/0250-mm-mark-mm-inline-functions-as-__always_inline.patch"
patch -p1 -R < "patches/0249-memcg-swap-mem_cgroup_move_swap_account-never-needs.patch"
patch -p1 -R < "patches/0248-memcg-swap-mem_cgroup_move_swap_account-never-needs.patch"
patch -p1 -R < "patches/0247-memcg-fix-change-behavior-of-shared-anon.patch"
patch -p1 -R < "patches/0246-page-alloc-cleanup.patch"
patch -p1 -R < "patches/0245-mm-page_alloc-catch-out-of-date-list-of-page-flag.patch"
patch -p1 -R < "patches/0244-REVERT-tmpfs-support-fallocate-FALLOC_FL_PUNCH.patch"
patch -p1 -R < "patches/0243-tmpfs-support-fallocate-FALLOC_FL_PUNCH_HOLE.patch"
patch -p1 -R < "patches/0242-tmpfs-optimize-clearing-when-writing.patch"
patch -p1 -R < "patches/0241-tmpfs-enable-NOSEC-optimization.patch"
patch -p1 -R < "patches/0240-shmem-replace-page-if-mapping-excludes-its-zone.patch"
patch -p1 -R < "patches/0239-mm-compaction-handle-incorrect-MIGRATE_UNMOVABLE-typ.patch"
patch -p1 -R < "patches/0238-mm-bootmem-split-out-goal-to-node-mapping-from-goal.patch"
patch -p1 -R < "patches/0237-mm-bootmem-rename-alloc_bootmem_core-to-bdata.patch"
patch -p1 -R < "patches/0236-mm-move-is_vma_temporary_stack-declaration-to-huge_m.patch"
patch -p1 -R < "patches/0235-bools-vm-page-types.c-cleanups.patch"
patch -p1 -R < "patches/0234-squash-of-a-handful-of-mm-commits.patch"
patch -p1 -R < "patches/0233-mm-thp-drop-page_table_lock-to-uncharge-memcg-pages.patch"
patch -p1 -R < "patches/0232-mm-rename-is_mlocked_vma-to-mlocked_vma_newpage.patch"
patch -p1 -R < "patches/0231-mm-do_migrate_pages-rename-arguments.patch"
patch -p1 -R < "patches/0230-mm-do_migrate_pages-calls-migrate_to_node.patch"
patch -p1 -R < "patches/0229-mm-thp-remove-unnecessary-ret-variable.patch"
patch -p1 -R < "patches/0228-mm-hugetlb.c-use-long-vars-instead-of-int-in-region.patch"
patch -p1 -R < "patches/0227-mm-mempolicy.c-use-enum-value-MPOL_REBIND_ONCE.patch"
patch -p1 -R < "patches/0226-mm-memory_failure-let-the-compiler-add-the-function.patch"
patch -p1 -R < "patches/0225-ext4-use-consistent-ssize_t-type-in-ext4_file_write.patch"
patch -p1 -R < "patches/0224-ext4-fix-format-flag-in-ext4_ext_binsearch_idx.patch"
patch -p1 -R < "patches/0223-ext4-return-ENOMEM-when-mounts-fail.patch"
patch -p1 -R < "patches/0222-ext4-remove-redundundant-char-bh-b_data-casts.patch"
patch -p1 -R < "patches/0221-ext4-remove-needs_recovery-in-ext4_mb_init.patch"
patch -p1 -R < "patches/0220-cgroup-superblock-cant-be-released-with-active-dentr.patch"
patch -p1 -R < "patches/0219-builddeb-include-autogenerated-header-files.patch"
patch -p1 -R < "patches/0218-some-btrfs-squashes.patch"
patch -p1 -R < "patches/0217-Btrfs-ulist-realloc-bugfix.patch"
patch -p1 -R < "patches/0216-Remove-unused-code-from-MPI-library.patch"
patch -p1 -R < "patches/0215-Revert-crypto-GnuPG-based-MPI-lib-additional-sources.patch"
patch -p1 -R < "patches/0214-perf-tools-fix-thread_map__new_by_pid_str-memory.patch"
patch -p1 -R < "patches/0213-scripts-coccinelle-sizeof-of-pointer.patch"
patch -p1 -R < "patches/0212-tick-Move-skew_tick-option-into-the-HIGH_RES_TIMERs.patch"
patch -p1 -R < "patches/0211-keys-fix-some-sparse-warnings.patch"
patch -p1 -R < "patches/0210-dox.patch"
patch -p1 -R < "patches/0209-tick-Add-tick-skew-boot-option.patch"
patch -p1 -R < "patches/0208-keys-kill-task_struct-replacement_session_keyring.patch"
patch -p1 -R < "patches/0207-keys-kill-the-dummy-key_replace_session_keyring.patch"
patch -p1 -R < "patches/0206-can-flexcan-add-PM-support.patch"
patch -p1 -R < "patches/0205-ring-buffer-Check-for-valid-buffer.patch"
patch -p1 -R < "patches/0204-Revert-sched-perf-Use-a-single-callback-into-the-sch.patch"
patch -p1 -R < "patches/0203-Guard-check-in-module-loader-against-integer-overflo.patch"
patch -p1 -R < "patches/0202-modpost-use-proper-kernel-style-for-autogenerated-fi.patch"
patch -p1 -R < "patches/0201-modpost-Stop-grab_file-from-leaking-filedescriptors.patch"
patch -p1 -R < "patches/0200-crypto-disable-preemption-while-benchmarking-RAID5.patch"
patch -p1 -R < "patches/0199-crypto-wait-for-a-full-jiffy-in-do_xor_speed.patch"
patch -p1 -R < "patches/0198-time-remove-obsolete-declaration.patch"
patch -p1 -R < "patches/0197-ntp-stale-comment-whitespace-cleaning.patch"
patch -p1 -R < "patches/0196-irq-Remove-irq_chip-release.patch"
patch -p1 -R < "patches/0195-arm-Select-core-options-instead-of-redefining-them.patch"
patch -p1 -R < "patches/0194-arm-Use-generic-time-config.patch"
patch -p1 -R < "patches/0193-drivers-net-stmmac-seq_file-fix-memory-leak.patch"
patch -p1 -R < "patches/0192-fix-goto-readability-in-nfs_update_inode.patch"
patch -p1 -R < "patches/0191-tracing-Remove-kernel_lock-annotations.patch"
patch -p1 -R < "patches/0190-dox.patch"
patch -p1 -R < "patches/0189-sched-Taint-kernel-with-TAINT_WARN-after-sleep-in-at.patch"
patch -p1 -R < "patches/0188-tracing-bullshit-squash.patch"
patch -p1 -R < "patches/0187-ring-buffer-Reset-head-page-before-running-self-test.patch"
patch -p1 -R < "patches/0186-coredump-ensure-the-fpu-state-is-flushed.patch"
patch -p1 -R < "patches/0185-fork-move-the-real-prepare_to_copy-users-to-arch_dup.patch"
patch -p1 -R < "patches/0184-Input-synaptics-fix-compile-warning.patch"
patch -p1 -R < "patches/0183-staging-android-logger-Fix-some-sparse-and-whitespac.patch"
patch -p1 -R < "patches/0182-perf-Pass-last-sampling-period-to-perf_sample_data_i.patch"
patch -p1 -R < "patches/0181-tracing-Remove-ftrace_disable-enable_cpu.patch"
patch -p1 -R < "patches/0180-tracing-Use-seq_-_private-interface-for-some-seq-fil.patch"
patch -p1 -R < "patches/0179-sound-allow-the-unit-search-until-256-in-sound_core.patch"
patch -p1 -R < "patches/0178-mmc-mmci-Fix-compiler-error-when-CONFIG_OF-is-not-se.patch"
patch -p1 -R < "patches/0177-task_allocator-Use-config-switches-instead-of-magic.patch"
patch -p1 -R < "patches/0176-fork-Provide-kmemcache-based-thread_info-allocator.patch"
patch -p1 -R < "patches/0175-fork-Provide-weak-arch_release_-task_struct-thread_i.patch"
patch -p1 -R < "patches/0174-fork-Move-thread-info-gfp-flags-to-header.patch"
patch -p1 -R < "patches/0173-fork-remove-the-weak-insanity.patch"
patch -p1 -R < "patches/0172-arm-remove-cpu-idle-wait.patch"
patch -p1 -R < "patches/0171-scripts-config-properly-report-and-set-string-option.patch"
patch -p1 -R < "patches/0170-better-not-fuck-around.patch"
patch -p1 -R < "patches/0169-staging-android-persistent_ram-introduce-vmap.patch"
patch -p1 -R < "patches/0168-sched-debug-Fix-printing-large-integers-on-32-bit.patch"
patch -p1 -R < "patches/0167-sched-fair-Revert-sched-domain-iteration-breakage.patch"
patch -p1 -R < "patches/0166-naw-i-am-too-lazy-for-i2c-code-right-now.patch"
patch -p1 -R < "patches/0165-muxes-rename-first-set-of-drivers-to-a-standard.patch"
patch -p1 -R < "patches/0164-of-i2c-implement-of_find_i2c_adapter_by_node.patch"
patch -p1 -R < "patches/0163-i2c-implement-i2c_verify_adapter.patch"
patch -p1 -R < "patches/0162-some-bs.patch"
patch -p1 -R < "patches/0161-KEYS-Permit-in-place-link-replacement-in-keyring.patch"
patch -p1 -R < "patches/0160-KEYS-Announce-key-type-un-registration.patch"
patch -p1 -R < "patches/0159-KEYS-Reorganise-keys-Makefile.patch"
patch -p1 -R < "patches/0158-KEYS-Move-the-key-config-into-security-keys-Kconfig.patch"
patch -p1 -R < "patches/0157-sched-perf-Use-a-single-callback-into-the-scheduler.patch"
patch -p1 -R < "patches/0156-mfd-Allow-for-const-stmpe-keyboard-data.patch"
patch -p1 -R < "patches/0155-0308-sched-fair-Propagate-struct-lb_env-usage-into-f.patch"
patch -p1 -R < "patches/0154-so-reverted-those.patch"
patch -p1 -R < "patches/0153-not-reverting-last-one-finishing-it-and-now-gonna-do.patch"
patch -p1 -R < "patches/0152-revertme.patch"
patch -p1 -R < "patches/0151-signal-make-sure-we-dont-get-stopped-with-pending-ta.patch"
patch -p1 -R < "patches/0150-kthread-reorganize.patch"
patch -p1 -R < "patches/0149-typo.patch"
patch -p1 -R < "patches/0148-PM-QoS-Use-NULL-pointer-instead-of-plain-integer.patch"
patch -p1 -R < "patches/0147-some-gcc-5.0-fixups.patch"
patch -p1 -R < "patches/0146-selinux-call-WARN_ONCE-instead-of-calling-audit_log.patch"
patch -p1 -R < "patches/0145-fix-some-build-warnings.patch"
patch -p1 -R < "patches/0144-REVERT-Revert-leds-use-led_set_brightness-in-led_tri.patch"
patch -p1 -R < "patches/0143-Revert-leds-use-led_set_brightness-in-led_trigger_ev.patch"
patch -p1 -R < "patches/0142-BACKPORT-signal-allow-to-send-any-siginfo-to-itself.patch"
patch -p1 -R < "patches/0141-Revert-msm-kgsl-Fix-Z180-memory-leak.patch"
patch -p1 -R < "patches/0140-revert-that.patch"
patch -p1 -R < "patches/0139-sched-make-scheduler-aware-of-cpu-freq-state.patch"
patch -p1 -R < "patches/0138-new-helper-iterate_fd.patch"
patch -p1 -R < "patches/0137-perf-updates.patch"
