//===--- IoctlCheck.cpp - clang-tidy --------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "IoctlCheck.h"
#include "CheriUtil.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include <map>

using namespace clang::ast_matchers;

namespace clang::tidy::cheri {

/*
 * With CHERI the third argument to the "ioctl" system call must be
 * an integer type that can hold a valid pointer to user space. With
 * CHERI this type is "user_uintptr_t".
 *
 * This check helps with detecting functions and structure field
 * that must be changed in order to handler this according to the
 * following rules:
 * - The field "unlocked_ioctl" in "struct file_operations" is the
 *   root ioctl handling function and this field is the first and
 *   initial ioctl field.
 * - Any function assigned to (an instance of) this field or other
 *   structure fields that are ioctl fields is itself an ioctl function.
 * - Each ioctl function has one argument that must have type
 *   user_uintptr_t. The index of this argument is part of the
 *   function description.
 * - Whenever such an argument is used verbatim as an actual argument
 *   in a function call the callee is considered to be an ioctl
 *   function, too. This applies to both direct function calls and
 *   calls through a function pointer in a structure.
 *   This will discover more ioctl fields and functions.
 * - A function can be blacklisted from being discovered by this
 *   rule. In this case the call site will wrap the argument in
 *   __c_ua() instead.
 * - Additionally, if it is clear from the function argument that
 *   the actual parameter is not used as a pointer the warning is
 *   not printed.
 * If the database of ioctl functions and fields is missing an entry
 * the check will print an appropriate warning and the list of ioctl
 * functions and fields in this file must be amended.
 *
 * In addition to missing ioctl functions and fields the check
 * complains if the user_uintptr_t argument of an identified ioctl
 * function or field has a different type. A replacement (fixup)
 * is suggested and can be applied automatically by passing the
 * "-fix" option to clang-tidy.
 */

/*
 * Registry of ioctl functions field structure fields of function type.
 */
class Ioctl {
public:
  /*
   * Description of a single ioctl function with the following fields:
   * - Name_: The name of the function. Currently there is no way to
   *       deal with different functions that have the same name.
   * - PtrArg_: The zero based(!) index of the function argument that
   *       holds a user pointer.
   */
  struct Func {
    const char *Name_ = nullptr;
    unsigned int PtrArg_ = ~0U;
  };

  /*
   * Description of a single field in a structure that holds a function
   * pointer with an ioctl user pointer argument.
   * - StructName_: The name of the structure.
   * - FieldName_: The name of the function pointer field within that
   *       structure.
   * - PtrArg_: The zero based(!) index of the function argument that
   *       hold a user pointer.
   * There is no way to disambiguate between different structures with
   * the same name but it is possible to have more than one ioctl
   * field in a structure.
   */
  struct Field {
    Field(const char *StructName, const char *FieldName, unsigned int PtrArg)
        : StructName_(StructName), FieldName_(FieldName), PtrArg_(PtrArg) {}
    const char *StructName_ = nullptr;
    const char *FieldName_ = nullptr;
    unsigned int PtrArg_ = ~0U;
  };

  static const std::map<StringRef, Func> &funcs() {
    init();
    return Funcs;
  }

  static const std::map<StringRef, std::vector<Field>> &fields() {
    init();
    return Fields;
  }

  /*
   * Find the Ioctl::Func structure for the function with the given name
   * and return a pointer to it.
   */
  static const Func *findFunc(StringRef Name) {
    const auto It = Funcs.find(Name);
    return It == Funcs.end() ? nullptr : &It->second;
  }

  /*
   * Return true if the function name is blacklisted, i.e. it should
   * never be an ioctl function.
   */
  static bool isBlacklisted(StringRef Name) {
    return Blacklist.find(Name) != Blacklist.end();
  }

  /*
   * Return a pointer to a vector with all the Ioctl::Field structures that
   * describe ioctl fields in the structure with the given name.
   */
  static const std::vector<Field> *findField(StringRef StructName) {
    const auto It = Fields.find(StructName);
    return It == Fields.end() ? nullptr : &It->second;
  }

  /*
   * Return a pointer to the Ioctl::Field structure for a structure
   * field name @FieldName in structure named @StructName and return
   * a pointer to it.
   */
  static const Field *findField(StringRef StructName, StringRef FieldName) {
    const auto *FieldList = findField(StructName);
    if (FieldList) {
      for (const auto &F : *FieldList)
        if (StringRef(F.FieldName_) == FieldName)
          return &F;
    }

    return nullptr;
  }

private:
  /*
   * Add a function name to the Ioctl database. A second call
   * to this function with the same function name will replace
   * the entry.
   */
  static void addFunc(const char *Name, unsigned int PtrArg) {
    Funcs[StringRef(Name)].Name_ = Name;
    Funcs[StringRef(Name)].PtrArg_ = PtrArg;
  }

  /*
   * Add a function naem to the blacklist.
   */
  static void addBlacklist(const char *Name) {
    Blacklist[StringRef(Name)] = true;
  }

  /*
   * Add a structure field to the Ioctl database. A second call
   * to this function with the same @StructName and @FieldName
   * will replace the entry. However, multiple calls with the
   * same @StructName but different @FieldName{}s are possible.
   */
  static void addField(const char *StructName, const char *FieldName,
                       unsigned int PtrArg) {
    auto *FV = &Fields[StructName];
    for (const auto &F : *FV) {
      if (StringRef(F.FieldName_) == StringRef(FieldName))
        return;
    }
    FV->push_back(Field(StructName, FieldName, PtrArg));
  }

  static bool InitDone;
  static void doInit();
  static void init() {
    if (InitDone)
      return;
    doInit();
  }
  static std::map<StringRef, Func> Funcs;
  static std::map<StringRef, std::vector<Field>> Fields;
  static std::map<StringRef, bool> Blacklist;
};

bool Ioctl::InitDone = false;
std::map<StringRef, Ioctl::Func> Ioctl::Funcs;
std::map<StringRef, std::vector<Ioctl::Field>> Ioctl::Fields;
std::map<StringRef, bool> Ioctl::Blacklist;

/*
 * Initialize the ioctl database. This function is called automatically
 * during the first lookup in the database.
 *
 * It should be possible to start with entries for just
 * "file_operations->unlocked_ioctl" and "__c_ua()".
 *
 * Other functions and structure field name that take an ioctl
 * argument will be detected automatically and the check will
 * print a hint that can be used to augment the list below.
 * Once the compile prints no more errors about prefixed with
 * "MISSSING:" the list is current for the active kernel version
 * and kernel configuration.
 *
 * FIXME: Ioctl functions passed to snd_ctl_register_ioctl() are not
 *     detected automatically. It should be fairly easy to add an
 *     appropriate check hardcoding the funtion name, though.
 */
void Ioctl::doInit() {
  /* Root */
  addField("file_operations", "unlocked_ioctl", 2);
  addFunc("do_vfs_ioctl", 3);
  addFunc("vfs_ioctl", 2);
  /* Not detected automatically due to NOLINT. */
  addField("media_file_operations", "ioctl", 2);
  /* Not detected automatically unless we re-write some code. */
  addField("proc_ops", "proc_ioctl", 2);
  /* Special case: Args to snd_ctl_register_ioctl */
  addFunc("snd_hwdep_control_ioctl", 3);
  addFunc("snd_pcm_control_ioctl", 3);
  addFunc("snd_rawmidi_control_ioctl", 3);

  /* Blacklisted functions that are never ioctl functions. */
  addBlacklist("cdrom_ioctl_clear_options");
  addBlacklist("cdrom_ioctl_debug");
  addBlacklist("cdrom_ioctl_drive_status");
  addBlacklist("cdrom_ioctl_eject_sw");
  addBlacklist("cdrom_ioctl_lock_door");
  addBlacklist("cdrom_ioctl_media_changed");
  addBlacklist("cdrom_ioctl_select_disc");
  addBlacklist("cdrom_ioctl_select_speed");
  addBlacklist("cdrom_ioctl_set_options");
  addBlacklist("gru_find_lock_gts");
  addBlacklist("gru_ktest");
  addBlacklist("hcall_set_irqline");
  addBlacklist("hci_uart_set_flags");
  addBlacklist("hpet_time_div");
  addBlacklist("i915_perf_config_locked");
  addBlacklist("ioctl_file_clone");
  addBlacklist("iommufd_vfio_check_extension");
  addBlacklist("iommufd_vfio_set_iommu");
  addBlacklist("ksys_ioperm");
  addBlacklist("kvm_arch_init_vm");
  addBlacklist("kvm_create_vm");
  addBlacklist("kvm_dev_ioctl_create_vm");
  addBlacklist("kvm_vm_ioctl_check_extension");
  addBlacklist("kvm_vm_ioctl_check_extension_generic");
  addBlacklist("loop_set_block_size");
  addBlacklist("loop_set_dio");
  addBlacklist("nbd_add_socket");
  addBlacklist("nbd_set_cmd_timeout");
  addBlacklist("nbd_set_size");
  addBlacklist("pin_user_pages_fast");
  addBlacklist("seccomp_notify_set_flags");
  addBlacklist("set_offload");
  addBlacklist("trace_binder_ioctl");
  addBlacklist("valid_signal");
  addBlacklist("vfio_iommu_type1_check_extension");

  /* Detected. Make sure list is unique. */
  addField("atm_ioctl", "ioctl", 2);
  addField("bcache_device", "ioctl", 3);
  addField("block_device_operations", "ioctl", 3);
  addField("dfl_feature_ops", "ioctl", 3);
  addField("fb_ops", "fb_ioctl", 2);
  addField("iio_ioctl_handler", "ioctl", 3);
  addField("isst_if_cmd_cb", "def_ioctl", 2);
  addField("kvm_device_ops", "ioctl", 2);
  addField("posix_clock_operations", "ioctl", 2);
  addField("ppp_channel_ops", "ioctl", 2);
  addField("pppox_proto", "ioctl", 2);
  addField("proto_ops", "ioctl", 2);
  addField("rtc_class_ops", "ioctl", 2);
  addField("smb_version_operations", "ioctl_query_info", 5);
  addField("snd_hwdep_ops", "ioctl", 3);
  addField("snd_info_entry_ops", "ioctl", 4);
  addField("snd_kctl_ioctl", "fioctl", 3);
  addField("snd_seq_oss_callback", "ioctl", 2);
  addField("thread_with_stdio_ops", "unlocked_ioctl", 2);
  addField("tty_ldisc_ops", "ioctl", 2);
  addField("tty_operations", "ioctl", 2);
  addField("uacce_ops", "ioctl", 2);
  addField("uart_ops", "ioctl", 2);
  addField("usb_gadget_ops", "ioctl", 2);
  addField("usb_serial_driver", "ioctl", 2);
  addField("usb_serial_driver", "tiocmiwait", 1);
  addField("v4l2_file_operations", "unlocked_ioctl", 2);
  addField("vfio_device_ops", "ioctl", 2);
  addField("vfio_iommu_driver_ops", "ioctl", 2);
  addField("vfio_iommu_driver_ops", "open", 0);
  addField("watchdog_ops", "ioctl", 2);
  addFunc("__cxl_memdev_ioctl", 2);
  addFunc("__ext4_ioctl", 2);
  addFunc("__f2fs_ioctl", 2);
  addFunc("__media_ioctl", 2);
  addFunc("__mptctl_ioctl", 2);
  addFunc("__nbd_ioctl", 3);
  addFunc("__nd_ioctl", 4);
  addFunc("__snd_timer_user_ioctl", 2);
  addFunc("__ssam_cdev_device_ioctl", 2);
  addFunc("__surface_dtx_ioctl", 2);
  addFunc("__tty_perform_flush", 1);
  addFunc("__tun_chr_ioctl", 2);
  addFunc("_ctl_ioctl", 2);
  addFunc("_ctl_mpt2_ioctl", 2);
  addFunc("_hl_ioctl", 2);
  addFunc("_perf_ioctl", 2);
  addFunc("aac_cfg_ioctl", 2);
  addFunc("abx80x_ioctl", 2);
  addFunc("ac_ioctl", 2);
  addFunc("acm_tty_ioctl", 2);
  addFunc("acpi_thermal_rel_ioctl", 2);
  addFunc("acq_ioctl", 2);
  addFunc("acrn_dev_ioctl", 2);
  addFunc("adf_ctl_alloc_resources", 1);
  addFunc("adf_ctl_ioctl", 2);
  addFunc("adf_ctl_ioctl_dev_config", 2);
  addFunc("adf_ctl_ioctl_dev_start", 2);
  addFunc("adf_ctl_ioctl_dev_stop", 2);
  addFunc("adf_ctl_ioctl_get_num_devices", 2);
  addFunc("adf_ctl_ioctl_get_status", 2);
  addFunc("advwdt_ioctl", 2);
  addFunc("afu_ioctl", 2);
  addFunc("afu_ioctl_check_extension", 1);
  addFunc("ali_ioctl", 2);
  addFunc("amdgpu_debugfs_gprwave_ioctl", 2);
  addFunc("amdgpu_debugfs_regs2_ioctl", 2);
  addFunc("amdgpu_drm_ioctl", 2);
  addFunc("aoeblk_ioctl", 3);
  addFunc("arcfb_ioctl", 2);
  addFunc("aspeed_lpc_ctrl_ioctl", 2);
  addFunc("aspeed_p2a_ioctl", 2);
  addFunc("asr_ioctl", 2);
  addFunc("assign_ctxt", 1);
  addFunc("at91_wdt_ioctl", 2);
  addFunc("atalk_ioctl", 2);
  addFunc("atm_mpoa_ioctl", 2);
  addFunc("atmtcp_ioctl", 2);
  addFunc("aty128fb_ioctl", 2);
  addFunc("atyfb_ioctl", 2);
  addFunc("autofs_dev_ioctl", 2);
  addFunc("autofs_root_ioctl", 2);
  addFunc("autofs_root_ioctl_unlocked", 3);
  addFunc("ax25_ioctl", 2);
  addFunc("base_sock_ioctl", 2);
  addFunc("bch2_chardev_ioctl", 2);
  addFunc("bch2_fs_file_ioctl", 2);
  addFunc("bcm_sock_no_ioctlcmd", 2);
  addFunc("bcm_vk_ioctl", 2);
  addFunc("binder_ctl_ioctl", 2);
  addFunc("binder_ioctl", 2);
  addFunc("binder_ioctl_write_read", 1);
  addFunc("blk_ioctl_discard", 2);
  addFunc("blk_ioctl_zeroout", 2);
  addFunc("blkdev_common_ioctl", 3);
  addFunc("blkdev_flushbuf", 2);
  addFunc("blkdev_ioctl", 2);
  addFunc("blkdev_report_zones_ioctl", 2);
  addFunc("blkdev_roset", 2);
  addFunc("blkdev_zone_mgmt_ioctl", 3);
  addFunc("blkif_ioctl", 3);
  addFunc("bnep_sock_ioctl", 2);
  addFunc("br2684_ioctl", 2);
  addFunc("bsg_ioctl", 2);
  addFunc("bt_bmc_ioctl", 2);
  addFunc("bt_sock_ioctl", 2);
  addFunc("btrfs_control_ioctl", 2);
  addFunc("btrfs_ioctl", 2);
  addFunc("bus_ioctl", 2);
  addFunc("ca8210_test_int_ioctl", 2);
  addFunc("cache_ioctl", 3);
  addFunc("cache_ioctl_pipefs", 2);
  addFunc("cache_ioctl_procfs", 2);
  addFunc("cached_dev_ioctl", 3);
  addFunc("cached_dev_ioctl", 3);
  addFunc("cachefiles_ondemand_fd_ioctl", 2);
  addFunc("can327_ldisc_ioctl", 2);
  addFunc("cap_ioctl_unlocked", 2);
  addFunc("capi_ioctl", 2);
  addFunc("capi_unlocked_ioctl", 2);
  addFunc("cdrom_ioctl", 3);
  addFunc("cdrom_ioctl_timed_media_change", 1);
  addFunc("cec_ioctl", 2);
  addFunc("ceph_ioctl", 2);
  addFunc("ceph_set_encryption_policy", 1);
  addFunc("ch_ioctl", 2);
  addFunc("cifs_ioctl", 2);
  addFunc("cifs_ioctl_copychunk", 2);
  addFunc("cifs_ioctl_query_info", 2);
  addFunc("cifs_shutdown", 1);
  addFunc("clip_ioctl", 2);
  addFunc("cmtp_sock_ioctl", 2);
  addFunc("coda_pioctl", 2);
  addFunc("coda_psdev_ioctl", 2);
  addFunc("comedi_unlocked_ioctl", 2);
  addFunc("counter_add_watch", 1);
  addFunc("counter_chrdev_ioctl", 2);
  addFunc("cpu5wdt_ioctl", 2);
  addFunc("cros_ec_chardev_ioctl", 2);
  addFunc("ctrl_cdev_ioctl", 2);
  addFunc("cuse_file_ioctl", 2);
  addFunc("cxl_memdev_ioctl", 2);
  addFunc("data_sock_ioctl", 2);
  addFunc("dbc_ioctl", 2);
  addFunc("ddb_ioctl", 2);
  addFunc("dell_smbios_wmi_ioctl", 2);
  addFunc("dfl_feature_ioctl_get_num_irqs", 2);
  addFunc("dfl_feature_ioctl_set_irq", 2);
  addFunc("dimm_ioctl", 2);
  addFunc("dispatch_ioctl_command", 1);
  addFunc("dlfb_ops_ioctl", 2);
  addFunc("dm_blk_ioctl", 3);
  addFunc("dm_ctl_ioctl", 2);
  addFunc("dm_verity_ioctl", 2);
  addFunc("dma_buf_ioctl", 2);
  addFunc("dma_heap_ioctl", 2);
  addFunc("dmirror_fops_unlocked_ioctl", 2);
  addFunc("do_cancel_ioctl", 1);
  addFunc("do_execute_ddcb", 1);
  addFunc("do_fb_ioctl", 2);
  addFunc("do_lock_ioctl", 1);
  addFunc("do_pagemap_cmd", 2);
  addFunc("do_pagemap_scan", 1);
  addFunc("do_poll_ioctl", 1);
  addFunc("do_setrsubd_ioctl", 1);
  addFunc("do_setwsubd_ioctl", 1);
  addFunc("do_unlock_ioctl", 1);
  addFunc("do_vcc_ioctl", 2);
  addFunc("dpi_dev_ioctl", 2);
  addFunc("drm_fb_helper_ioctl", 2);
  addFunc("drm_ioctl", 2);
  addFunc("ds1286_ioctl", 2);
  addFunc("dst_ca_ioctl", 2);
  addFunc("dvb_ca_en50221_io_ioctl", 2);
  addFunc("dvb_demux_ioctl", 2);
  addFunc("dvb_dvr_ioctl", 2);
  addFunc("dvb_frontend_ioctl", 2);
  addFunc("dvb_generic_ioctl", 2);
  addFunc("dvb_net_ioctl", 2);
  addFunc("dvb_usercopy", 2);
  addFunc("ecryptfs_unlocked_ioctl", 2);
  addFunc("edge_ioctl", 2);
  addFunc("efi_runtime_get_nexthighmonocount", 0);
  addFunc("efi_runtime_get_nextvariablename", 0);
  addFunc("efi_runtime_get_supported_mask", 0);
  addFunc("efi_runtime_get_time", 0);
  addFunc("efi_runtime_get_variable", 0);
  addFunc("efi_runtime_get_waketime", 0);
  addFunc("efi_runtime_query_capsulecaps", 0);
  addFunc("efi_runtime_query_variableinfo", 0);
  addFunc("efi_runtime_reset_system", 0);
  addFunc("efi_runtime_set_time", 0);
  addFunc("efi_runtime_set_variable", 0);
  addFunc("efi_runtime_set_waketime", 0);
  addFunc("efi_test_ioctl", 2);
  addFunc("ep_eventpoll_bp_ioctl", 2);
  addFunc("ep_eventpoll_ioctl", 2);
  addFunc("ep_ioctl", 2);
  addFunc("epx_c3_ioctl", 2);
  addFunc("erst_dbg_ioctl", 2);
  addFunc("esas2r_proc_ioctl", 2);
  addFunc("eurwdt_ioctl", 2);
  addFunc("evdev_ioctl", 2);
  addFunc("evtchn_ioctl", 2);
  addFunc("exfat_ioctl", 2);
  addFunc("exfat_ioctl_fitrim", 1);
  addFunc("exfat_ioctl_shutdown", 1);
  addFunc("ext2_ioctl", 2);
  addFunc("ext4_ioctl", 2);
  addFunc("ext4_ioctl_checkpoint", 1);
  addFunc("ext4_ioctl_get_es_cache", 1);
  addFunc("ext4_ioctl_shutdown", 1);
  addFunc("f2fs_ioc_add_encryption_key", 1);
  addFunc("f2fs_ioc_defragment", 1);
  addFunc("f2fs_ioc_enable_verity", 1);
  addFunc("f2fs_ioc_fitrim", 1);
  addFunc("f2fs_ioc_flush_device", 1);
  addFunc("f2fs_ioc_gc", 1);
  addFunc("f2fs_ioc_gc_range", 1);
  addFunc("f2fs_ioc_get_compress_blocks", 1);
  addFunc("f2fs_ioc_get_compress_option", 1);
  addFunc("f2fs_ioc_get_encryption_key_status", 1);
  addFunc("f2fs_ioc_get_encryption_nonce", 1);
  addFunc("f2fs_ioc_get_encryption_policy", 1);
  addFunc("f2fs_ioc_get_encryption_policy_ex", 1);
  addFunc("f2fs_ioc_get_encryption_pwsalt", 1);
  addFunc("f2fs_ioc_get_features", 1);
  addFunc("f2fs_ioc_get_pin_file", 1);
  addFunc("f2fs_ioc_getfslabel", 1);
  addFunc("f2fs_ioc_getversion", 1);
  addFunc("f2fs_ioc_measure_verity", 1);
  addFunc("f2fs_ioc_move_range", 1);
  addFunc("f2fs_ioc_read_verity_metadata", 1);
  addFunc("f2fs_ioc_remove_encryption_key", 1);
  addFunc("f2fs_ioc_remove_encryption_key_all_users", 1);
  addFunc("f2fs_ioc_resize_fs", 1);
  addFunc("f2fs_ioc_set_compress_option", 1);
  addFunc("f2fs_ioc_set_encryption_policy", 1);
  addFunc("f2fs_ioc_set_pin_file", 1);
  addFunc("f2fs_ioc_setfslabel", 1);
  addFunc("f2fs_ioc_shutdown", 1);
  addFunc("f2fs_ioctl", 2);
  addFunc("f2fs_release_compress_blocks", 1);
  addFunc("f2fs_reserve_compress_blocks", 1);
  addFunc("f2fs_sec_trim_file", 1);
  addFunc("f_hidg_ioctl", 2);
  addFunc("fanotify_ioctl", 2);
  addFunc("fastrpc_device_ioctl", 2);
  addFunc("fat_dir_ioctl", 2);
  addFunc("fat_generic_ioctl", 2);
  addFunc("fat_ioctl_fitrim", 1);
  addFunc("fb_ioctl", 2);
  addFunc("fd_ioctl", 3);
  addFunc("fd_locked_ioctl", 3);
  addFunc("ffs_ep0_ioctl", 2);
  addFunc("ffs_epfile_ioctl", 2);
  addFunc("fitpc2_wdt_ioctl", 2);
  addFunc("flash_dev_ioctl", 3);
  addFunc("fme_global_error_ioctl", 3);
  addFunc("fme_hdr_ioctl", 3);
  addFunc("fme_hdr_ioctl_assign_port", 1);
  addFunc("fme_hdr_ioctl_release_port", 1);
  addFunc("fme_ioctl", 2);
  addFunc("fme_ioctl_check_extension", 1);
  addFunc("fme_pr", 1);
  addFunc("fme_pr_ioctl", 3);
  addFunc("fop_ioctl", 2);
  addFunc("fsl_mc_uapi_dev_ioctl", 2);
  addFunc("fsl_mc_uapi_send_command", 1);
  addFunc("ftdi_ioctl", 2);
  addFunc("full_proxy_unlocked_ioctl", 2);
  addFunc("fuse_dev_ioctl", 2);
  addFunc("fuse_dir_ioctl", 2);
  addFunc("fuse_do_ioctl", 2);
  addFunc("fuse_file_ioctl", 2);
  addFunc("fuse_ioctl_common", 2);
  addFunc("fuse_setup_enable_verity", 0);
  addFunc("fuse_setup_measure_verity", 0);
  addFunc("fw_device_op_ioctl", 2);
  addFunc("fw_mgmt_ioctl_unlocked", 2);
  addFunc("fwctl_fops_ioctl", 2);
  addFunc("gadget_dev_ioctl", 2);
  addFunc("gamecube_rtc_ioctl", 2);
  addFunc("gb_tty_ioctl", 2);
  addFunc("genwqe_ioctl", 2);
  addFunc("geodewdt_ioctl", 2);
  addFunc("get_base_info", 1);
  addFunc("get_ctxt_info", 1);
  addFunc("gfs2_ioctl", 2);
  addFunc("gntalloc_ioctl", 2);
  addFunc("gntdev_ioctl", 2);
  addFunc("gpio_ioctl", 2);
  addFunc("gru_create_new_context", 0);
  addFunc("gru_dump_chiplet_request", 0);
  addFunc("gru_file_unlocked_ioctl", 2);
  addFunc("gru_get_config_info", 0);
  addFunc("gru_get_exception_detail", 0);
  addFunc("gru_get_gseg_statistics", 0);
  addFunc("gru_handle_user_call_os", 0);
  addFunc("gru_set_context_option", 0);
  addFunc("gru_user_flush_tlb", 0);
  addFunc("gru_user_unload_context", 0);
  addFunc("gsmld_ioctl", 2);
  addFunc("gsmtty_ioctl", 2);
  addFunc("gup_test_ioctl", 2);
  addFunc("hci_sock_bound_ioctl", 2);
  addFunc("hci_sock_ioctl", 2);
  addFunc("hci_uart_tty_ioctl", 2);
  addFunc("hda_hwdep_ioctl", 3);
  addFunc("hfi1_file_ioctl", 2);
  addFunc("hfsplus_ioctl", 2);
  addFunc("hiddev_ioctl", 2);
  addFunc("hidp_sock_ioctl", 2);
  addFunc("hidraw_ioctl", 2);
  addFunc("hisi_acc_vf_precopy_ioctl", 2);
  addFunc("hisi_acc_vfio_pci_ioctl", 2);
  addFunc("hisi_qm_uacce_ioctl", 2);
  addFunc("hl_ioctl_control", 2);
  addFunc("hpet_ioctl", 2);
  addFunc("hpet_ioctl_common", 2);
  addFunc("hpfs_ioctl", 2);
  addFunc("hsc_ioctl", 2);
  addFunc("hsmp_ioctl", 2);
  addFunc("hso_serial_ioctl", 2);
  addFunc("hso_wait_modem_status", 1);
  addFunc("hung_up_tty_ioctl", 2);
  addFunc("hwdep_ioctl", 3);
  addFunc("i2cdev_ioctl", 2);
  addFunc("i8k_ioctl", 2);
  addFunc("i915_perf_ioctl", 2);
  addFunc("i915_perf_ioctl_locked", 2);
  addFunc("ib_umad_ioctl", 2);
  addFunc("ib_uverbs_ioctl", 2);
  addFunc("ibwdt_ioctl", 2);
  addFunc("ieee802154_sock_ioctl", 2);
  addFunc("iio_buffer_chrdev_ioctl", 2);
  addFunc("iio_device_buffer_getfd", 1);
  addFunc("iio_device_buffer_ioctl", 3);
  addFunc("iio_event_ioctl", 3);
  addFunc("iio_ioctl", 2);
  addFunc("imsttfb_ioctl", 2);
  addFunc("inet6_ioctl", 2);
  addFunc("inet_ioctl", 2);
  addFunc("inotify_ioctl", 2);
  addFunc("intel_vgpu_ioctl", 2);
  addFunc("ioctl_dev", 3);
  addFunc("iommufd_fops_ioctl", 2);
  addFunc("iommufd_vfio_ioctl", 2);
  addFunc("iowarrior_ioctl", 2);
  addFunc("ipmi_ioctl", 2);
  addFunc("ipmi_unlocked_ioctl", 2);
  addFunc("ipw_ioctl", 2);
  addFunc("ipwireless_ppp_ioctl", 2);
  addFunc("isl12022_rtc_ioctl", 2);
  addFunc("isotp_sock_no_ioctlcmd", 2);
  addFunc("isst_if_def_ioctl", 2);
  addFunc("it8712f_wdt_ioctl", 2);
  addFunc("ivtvfb_ioctl", 2);
  addFunc("j1939_sk_no_ioctlcmd", 2);
  addFunc("jffs2_ioctl", 2);
  addFunc("jfs_ioctl", 2);
  addFunc("joydev_ioctl", 2);
  addFunc("kcm_ioctl", 2);
  addFunc("kcov_get_mode", 0);
  addFunc("kcov_ioctl", 2);
  addFunc("kcov_ioctl_locked", 2);
  addFunc("kcs_bmc_ipmi_ioctl", 2);
  addFunc("kempld_wdt_ioctl", 2);
  addFunc("kfd_ioctl", 2);
  addFunc("kobil_ioctl", 2);
  addFunc("kvm_arch_dev_ioctl", 2);
  addFunc("kvm_arch_vcpu_async_ioctl", 2);
  addFunc("kvm_arch_vcpu_ioctl", 2);
  addFunc("kvm_arch_vm_ioctl", 2);
  addFunc("kvm_dev_ioctl", 2);
  addFunc("kvm_device_ioctl", 2);
  addFunc("kvm_device_ioctl_attr", 2);
  addFunc("kvm_mmu_change_mmu_pages", 1);
  addFunc("kvm_vcpu_ioctl", 2);
  addFunc("kvm_vm_ioctl", 2);
  addFunc("kvm_vm_ioctl_set_nr_mmu_pages", 1);
  addFunc("kvm_vm_ioctl_set_tss_addr", 1);
  addFunc("kyrofb_ioctl", 2);
  addFunc("lane_ioctl", 2);
  addFunc("lcd_ioctl", 2);
  addFunc("lineevent_ioctl", 2);
  addFunc("linehandle_ioctl", 2);
  addFunc("linereq_ioctl", 2);
  addFunc("lirc_ioctl", 2);
  addFunc("llc_ui_ioctl", 2);
  addFunc("lo_ioctl", 3);
  addFunc("lo_simple_ioctl", 2);
  addFunc("loop_control_ioctl", 2);
  addFunc("lp_do_ioctl", 2);
  addFunc("lp_ioctl", 2);
  addFunc("mISDN_ioctl", 2);
  addFunc("manage_rcvq", 2);
  addFunc("map_benchmark_ioctl", 2);
  addFunc("matroxfb_dh_ioctl", 2);
  addFunc("matroxfb_ioctl", 2);
  addFunc("mb862xxfb_ioctl", 2);
  addFunc("mbochs_ioctl", 2);
  addFunc("mce_chrdev_ioctl", 2);
  addFunc("mctp_ioctl", 2);
  addFunc("mctp_ioctl_alloctag", 2);
  addFunc("mctp_ioctl_droptag", 2);
  addFunc("mctp_ioctl_tag_copy_from_user", 0);
  addFunc("mctp_ioctl_tag_copy_to_user", 0);
  addFunc("md_ioctl", 3);
  addFunc("mdpy_ioctl", 2);
  addFunc("media_device_ioctl", 2);
  addFunc("media_ioctl", 2);
  addFunc("media_request_ioctl", 2);
  addFunc("megadev_ioctl", 2);
  addFunc("megadev_unlocked_ioctl", 2);
  addFunc("megasas_mgmt_ioctl", 2);
  addFunc("megasas_mgmt_ioctl_aen", 1);
  addFunc("megasas_mgmt_ioctl_fw", 1);
  addFunc("mei_ioctl", 2);
  addFunc("mkiss_ioctl", 2);
  addFunc("mlx5vf_precopy_ioctl", 2);
  addFunc("mmc_blk_ioctl", 3);
  addFunc("mmc_ioctl", 2);
  addFunc("mmc_rpmb_ioctl", 2);
  addFunc("mon_bin_ioctl", 2);
  addFunc("mos7720_ioctl", 2);
  addFunc("mos7840_ioctl", 2);
  addFunc("moxa_ioctl", 2);
  addFunc("mport_cdev_ioctl", 2);
  addFunc("mptctl_do_reset", 1);
  addFunc("mptctl_eventenable", 1);
  addFunc("mptctl_eventquery", 1);
  addFunc("mptctl_eventreport", 1);
  addFunc("mptctl_fw_download", 1);
  addFunc("mptctl_getiocinfo", 1);
  addFunc("mptctl_gettargetinfo", 1);
  addFunc("mptctl_hp_hostinfo", 1);
  addFunc("mptctl_hp_targetinfo", 1);
  addFunc("mptctl_ioctl", 2);
  addFunc("mptctl_mpt_command", 1);
  addFunc("mptctl_readtest", 1);
  addFunc("mptctl_replace_fw", 1);
  addFunc("mraid_mm_ioctl", 2);
  addFunc("mraid_mm_unlocked_ioctl", 2);
  addFunc("msr_ioctl", 2);
  addFunc("mtdchar_ioctl", 2);
  addFunc("mtdchar_unlocked_ioctl", 2);
  addFunc("mtip_block_ioctl", 3);
  addFunc("mtip_hw_ioctl", 2);
  addFunc("mtrr_ioctl", 2);
  addFunc("mtty_ioctl", 2);
  addFunc("mtty_precopy_ioctl", 2);
  addFunc("mwave_ioctl", 2);
  addFunc("mxser_cflags_changed", 1);
  addFunc("mxser_ioctl", 2);
  addFunc("n_hdlc_tty_ioctl", 2);
  addFunc("n_tty_ioctl", 2);
  addFunc("n_tty_ioctl_helper", 2);
  addFunc("nbd_ioctl", 3);
  addFunc("nci_uart_tty_ioctl", 2);
  addFunc("nct3018y_ioctl", 2);
  addFunc("nd_ioctl", 2);
  addFunc("ne_enclave_ioctl", 2);
  addFunc("ne_ioctl", 2);
  addFunc("netlink_ioctl", 2);
  addFunc("nilfs_ioctl", 2);
  addFunc("nosy_ioctl", 2);
  addFunc("nouveau_drm_ioctl", 2);
  addFunc("nr_ioctl", 2);
  addFunc("ns_ioctl", 2);
  addFunc("nsm_dev_ioctl", 2);
  addFunc("ntfs_ioctl", 2);
  addFunc("ntfs_ioctl_fitrim", 1);
  addFunc("ntsync_char_ioctl", 2);
  addFunc("ntsync_obj_ioctl", 2);
  addFunc("ntty_cflags_changed", 1);
  addFunc("ntty_ioctl", 2);
  addFunc("nv_tco_ioctl", 2);
  addFunc("nvgrace_gpu_ioctl", 2);
  addFunc("nvgrace_gpu_ioctl_get_region_info", 1);
  addFunc("nvme_dev_ioctl", 2);
  addFunc("nvme_ioctl", 3);
  addFunc("nvme_ns_chr_ioctl", 2);
  addFunc("nvme_ns_head_chr_ioctl", 2);
  addFunc("nvme_ns_head_ioctl", 3);
  addFunc("nvram_misc_ioctl", 2);
  addFunc("ocfs2_ioctl", 2);
  addFunc("odev_ioctl", 2);
  addFunc("omapfb_ioctl", 2);
  addFunc("orangefs_devreq_ioctl", 2);
  addFunc("packet_ioctl", 2);
  addFunc("pagemap_scan_get_args", 1);
  addFunc("pagemap_scan_writeback_args", 1);
  addFunc("pc87413_ioctl", 2);
  addFunc("pcf2127_rtc_ioctl", 2);
  addFunc("pcf85063_ioctl", 2);
  addFunc("pcf8523_rtc_ioctl", 2);
  addFunc("pcf8563_rtc_ioctl", 2);
  addFunc("pci_endpoint_test_copy", 1);
  addFunc("pci_endpoint_test_ioctl", 2);
  addFunc("pci_endpoint_test_read", 1);
  addFunc("pci_endpoint_test_write", 1);
  addFunc("pcipcwd_ioctl", 2);
  addFunc("pde_ioctl", 3);
  addFunc("perf_ioctl", 2);
  addFunc("pfrt_log_ioctl", 2);
  addFunc("pfru_ioctl", 2);
  addFunc("phantom_ioctl", 2);
  addFunc("pidfd_info", 2);
  addFunc("pidfd_ioctl", 2);
  addFunc("pin_longterm_test_ioctl", 2);
  addFunc("pin_longterm_test_read", 0);
  addFunc("pin_longterm_test_start", 0);
  addFunc("pipe_ioctl", 2);
  addFunc("pkt_ctl_ioctl", 2);
  addFunc("pkt_ioctl", 3);
  addFunc("pmcraid_chr_ioctl", 2);
  addFunc("pn_socket_ioctl", 2);
  addFunc("port_err_ioctl", 3);
  addFunc("port_hdr_ioctl", 3);
  addFunc("port_uint_ioctl", 3);
  addFunc("posix_clock_ioctl", 2);
  addFunc("pp_do_ioctl", 2);
  addFunc("pp_ioctl", 2);
  addFunc("ppp_async_ioctl", 2);
  addFunc("ppp_asynctty_ioctl", 2);
  addFunc("ppp_ioctl", 2);
  addFunc("ppp_sync_ioctl", 2);
  addFunc("ppp_synctty_ioctl", 2);
  addFunc("ppp_unattached_ioctl", 4);
  addFunc("pppoatm_devppp_ioctl", 2);
  addFunc("pppoatm_ioctl", 2);
  addFunc("pppoe_ioctl", 2);
  addFunc("pppol2tp_ioctl", 2);
  addFunc("pppox_ioctl", 2);
  addFunc("pps_cdev_ioctl", 2);
  addFunc("pps_gen_cdev_ioctl", 2);
  addFunc("pptp_ppp_ioctl", 2);
  addFunc("printer_ioctl", 2);
  addFunc("privcmd_ioctl", 2);
  addFunc("proc_bus_pci_ioctl", 2);
  addFunc("proc_reg_unlocked_ioctl", 2);
  addFunc("procfs_procmap_ioctl", 2);
  addFunc("ptp_ioctl", 2);
  addFunc("pty_bsd_ioctl", 2);
  addFunc("pty_unix98_ioctl", 2);
  addFunc("qat_vf_precopy_ioctl", 2);
  addFunc("qrtr_ioctl", 2);
  addFunc("radeon_drm_ioctl", 2);
  addFunc("radeonfb_ioctl", 2);
  addFunc("random_ioctl", 2);
  addFunc("raw_ioctl", 2);
  addFunc("raw_ioctl_configure", 1);
  addFunc("raw_ioctl_ep0_read", 1);
  addFunc("raw_ioctl_ep0_stall", 1);
  addFunc("raw_ioctl_ep0_write", 1);
  addFunc("raw_ioctl_ep_disable", 1);
  addFunc("raw_ioctl_ep_enable", 1);
  addFunc("raw_ioctl_ep_read", 1);
  addFunc("raw_ioctl_ep_set_clear_halt_wedge", 1);
  addFunc("raw_ioctl_ep_write", 1);
  addFunc("raw_ioctl_eps_info", 1);
  addFunc("raw_ioctl_event_fetch", 1);
  addFunc("raw_ioctl_init", 1);
  addFunc("raw_ioctl_run", 1);
  addFunc("raw_ioctl_vbus_draw", 1);
  addFunc("raw_sock_no_ioctlcmd", 2);
  addFunc("rdc321x_wdt_ioctl", 2);
  addFunc("rds_ioctl", 2);
  addFunc("reiserfs_ioctl", 2);
  addFunc("rfcomm_sock_ioctl", 2);
  addFunc("rfcomm_tty_ioctl", 2);
  addFunc("rfkill_fop_ioctl", 2);
  addFunc("riocm_cdev_ioctl", 2);
  addFunc("roccat_ioctl", 2);
  addFunc("rose_ioctl", 2);
  addFunc("rpc_pipe_ioctl", 2);
  addFunc("rpmsg_ctrldev_ioctl", 2);
  addFunc("rpmsg_eptdev_ioctl", 2);
  addFunc("rproc_device_ioctl", 2);
  addFunc("rs5c372_ioctl", 2);
  addFunc("rtc_dev_ioctl", 2);
  addFunc("rv3028_ioctl", 2);
  addFunc("rv3029_ioctl", 2);
  addFunc("rv3032_ioctl", 2);
  addFunc("rv8803_ioctl", 2);
  addFunc("rx8010_ioctl", 2);
  addFunc("rx8111_ioctl", 2);
  addFunc("s35390a_rtc_ioctl", 2);
  addFunc("s3c_fb_ioctl", 2);
  addFunc("sa1100dog_ioctl", 2);
  addFunc("sbefifo_user_ioctl", 2);
  addFunc("sc1200wdt_ioctl", 2);
  addFunc("scarlett2_hwdep_ioctl", 3);
  addFunc("sch311x_wdt_ioctl", 2);
  addFunc("scom_ioctl", 2);
  addFunc("scu_ipc_ioctl", 2);
  addFunc("sd_ioctl", 3);
  addFunc("seccomp_notify_ioctl", 2);
  addFunc("serial_ioctl", 2);
  addFunc("serport_ldisc_ioctl", 2);
  addFunc("set_ctxt_pkey", 1);
  addFunc("sev_ioctl", 2);
  addFunc("sg_ioctl", 2);
  addFunc("sgx_ioctl", 2);
  addFunc("sgx_vepc_ioctl", 2);
  addFunc("sh_mobile_lcdc_ioctl", 2);
  addFunc("sh_mobile_lcdc_overlay_ioctl", 2);
  addFunc("sisfb_ioctl", 2);
  addFunc("sisusb_handle_command", 2);
  addFunc("sisusb_ioctl", 2);
  addFunc("sixpack_ioctl", 2);
  addFunc("slcan_ioctl", 2);
  addFunc("slip_ioctl", 2);
  addFunc("smb2_ioctl_query_info", 5);
  addFunc("smc_ioctl", 2);
  addFunc("snapshot_ioctl", 2);
  addFunc("snd_compr_get_caps", 1);
  addFunc("snd_compr_get_codec_caps", 1);
  addFunc("snd_compr_get_metadata", 1);
  addFunc("snd_compr_get_params", 1);
  addFunc("snd_compr_ioctl", 2);
  addFunc("snd_compr_ioctl_avail", 1);
  addFunc("snd_compr_set_metadata", 1);
  addFunc("snd_compr_set_params", 1);
  addFunc("snd_compr_task_create", 1);
  addFunc("snd_compr_task_seq", 1);
  addFunc("snd_compr_task_seq", 1);
  addFunc("snd_compr_task_start_ioctl", 1);
  addFunc("snd_compr_task_status_ioctl", 1);
  addFunc("snd_compr_tstamp", 1);
  addFunc("snd_ctl_ioctl", 2);
  addFunc("snd_disconnect_ioctl", 2);
  addFunc("snd_emux_ioctl_seq_oss", 2);
  addFunc("snd_hdsp_hwdep_ioctl", 3);
  addFunc("snd_hdspm_hwdep_ioctl", 3);
  addFunc("snd_hwdep_ioctl", 2);
  addFunc("snd_info_entry_ioctl", 2);
  addFunc("snd_mixer_oss_ioctl", 2);
  addFunc("snd_mixer_oss_ioctl1", 2);
  addFunc("snd_mixer_oss_ioctl_card", 2);
  addFunc("snd_opl3_ioctl", 3);
  addFunc("snd_opl3_ioctl_seq_oss", 2);
  addFunc("snd_pcm_ioctl", 2);
  addFunc("snd_pcm_oss_ioctl", 2);
  addFunc("snd_rawmidi_ioctl", 2);
  addFunc("snd_seq_ioctl", 2);
  addFunc("snd_seq_ioctl_client_ump_info", 2);
  addFunc("snd_seq_oss_ioctl", 2);
  addFunc("snd_seq_oss_synth_ioctl", 3);
  addFunc("snd_timer_user_ioctl", 2);
  addFunc("snd_utimer_ioctl", 2);
  addFunc("snp_guest_ioctl", 2);
  addFunc("sock_do_ioctl", 3);
  addFunc("sock_ioctl", 2);
  addFunc("sock_no_ioctl", 2);
  addFunc("sonypi_misc_ioctl", 2);
  addFunc("spidev_ioctl", 2);
  addFunc("sr_block_ioctl", 3);
  addFunc("ssam_cdev_device_ioctl", 2);
  addFunc("sstfb_ioctl", 2);
  addFunc("st_ioctl", 2);
  addFunc("stm_char_ioctl", 2);
  addFunc("subdev_ioctl", 2);
  addFunc("surface_dtx_ioctl", 2);
  addFunc("svc_ioctl", 2);
  addFunc("sw_sync_ioctl", 2);
  addFunc("sw_sync_ioctl_create_fence", 1);
  addFunc("sw_sync_ioctl_get_deadline", 1);
  addFunc("sw_sync_ioctl_inc", 1);
  addFunc("switchtec_dev_ioctl", 2);
  addFunc("sync_file_ioctl", 2);
  addFunc("sync_file_ioctl_fence_info", 1);
  addFunc("sync_file_ioctl_merge", 1);
  addFunc("sync_file_ioctl_set_deadline", 1);
  addFunc("tap_ioctl", 2);
  addFunc("tdx_guest_ioctl", 2);
  addFunc("tee_ioctl", 2);
  addFunc("thread_with_stdio_ioctl", 2);
  addFunc("timerfd_ioctl", 2);
  addFunc("tioclinux", 1);
  addFunc("tipc_ioctl", 2);
  addFunc("toshiba_acpi_ioctl", 2);
  addFunc("tps6594_pfsm_ioctl", 2);
  addFunc("tracing_buffers_ioctl", 2);
  addFunc("tty_ioctl", 2);
  addFunc("tty_jobctrl_ioctl", 4);
  addFunc("tty_mode_ioctl", 2);
  addFunc("tty_perform_flush", 1);
  addFunc("tun_chr_ioctl", 2);
  addFunc("tw_chrdev_ioctl", 2);
  addFunc("twa_chrdev_ioctl", 2);
  addFunc("twl_chrdev_ioctl", 2);
  addFunc("uacce_fops_unl_ioctl", 2);
  addFunc("uart_ioctl", 2);
  addFunc("uart_wait_modem_status", 1);
  addFunc("ubi_cdev_ioctl", 2);
  addFunc("ubifs_ioctl", 2);
  addFunc("udf_ioctl", 2);
  addFunc("udmabuf_ioctl", 2);
  addFunc("udmabuf_ioctl_create", 1);
  addFunc("udmabuf_ioctl_create_list", 1);
  addFunc("ufx_ops_ioctl", 2);
  addFunc("uinput_ioctl", 2);
  addFunc("uinput_ioctl_handler", 2);
  addFunc("unix_ioctl", 2);
  addFunc("usb_pcwd_ioctl", 2);
  addFunc("usb_serial_generic_msr_changed", 1);
  addFunc("usb_serial_generic_tiocmiwait", 1);
  addFunc("usb_stream_hwdep_ioctl", 3);
  addFunc("usbdev_ioctl", 2);
  addFunc("usblp_ioctl", 2);
  addFunc("usbtmc_ioctl", 2);
  addFunc("user_event_ack", 2);
  addFunc("user_events_ioctl", 2);
  addFunc("user_events_ioctl_del", 1);
  addFunc("user_events_ioctl_reg", 1);
  addFunc("user_events_ioctl_unreg", 0);
  addFunc("user_exp_rcv_clear", 1);
  addFunc("user_exp_rcv_invalid", 1);
  addFunc("user_exp_rcv_setup", 1);
  addFunc("userfaultfd_api", 1);
  addFunc("userfaultfd_continue", 1);
  addFunc("userfaultfd_copy", 1);
  addFunc("userfaultfd_dev_ioctl", 2);
  addFunc("userfaultfd_ioctl", 2);
  addFunc("userfaultfd_move", 1);
  addFunc("userfaultfd_poison", 1);
  addFunc("userfaultfd_register", 1);
  addFunc("userfaultfd_unregister", 1);
  addFunc("userfaultfd_wake", 1);
  addFunc("userfaultfd_writeprotect", 1);
  addFunc("userfaultfd_zeropage", 1);
  addFunc("uv_mmtimer_ioctl", 2);
  addFunc("v4l2_ioctl", 2);
  addFunc("vbg_misc_device_ioctl", 2);
  addFunc("vcc_ioctl", 2);
  addFunc("vchiq_ioctl", 2);
  addFunc("vduse_dev_ioctl", 2);
  addFunc("vduse_ioctl", 2);
  addFunc("vfio_container_ioctl_check_extension", 1);
  addFunc("vfio_device_fops_unl_ioctl", 2);
  addFunc("vfio_fops_unl_ioctl", 2);
  addFunc("vfio_fsl_mc_ioctl", 2);
  addFunc("vfio_group_fops_unl_ioctl", 2);
  addFunc("vfio_ioctl_set_iommu", 1);
  addFunc("vfio_iommu_type1_dirty_pages", 1);
  addFunc("vfio_iommu_type1_get_info", 1);
  addFunc("vfio_iommu_type1_ioctl", 2);
  addFunc("vfio_iommu_type1_map_dma", 1);
  addFunc("vfio_iommu_type1_open", 0);
  addFunc("vfio_iommu_type1_unmap_dma", 1);
  addFunc("vfio_noiommu_ioctl", 2);
  addFunc("vfio_noiommu_open", 0);
  addFunc("vfio_pci_core_ioctl", 2);
  addFunc("vfio_platform_ioctl", 2);
  addFunc("vhost_net_ioctl", 2);
  addFunc("vhost_scsi_ioctl", 2);
  addFunc("vhost_vdpa_unlocked_ioctl", 2);
  addFunc("vhost_vsock_dev_ioctl", 2);
  addFunc("viafb_ioctl", 2);
  addFunc("viafb_ioctl_get_viafb_info", 0);
  addFunc("video_ioctl2", 2);
  addFunc("video_usercopy", 2);
  addFunc("virtiovf_pci_ioctl_get_region_info", 2);
  addFunc("virtiovf_precopy_ioctl", 2);
  addFunc("virtiovf_vfio_pci_core_ioctl", 2);
  addFunc("virtual_ncidev_ioctl", 2);
  addFunc("vivid_fb_ioctl", 2);
  addFunc("vmci_host_unlocked_ioctl", 2);
  addFunc("vme_user_ioctl", 3);
  addFunc("vme_user_unlocked_ioctl", 2);
  addFunc("vmw_generic_ioctl", 2);
  addFunc("vmw_unlocked_ioctl", 2);
  addFunc("vol_cdev_ioctl", 2);
  addFunc("vsock_dev_ioctl", 2);
  addFunc("vsock_ioctl", 2);
  addFunc("vt_do_kdskled", 2);
  addFunc("vt_ioctl", 2);
  addFunc("vt_k_ioctl", 2);
  addFunc("vt_kdsetmode", 1);
  addFunc("vtpmx_fops_ioctl", 2);
  addFunc("vtpmx_ioc_new_dev", 2);
  addFunc("wafwdt_ioctl", 2);
  addFunc("wait_serial_change", 1);
  addFunc("watchdog_ioctl", 2);
  addFunc("watchdog_ioctl_op", 2);
  addFunc("wb_smsc_wdt_ioctl", 2);
  addFunc("wdm_ioctl", 2);
  addFunc("wdt_ioctl", 2);
  addFunc("wdt_unlocked_ioctl", 2);
  addFunc("wdtpci_ioctl", 2);
  addFunc("wwan_port_fops_at_ioctl", 2);
  addFunc("wwan_port_fops_ioctl", 2);
  addFunc("x25_ioctl", 2);
  addFunc("xe_drm_ioctl", 2);
  addFunc("xe_eu_stall_stream_ioctl", 2);
  addFunc("xe_eu_stall_stream_ioctl_locked", 2);
  addFunc("xe_oa_add_config_ioctl", 1);
  addFunc("xe_oa_config_locked", 1);
  addFunc("xe_oa_info_locked", 1);
  addFunc("xe_oa_ioctl", 2);
  addFunc("xe_oa_ioctl_locked", 2);
  addFunc("xe_oa_remove_config_ioctl", 1);
  addFunc("xe_oa_status_locked", 1);
  addFunc("xe_oa_stream_open_ioctl", 1);
  addFunc("xe_oa_user_extensions", 2);
  addFunc("xen_mce_chrdev_ioctl", 2);
  addFunc("xenbus_backend_ioctl", 2);
  addFunc("xfs_file_ioctl", 2);
  addFunc("xr_ioctl", 2);
  addFunc("xsdfec_dev_ioctl", 2);
  addFunc("zf_ioctl", 2);
  addFunc("scarlett2_ioctl_select_flash_segment", 1);
  addFunc("scarlett2_ioctl_get_erase_progress", 1);
}

/*
 * Create a fixup that replaces the type of a declartion with
 * "user_uintptr_t". Due to very rare false positives resulting
 * from duplicate function names no fixup is created if the
 * current declaration has pointer type.
 */
void IoctlCheck::createTypeFixup(const DeclaratorDecl *Decl,
                                 SourceManager *SM) {
  if (Decl->getType()->isPointerType())
    return;

  auto Begin = Decl->getTypeSpecStartLoc();
  auto End = Decl->getTypeSpecEndLoc();

  diag(Decl->getLocation(), "CHERI: Replace type with 'user_uintptr_t'")
      << FixItHint::CreateReplacement(
             CharSourceRange::getTokenRange(Begin, End), "user_uintptr_t");
}

void IoctlCheck::createArgFixup(const Expr *Arg, SourceManager *SM) {
  auto S = Arg->getExprLoc();
  auto E = Lexer::getLocForEndOfToken(S, 0, *SM, getLangOpts());

  diag(Arg->getExprLoc(), "CHERI: Wrap argument in __c_ua(...)")
      << FixItHint::CreateInsertion(S, "__c_ua(")
      << FixItHint::CreateInsertion(E, ")");
}

/*
 * Check the function declaration of an ioctl function and complain
 * if the formal parameter that contains the used pointer does not
 * have the correct type.
 */
void IoctlCheck::checkFunctionDecl(
    const ast_matchers::MatchFinder::MatchResult &Result,
    const FunctionDecl *Func) {
  auto F = Ioctl::findFunc(Func->getName());

  if (!F)
    return;

  if (Ioctl::isBlacklisted(Func->getName())) {
    diag(Func->getLocation(), "CHERI: ERROR: Ioctl function also on blacklist");
    return;
  }

  if (!F) {
    diag(Func->getLocation(), "CHERI: ERROR: Ioctl function mismatch for '%0'")
        << Func->getName();
    return;
  }

  if (F->PtrArg_ >= Func->getNumParams()) {
    diag(Func->getLocation(),
         "CHERI: Ioctl function '%0' should have at least %1 parameter(s)!")
        << Func->getName() << (F->PtrArg_ + 1);
    return;
  }

  auto P = Func->getParamDecl(F->PtrArg_);
  if (P->getType().getUnqualifiedType().getAsString() !=
      StringRef("user_uintptr_t")) {
    diag(P->getLocation(), "CHERI: Parameter %0 of ioctl function '%1' should "
                           "have type 'user_uintptr_t'")
        << (F->PtrArg_ + 1) << Func->getName();
    /*
     * Hack:
     * Do not create a fixup for extern typeof(func_name) func_name.
     * which happens in EXPORT_SYMBOL(). The quick and dirty way to
     * detect this is that the location of the ParamDecl is the same
     * as the location of the FunctionDecl itself.
     */
    if (P->getLocation() != Func->getLocation())
      createTypeFixup(P, Result.SourceManager);
  }
}

/*
 * Check the type of an ioctl structure field and complain if
 * the formal argument that holds the user pointer does not have
 * the correct type.
 */
void IoctlCheck::checkFieldDecl(
    const ast_matchers::MatchFinder::MatchResult &Result,
    const FieldDecl *Field) {
  const auto *Rec = Result.Nodes.getNodeAs<RecordDecl>("record");

  if (!Rec) {
    diag(Field->getLocation(), "CHERI: ERROR: No RecordDecl found!");
    return;
  }

  auto F = Ioctl::findField(Rec->getName(), Field->getName());
  if (!F) {
    diag(Field->getLocation(), "CHERI: ERROR: Field is not an ioctl field");
    return;
  }

  const auto *FT = Util::extractFunctionProtoType(Field);
  if (!FT) {
    diag(Field->getLocation(), "CHERI: Ioctl field '%0' in '%1' should have "
                               "function pointer type instead of '%2'")
        << F->FieldName_ << F->StructName_ << Field->getType().getAsString();
    return;
  }

  if (F->PtrArg_ >= FT->getNumParams()) {
    diag(Field->getLocation(),
         "CHERI: Ioctl field '%0' should have at least %1 parameter(s)!")
        << F->FieldName_ << (F->PtrArg_ + 1);
    return;
  }

  if (FT->getParamType(F->PtrArg_).getUnqualifiedType().getAsString() !=
      StringRef("user_uintptr_t")) {
    diag(Field->getLocation(), "CHERI: Argument %0 of ioctl field '%1' should "
                               "have type user_uintptr_t")
        << (F->PtrArg_ + 1) << F->FieldName_;
    auto Loc = Field->getTypeSourceInfo()->getTypeLoc();
    /* Create the fixup inline types. */
    bool DerefDone = false;
    while (1) {
      if (Loc.getAs<FunctionTypeLoc>())
        break;
      if (auto PLoc = Loc.getAs<ParenTypeLoc>()) {
        Loc = PLoc.getInnerLoc();
        continue;
      }
      if (!DerefDone) {
        if (auto PLoc = Loc.getAs<PointerTypeLoc>()) {
          Loc = PLoc.getPointeeLoc();
          DerefDone = true;
          continue;
        }
      }
      break;
    }
    if (auto FLoc = Loc.getAs<FunctionTypeLoc>()) {
      if (F->PtrArg_ < FLoc.getNumParams()) {
        const auto *P = FLoc.getParam(F->PtrArg_);
        if (P)
          createTypeFixup(P, Result.SourceManager);
      }
    }
  }
}

/*
 * Given an expression determine if the expression directly
 * references a function declaration and if so store that
 * declaration in @Decl.
 * Otherwise set @Decl to nullptr.
 */
void IoctlCheck::extractFunctionDecl(const Expr *E, const FunctionDecl *&Decl) {
  Decl = nullptr;
  while (1) {
    if (auto C = dyn_cast<CastExpr>(E)) {
      E = C->getSubExpr();
      continue;
    }
    if (auto P = dyn_cast<ParenExpr>(E)) {
      E = P->getSubExpr();
      continue;
    }
    break;
  }

  if (const auto DR = dyn_cast<DeclRefExpr>(E))
    if (const auto D = DR ? DR->getDecl() : nullptr)
      Decl = dyn_cast_if_present<FunctionDecl>(D);
}

/*
 * Check if the expression is a member expression that references
 * a structure field. If so return the containing record in
 * @RecordDecl and the field in @Field.
 * Otherwise set both @RecordDecl and @Field to nullptr.
 */
void IoctlCheck::extractFieldDecl(const Expr *E, const RecordDecl *&Record,
                                  const FieldDecl *&Field) {
  Record = nullptr;
  Field = nullptr;

  while (1) {
    if (auto C = dyn_cast<CastExpr>(E)) {
      E = C->getSubExpr();
      continue;
    }
    if (auto P = dyn_cast<ParenExpr>(E)) {
      E = P->getSubExpr();
      continue;
    }
    break;
  }

  const auto *M = dyn_cast<MemberExpr>(E);
  if (!M)
    return;
  const auto *F = dyn_cast_if_present<FieldDecl>(M->getMemberDecl());
  if (!F)
    return;
  const auto *T = M->getBase()->getType().getTypePtr();
  if (!T)
    return;
  if (M->isArrow()) {
    const auto *PT = dyn_cast<PointerType>(T);
    if (!PT)
      return;
    T = PT->getPointeeType().getTypePtr();
  }
  if (!T || !T->isRecordType())
    return;
  auto RD = T->getAsRecordDecl();

  if (RD && F) {
    Record = RD;
    Field = F;
  }
}

/*
 * Check an expression that is used to initialize an ioctl field.
 * Complain if the value used to initialize the field is a function
 * that is not an ioctl function. Additionally, complain if the
 * function is an ioctl function but with a different index of
 * the user_uintptr_t field.
 */
void IoctlCheck::checkFieldInitializer(const Expr *I, const RecordDecl *Record,
                                       const FieldDecl *Field) {
  const FunctionDecl *Decl;

  extractFunctionDecl(I, Decl);
  if (!Decl)
    return;
  const Ioctl::Field *M =
      Record ? Ioctl::findField(Record->getName(), Field->getName()) : nullptr;
  if (!Record || !Field || !M) {
    diag(I->getExprLoc(), "ERROR: Cannot extract struct type and field name");
    return;
  }
  auto F = Ioctl::findFunc(Decl->getName());
  if (!F) {
    diag(I->getExprLoc(),
         "CHERI: Initialization of "
         "ioctl field '%0' in '%1' with non-ioctl function '%2'")
        << M->FieldName_ << M->StructName_ << Decl->getName();
    if (!Ioctl::isBlacklisted(Decl->getName()))
      diag(I->getExprLoc(), "MISSING: addFunc(\"%0\", %1)")
          << Decl->getName() << M->PtrArg_;
  } else if (F->PtrArg_ != M->PtrArg_) {
    diag(I->getExprLoc(), "CHERI: Different pointer argument "
                          "positions in initialization of ioctl field '%0' "
                          "in '%1' with non-ioctl function '%2'")
        << M->FieldName_ << M->StructName_ << F->Name_;
  }
}

/*
 * Check an initializer list for a structure containing ioctl fields.
 * Identifiy the initializer expressions for all ioctl fields in
 * the structure and check them unsing checkFieldInitializer().
 */
void IoctlCheck::checkInitList(const InitListExpr *InitList) {
  if (InitList->isSyntacticForm()) {
    InitList = InitList->getSemanticForm();
    if (!InitList)
      return;
  }
  auto T = InitList->getType().getTypePtr();
  if (!T->isRecordType())
    return;
  auto RD = T->getAsRecordDecl();
  if (!RD) {
    diag(InitList->getExprLoc(), "CHERI: ERROR: Record without a declaration?");
    return;
  }

  const auto *FL = Ioctl::findField(RD->getName());
  if (!FL)
    return;
  for (auto M : *FL) {
    for (const auto F : RD->fields()) {
      if (F->getName() != StringRef(M.FieldName_))
        continue;
      if (F->getFieldIndex() >= InitList->getNumInits())
        continue;
      auto I = InitList->getInit(F->getFieldIndex());
      if (!I || isa<ImplicitValueInitExpr>(I))
        continue;
      checkFieldInitializer(I, RD, F);
    }
  }
}

/*
 * Iterate through the various actual arguments of a CallExpr and
 * call checkCallWithArg() for each of them.
 */
void IoctlCheck::checkCallWithArgs(
    const ast_matchers::MatchFinder::MatchResult &Result,
    const CallExpr *Call) {
  for (unsigned int i = 0; i < Call->getNumArgs(); ++i)
    checkCallWithArg(Result, Call, Call->getArg(i));
}

/*
 * Check the argument of a CallExpr within another outer function.
 * First filter out cases where:
 * - The outer function is not in ioctl function at all
 * - The actual argument in question is not just one of the formal
 *   parameters of the outer function.
 * - The formal parameter of the outer function that is forwarded
 *   is not the ioctl pointer argument.
 * Complain if the called entity is
 * - a function but not an ioctl function
 * - a structure field that is not an ioctl field.
 * Additionally, complain if the argument position does not match.
 */
void IoctlCheck::checkCallWithArg(
    const ast_matchers::MatchFinder::MatchResult &Result, const CallExpr *Call,
    const Expr *Arg) {
  auto Ctx = Result.Context;
  const auto *Outer = Result.Nodes.getNodeAs<FunctionDecl>("outer");
  const ParmVarDecl *Param = nullptr;
  const FunctionDecl *Decl;
  const RecordDecl *Record;
  const FieldDecl *Field;
  const Expr *Tmp;

  if (!Outer) {
    diag(Arg->getExprLoc(), "CHERI: ERROR: Inconsistent match");
    return;
  }

  /* Ignore anything that is not in an ioctl function. */
  const auto *F = Ioctl::findFunc(Outer->getName());
  if (!F)
    return;

  /*
   * Remove implicit casts and the DeclRefExpr to find the ParamVarDecl
   * of the forwarded formal parameter (if any).
   */
  Tmp = Arg;
  while (1) {
    if (const auto *Ref = dyn_cast<DeclRefExpr>(Tmp)) {
      Param = dyn_cast<ParmVarDecl>(Ref->getDecl());
      break;
    }
    if (const auto *Paren = dyn_cast<ParenExpr>(Tmp)) {
      Tmp = Paren->getSubExpr();
      continue;
    }
    if (const auto *Impl = dyn_cast<ImplicitCastExpr>(Tmp)) {
      Tmp = Impl->getSubExpr();
      continue;
    }
    return;
  }
  if (!Param)
    return;

  /*
   * Ignore if the parameter that is forwarded is not the ioctl
   * pointer arg.
   */
  if (F->PtrArg_ >= Outer->getNumParams())
    return;
  if (Outer->getParamDecl(F->PtrArg_) != Param)
    return;

  if (Ioctl::isBlacklisted(Outer->getName())) {
    diag(Outer->getLocation(),
         "CHERI: ERROR: Ioctl function also on blacklist");
    return;
  }

  /* Determine the index of the parameter in the call. */
  Tmp = Arg;
  while (1) {
    const auto &Parents = Ctx->getParents(*Tmp);
    const Expr *PE = nullptr;
    if (!Parents.empty())
      PE = Parents[0].get<Expr>();
    if (!PE) {
      diag(Arg->getExprLoc(),
           "CHERI: ERROR: Cannot determine index of call arg");
      return;
    }
    if (PE == Call)
      break;
    Tmp = PE;
  }
  unsigned int ArgPos;
  const Expr *TheArg = nullptr;
  for (unsigned int i = 0; i < Call->getNumArgs(); ++i) {
    if (Call->getArg(i) == Tmp) {
      ArgPos = i;
      TheArg = Tmp;
      break;
    }
  }
  if (!TheArg) {
    diag(Arg->getExprLoc(),
         "CHERI: ERROR: Argument not found in Argument List");
    return;
  }

  /*
   * The argument is passed to a function but cast to a smaller
   * integer or enumeration type. Do not claim that the target
   * function is an ioctl function. We may need to refine this
   * along the lines of the PtrToIntCast check if is not precise
   * enough.
   */
  const auto *ArgT =
      Ctx->getCanonicalType(TheArg->getType()->getUnqualifiedDesugaredType());
  if (ArgT->isEnumeralType())
    return;
  if (const auto *BT = dyn_cast<BuiltinType>(ArgT)) {
    switch (BT->getKind()) {
    case BuiltinType::Bool:
    case BuiltinType::Char_S:
    case BuiltinType::Char_U:
    case BuiltinType::SChar:
    case BuiltinType::UChar:
    case BuiltinType::Short:
    case BuiltinType::UShort:
    case BuiltinType::Int:
    case BuiltinType::UInt:
    case BuiltinType::Char8:
    case BuiltinType::Char16:
    case BuiltinType::Char32:
    case BuiltinType::WChar_S:
    case BuiltinType::WChar_U:
      return;
    default:
      break;
    }
  }
  if (const auto *BitT = dyn_cast<BitIntType>(ArgT)) {
    if (BitT->getNumBits() < 64)
      return;
  }

  extractFunctionDecl(Call->getCallee(), Decl);
  if (Decl) {
    /*
     * An ioctl argument passed to a parameter of a variadic function
     * is likely a call to something like printf(...). In any case we
     * cannot assume that the function is an ioctl function.
     */
    if (Decl->isVariadic() && ArgPos >= Decl->getNumParams())
      return;

    /* Do not warn for explicit cast with __c_ua() */
    if (Decl->getName() == StringRef("__c_ua"))
      return;

    const auto *CF = Ioctl::findFunc(Decl->getName());
    if (!CF || CF->PtrArg_ != ArgPos) {
      diag(Arg->getExprLoc(),
           "CHERI: Ioctl pointer parameter '%0' of ioctl function '%1' "
           "used as argument %2 in call to non-ioctl function '%3'")
          << Param->getName() << F->Name_ << (ArgPos + 1) << Decl->getName();
      if (!Ioctl::isBlacklisted(Decl->getName()))
        diag(Arg->getExprLoc(), "MISSING: addFunc(\"%0\", %1)")
            << Decl->getName() << ArgPos;
      else
        createArgFixup(Arg, Result.SourceManager);
    }
  }

  extractFieldDecl(Call->getCallee(), Record, Field);
  if (Record && Field) {
    const auto *M = Ioctl::findField(Record->getName(), Field->getName());
    if (!M || M->PtrArg_ != ArgPos) {
      diag(Arg->getExprLoc(),
           "CHERI: Ioctl pointer parameter '%0' of ioctl function '%1' "
           "used as argument %2 "
           "in call to non-ioctl function pointed to by field '%3' in '%4'")
          << Param->getName() << F->Name_ << (ArgPos + 1) << Field->getName()
          << Record->getName();
      diag(Arg->getExprLoc(), "MISSING: addField(\"%0\", \"%1\", %2)")
          << Record->getName() << Field->getName() << ArgPos;
    }
  } else if (!Decl) {
    diag(Arg->getExprLoc(),
         "CHERI: Ioctl pointer parameter '%0' of ioctl function '%1' "
         "used as argument %2 in a call to an unknown object")
        << Param->getName() << F->Name_ << (ArgPos + 1);
  }
}

/*
 * Register the various matchers for ioctl functions, ioctl fields
 * initializers and calls.
 */
void IoctlCheck::registerMatchers(MatchFinder *Finder) {
  /*
   * Match all function declarations. We will only look at ioctl
   * functions in the check() routine.
   */
  Finder->addMatcher(functionDecl().bind("func"), this);

  /*
   * Matches all CallExpr within the body of another function where
   * one of the parameters of the outer function is forwarded verbatim
   * to the callee. The check() function will ignore the match if the
   * outer function is not an ioctl function or the parameter is not
   * one of the ioctl parameters.
   * NOTE:
   * The hasAnyArgument() matcher will only match once even if many
   * parameters of outer function are used verbatim in the same
   * CallExpr. We would like to use forEachArgumentWithParmType()
   * but that apparently matches on any subexpression of the actual
   * argument expression. Thus the check() function must examine all
   * actual arguments of the CallExpr.
   */
  // clang-format off
  Finder->addMatcher(
      functionDecl(
          hasBody(
              forEachDescendant(
                  callExpr(
                      hasAnyArgument(
                          traverse(
                              TK_IgnoreUnlessSpelledInSource,
                              declRefExpr(
                                  to(
                                      parmVarDecl()
                                  )
                              )
                          )
                      )
                  ).bind("call")
              )
          )
      ).bind("outer"),
      this
  );
  // clang-format on

  for (const auto &It : Ioctl::fields()) {
    for (const auto &M : It.second) {
      /* Declaration of a record with an ioctl field. */
      // clang-format off
      Finder->addMatcher(
          recordDecl(
              hasName(M.StructName_),
              has(
                  fieldDecl(
                      hasName(M.FieldName_)
                  ).bind("field")
              )
          ).bind("record"),
          this
      );

      /* Initialization of a struct with an ioctl field. */
      Finder->addMatcher(
          initListExpr(
              hasType(
                  recordDecl(
                      hasName(M.StructName_)
                  )
              )
          ).bind("initlist"),
          this
      );

      /* Assignment to an ioctl field with "->field = ...". */
      Finder->addMatcher(
          binaryOperator(
              hasOperatorName("="),
              hasLHS(
                  memberExpr(
                      hasObjectExpression(
                          hasType(
                              pointsTo(
                                  recordDecl(
                                      hasName(M.StructName_)
                                  )
                              )
                          )
                      ),
                      hasDeclaration(
                          fieldDecl(
                              hasName(M.FieldName_)
                          )
                      )
                  )
              )
          ).bind("assign"),
          this
      );

      /* Assignment to an ioctl field with ".field = ..." */
      Finder->addMatcher(
          binaryOperator(
              hasOperatorName("="),
              hasLHS(
                  memberExpr(
                      hasObjectExpression(
                          hasType(
                              recordDecl(
                                  hasName(M.StructName_)
                              )
                          )
                      ),
                      hasDeclaration(
                          fieldDecl(
                              hasName(M.FieldName_)
                          )
                      )
                  )
              )
          ).bind("assign"),
          this
      );
      // clang-format on
    }
  }
}

/*
 * Inspect a match and delegate it to the correct subfunction for checking.
 */
void IoctlCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func")) {
    checkFunctionDecl(Result, Func);
  }
  if (const auto *Field = Result.Nodes.getNodeAs<FieldDecl>("field"))
    checkFieldDecl(Result, Field);
  if (const auto *InitList = Result.Nodes.getNodeAs<InitListExpr>("initlist"))
    checkInitList(InitList);
  if (const auto *E = Result.Nodes.getNodeAs<BinaryOperator>("assign")) {
    const RecordDecl *Record;
    const FieldDecl *Field;
    extractFieldDecl(E->getLHS(), Record, Field);
    checkFieldInitializer(E->getRHS(), Record, Field);
  }
  if (const auto *C = Result.Nodes.getNodeAs<CallExpr>("call"))
    checkCallWithArgs(Result, C);
}

} // namespace clang::tidy::cheri
