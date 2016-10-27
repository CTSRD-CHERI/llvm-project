//===- IntrinsicsMips.td - Defines Mips intrinsics ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the memory-capability-specific intrinsics.
//
//===----------------------------------------------------------------------===//


def int_memcap_cap_length_get :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_length">,
  GCCBuiltin<"__builtin_memcap_length_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_base_get :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_base">,
  GCCBuiltin<"__builtin_memcap_base_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_memcap_cap_perms_and :
  GCCBuiltinAlias<"__builtin_cheri_and_cap_perms">,
  GCCBuiltin<"__builtin_memcap_perms_and">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_i64_ty],
            [IntrNoMem]>;
def int_memcap_cap_perms_get :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_perms">,
  GCCBuiltin<"__builtin_memcap_perms_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_bounds_set :
  GCCBuiltin<"__builtin_memcap_bounds_set">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_i64_ty],
            [IntrNoMem]>;
def int_memcap_cap_type_get :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_type">,
  GCCBuiltin<"__builtin_memcap_type_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_tag_get  :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_tag">,
  GCCBuiltin<"__builtin_memcap_tag_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_sealed_get  :
  GCCBuiltinAlias<"__builtin_cheri_get_cap_sealed">,
  GCCBuiltin<"__builtin_memcap_sealed_get">,
  Intrinsic<[llvm_i64_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_memcap_cap_tag_clear  :
  GCCBuiltinAlias<"__builtin_cheri_clear_cap_tag">,
  GCCBuiltin<"__builtin_memcap_tag_clear">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_memcap_cap_seal :
  GCCBuiltinAlias<"__builtin_cheri_seal_cap">,
  GCCBuiltin<"__builtin_memcap_seal">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_unseal  :
  GCCBuiltinAlias<"__builtin_cheri_unseal_cap">,
  GCCBuiltin<"__builtin_memcap_unseal">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem]>;
def int_memcap_cap_perms_check :
  GCCBuiltinAlias<"__builtin_cheri_check_perms">,
  GCCBuiltin<"__builtin_memcap_perms_check">,
  Intrinsic<[], [llvm_fatptr_ty, llvm_i64_ty], []>;
def int_memcap_cap_type_check :
  GCCBuiltinAlias<"__builtin_cheri_check_type">,
  GCCBuiltin<"__builtin_memcap_type_check">,
  Intrinsic<[], [llvm_fatptr_ty, llvm_fatptr_ty], []>;

def int_memcap_stack_cap_get :
  GCCBuiltin<"__builtin_memcap_stack_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;
def int_memcap_ddc_get :
  GCCBuiltinAlias<"__builtin_cheri_get_global_data_cap">,
  GCCBuiltin<"__builtin_memcap_global_data_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;
def int_memcap_pcc_get :
  GCCBuiltinAlias<"__builtin_cheri_get_program_counter_cap">,
  GCCBuiltin<"__builtin_memcap_program_counter_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Offset-related intrinsics
////////////////////////////////////////////////////////////////////////////////

def int_memcap_cap_offset_increment:
  GCCBuiltinAlias<"__builtin_cheri_cap_offset_increment">,
  GCCBuiltin<"__builtin_memcap_offset_increment">,
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_i64_ty], [IntrNoMem]>;
def int_memcap_cap_offset_set:
  GCCBuiltinAlias<"__builtin_cheri_cap_offset_set">,
  GCCBuiltin<"__builtin_memcap_offset_set">,
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_i64_ty], [IntrNoMem]>;
def int_memcap_cap_offset_get:
  GCCBuiltinAlias<"__builtin_cheri_cap_offset_get">,
  GCCBuiltin<"__builtin_memcap_offset_get">,
  Intrinsic<[llvm_i64_ty], [llvm_fatptr_ty], [IntrNoMem]>;
def int_memcap_cap_diff:
  Intrinsic<[llvm_i64_ty], [llvm_fatptr_ty,llvm_fatptr_ty], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Pointer / capability interoperation intrinsics
////////////////////////////////////////////////////////////////////////////////

def int_memcap_cap_to_pointer:
  Intrinsic<[llvm_i64_ty], [llvm_fatptr_ty, llvm_fatptr_ty], [IntrNoMem]>;
def int_memcap_cap_from_pointer:
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_i64_ty], [IntrNoMem]>;

