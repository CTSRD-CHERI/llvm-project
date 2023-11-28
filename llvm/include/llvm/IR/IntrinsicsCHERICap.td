//===- IntrinsicsCHERICap.td - Defines CHERI intrinsics ----*- tablegen -*-===//
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
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_base_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_copy_from_high :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_copy_to_high :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;

def int_cheri_cap_perms_and :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_perms_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_flags_set :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_flags_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_high_set :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_high_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_bounds_set :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_bounds_set_exact :
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_type_get :
  Intrinsic<[llvm_anyint_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_tag_get  :
  ClangBuiltin<"__builtin_cheri_tag_get">,
  Intrinsic<[llvm_i1_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_sealed_get  :
  ClangBuiltin<"__builtin_cheri_sealed_get">,
  Intrinsic<[llvm_i1_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;

def int_cheri_cap_tag_clear  :
  ClangBuiltin<"__builtin_cheri_tag_clear">,
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;

def int_cheri_cap_seal :
  ClangBuiltin<"__builtin_cheri_seal">,
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_conditional_seal :
  ClangBuiltin<"__builtin_cheri_conditional_seal">,
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_unseal  :
  ClangBuiltin<"__builtin_cheri_unseal">,
  Intrinsic<[llvm_cap_ty],
            [llvm_cap_ty, llvm_cap_ty],
            [IntrNoMem, IntrWillReturn]>;
 def int_cheri_cap_seal_entry:
   ClangBuiltin<"__builtin_cheri_seal_entry">,
   Intrinsic<[llvm_cap_ty], [llvm_cap_ty],
    [IntrNoMem, IntrWillReturn]>;

def int_cheri_cap_perms_check :
  Intrinsic<[], [llvm_cap_ty, llvm_anyint_ty],
            [IntrNoMem, IntrHasSideEffects, IntrWillReturn]>;
def int_cheri_cap_type_check :
  ClangBuiltin<"__builtin_cheri_type_check">,
  Intrinsic<[], [llvm_cap_ty, llvm_cap_ty],
            [IntrNoMem, IntrHasSideEffects, IntrWillReturn]>;
def int_cheri_cap_subset_test:
  ClangBuiltin<"__builtin_cheri_subset_test">,
  Intrinsic<[llvm_i1_ty], [llvm_cap_ty, llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_equal_exact:
  ClangBuiltin<"__builtin_cheri_equal_exact">,
  Intrinsic<[llvm_i1_ty], [llvm_cap_ty, llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;

def int_cheri_stack_cap_get :
  ClangBuiltin<"__builtin_cheri_stack_get">,
  Intrinsic<[llvm_cap_ty], [], [IntrNoMem, IntrWillReturn]>;
def int_cheri_ddc_get :
  ClangBuiltin<"__builtin_cheri_global_data_get">,
  Intrinsic<[llvm_cap_ty], [], [IntrNoMem, IntrWillReturn]>;
def int_cheri_pcc_get :
  ClangBuiltin<"__builtin_cheri_program_counter_get">,
  Intrinsic<[llvm_cap_ty], [], [IntrNoMem, IntrWillReturn]>;

////////////////////////////////////////////////////////////////////////////////
// Offset-related intrinsics
////////////////////////////////////////////////////////////////////////////////

def int_cheri_cap_offset_set:
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_anyint_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_offset_get:
  Intrinsic<[llvm_anyint_ty], [llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_diff:
  Intrinsic<[llvm_anyint_ty], [llvm_cap_ty,llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;

////////////////////////////////////////////////////////////////////////////////
// Pointer / capability interoperation intrinsics
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_address_get:
  Intrinsic<[llvm_anyint_ty], [llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_address_set:
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_anyint_ty], [IntrNoMem, IntrWillReturn]>;


def int_cheri_cap_to_pointer:
  Intrinsic<[llvm_anyint_ty], [llvm_cap_ty, llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_from_pointer:
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_anyint_ty], [IntrNoMem, IntrWillReturn]>;

////////////////////////////////////////////////////////////////////////////////
// Building capabilities from untagged data
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_build:
  ClangBuiltin<"__builtin_cheri_cap_build">,
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_cap_type_copy:
  ClangBuiltin<"__builtin_cheri_cap_type_copy">,
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_cap_ty], [IntrNoMem, IntrWillReturn]>;

////////////////////////////////////////////////////////////////////////////////
// Tag memory accessor functions
////////////////////////////////////////////////////////////////////////////////
def int_cheri_cap_load_tags:
  Intrinsic<[llvm_anyint_ty], [llvm_cap_ty], [IntrReadMem]>;

/// Round length to compressed capability precision:
def int_cheri_round_representable_length :
   ClangBuiltin<"__builtin_cheri_round_representable_length">,
   Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrNoMem, IntrWillReturn]>;
def int_cheri_representable_alignment_mask :
   ClangBuiltin<"__builtin_cheri_representable_alignment_mask">,
   Intrinsic<[llvm_anyint_ty], [LLVMMatchType<0>], [IntrNoMem, IntrWillReturn]>;


/// This intrinsic makes it easier to avoid spilling registers to create
/// bounded capabilities for the stack (such as when passing a stack buffer to
/// another function, etc).
/// We also need both this and the dynamic variant in order to ensure we can
/// preserve lifetime markers properly in certain edge cases.
///
/// NB: They *only* has this additional meaning at the IR level; at the DAG and
///     MIR levels they cannot be assumed safe like at the IR level.
def int_cheri_bounded_stack_cap :
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_anyint_ty], [IntrNoMem, IntrWillReturn]>;
def int_cheri_bounded_stack_cap_dynamic :
  Intrinsic<[llvm_cap_ty], [llvm_cap_ty, llvm_anyint_ty], [IntrNoMem, IntrWillReturn]>;
