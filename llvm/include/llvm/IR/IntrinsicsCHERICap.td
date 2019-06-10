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


def int_cheri_cap_length_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_base_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_cheri_cap_perms_and :
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_anyint_ty],
            [IntrNoMem]>;
def int_cheri_cap_perms_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_flags_set :
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_anyint_ty],
            [IntrNoMem]>;
def int_cheri_cap_flags_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_bounds_set :
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_anyint_ty],
            [IntrNoMem]>;
def int_cheri_cap_bounds_set_exact :
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_anyint_ty],
            [IntrNoMem]>;
def int_cheri_cap_type_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_tag_get  :
  GCCBuiltinAlias<"__builtin_mips_cheri_get_cap_tag">,
  GCCBuiltin<"__builtin_cheri_tag_get">,
  Intrinsic<[llvm_i1_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_sealed_get  :
  GCCBuiltinAlias<"__builtin_mips_cheri_get_cap_sealed">,
  GCCBuiltin<"__builtin_cheri_sealed_get">,
  Intrinsic<[llvm_i1_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_cheri_cap_tag_clear  :
  GCCBuiltinAlias<"__builtin_mips_cheri_clear_cap_tag">,
  GCCBuiltin<"__builtin_cheri_tag_clear">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty],
            [IntrNoMem]>;

def int_cheri_cap_seal :
  GCCBuiltinAlias<"__builtin_mips_cheri_seal_cap">,
  GCCBuiltin<"__builtin_cheri_seal">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_conditional_seal :
  GCCBuiltin<"__builtin_cheri_conditional_seal">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_unseal  :
  GCCBuiltinAlias<"__builtin_mips_cheri_unseal_cap">,
  GCCBuiltin<"__builtin_cheri_unseal">,
  Intrinsic<[llvm_fatptr_ty],
            [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem]>;
def int_cheri_cap_perms_check :
  Intrinsic<[], [llvm_fatptr_ty, llvm_anyint_ty],
            [IntrNoMem, IntrHasSideEffects]>;
def int_cheri_cap_type_check :
  GCCBuiltinAlias<"__builtin_mips_cheri_check_type">,
  GCCBuiltin<"__builtin_cheri_type_check">,
  Intrinsic<[], [llvm_fatptr_ty, llvm_fatptr_ty],
            [IntrNoMem, IntrHasSideEffects]>;
def int_cheri_cap_subset_test:
  GCCBuiltin<"__builtin_cheri_subset_test">,
  Intrinsic<[llvm_i1_ty], [llvm_fatptr_ty, llvm_fatptr_ty], [IntrNoMem]>;

def int_cheri_stack_cap_get :
  GCCBuiltin<"__builtin_cheri_stack_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;
def int_cheri_ddc_get :
  GCCBuiltinAlias<"__builtin_mips_cheri_get_global_data_cap">,
  GCCBuiltin<"__builtin_cheri_global_data_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;
def int_cheri_pcc_get :
  GCCBuiltinAlias<"__builtin_mips_cheri_get_program_counter_cap">,
  GCCBuiltin<"__builtin_cheri_program_counter_get">,
  Intrinsic<[llvm_fatptr_ty], [], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Offset-related intrinsics
////////////////////////////////////////////////////////////////////////////////

def int_cheri_cap_offset_increment:
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_anyint_ty], [IntrNoMem]>;
def int_cheri_cap_offset_set:
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_anyint_ty], [IntrNoMem]>;
def int_cheri_cap_offset_get:
  Intrinsic<[llvm_anyint_ty], [llvm_fatptr_ty], [IntrNoMem]>;
def int_cheri_cap_diff:
  Intrinsic<[llvm_anyint_ty], [llvm_fatptr_ty,llvm_fatptr_ty], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Pointer / capability interoperation intrinsics
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_address_get:
  Intrinsic<[llvm_anyint_ty], [llvm_fatptr_ty], [IntrNoMem]>;
def int_cheri_cap_address_set:
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_anyint_ty], [IntrNoMem]>;


def int_cheri_cap_to_pointer:
  Intrinsic<[llvm_anyint_ty], [llvm_fatptr_ty, llvm_fatptr_ty], [IntrNoMem]>;
def int_cheri_cap_from_pointer:
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_anyint_ty], [IntrNoMem]>;
def int_cheri_cap_from_ddc:
  Intrinsic<[llvm_fatptr_ty], [llvm_anyint_ty], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Building capabilities from untagged data
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_build:
  GCCBuiltin<"__builtin_cheri_cap_build">,
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_fatptr_ty], [IntrNoMem]>;
def int_cheri_cap_type_copy:
  GCCBuiltin<"__builtin_cheri_cap_type_copy">,
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_fatptr_ty], [IntrNoMem]>;

////////////////////////////////////////////////////////////////////////////////
// Tag memory accessor functions
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_load_tags:
  Intrinsic<[llvm_anyint_ty], [llvm_fatptr_ty], [IntrReadMem]>;

/// Round length to compressed capability precision:
def int_cheri_round_representable_length :
   GCCBuiltin<"__builtin_cheri_round_representable_length">,
   Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrNoMem]>;
def int_cheri_representable_alignment_mask :
   GCCBuiltin<"__builtin_cheri_representable_alignment_mask">,
   Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrNoMem]>;


/// This intrinsics makes it easier to avoid spilling registers to create
/// bounded capabilities for the stack (such as when passing a stack buffer to
/// another function, etc)
def int_cheri_bounded_stack_cap :
  Intrinsic<[llvm_fatptr_ty], [llvm_fatptr_ty, llvm_anyint_ty], [IntrNoMem]>;
