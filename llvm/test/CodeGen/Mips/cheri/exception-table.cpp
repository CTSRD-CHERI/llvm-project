// REQUIRES: clang
// RUN: %cheri_cc1 -O2 -munwind-tables -fcxx-exceptions -fexceptions %s -emit-llvm -o -
// RUN: %cheri_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -S -o - | FileCheck %s --check-prefixes MIPS,CHECK
// RUN: %cheri_purecap_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -S -o - | %cheri_FileCheck %s --check-prefixes PURECAP,CHECK
// RUN: %cheri_purecap_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -emit-obj -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck %s --check-prefix OBJ-RELOCS
// OBJ-RELOCS: Section ({{.+}}) .rela.gcc_except_table {
// OBJ-RELOCS:   0x10 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll 0x8C
// OBJ-RELOCS:   0x30 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll 0x6C
// OBJ-RELOCS: }
// OBJ-RELOCS: Section ({{.+}}) .rela.data.DW.ref.__gxx_personality_v0 {
// OBJ-RELOCS:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __gxx_personality_v0 0x0
// OBJ-RELOCS: }

bool external_fn(long arg);

long test(long arg, long arg2) {
  long a = 0;
  long b = 0;
  try {
    a = external_fn(arg);
  } catch (...) {
    return -1;
  }
  try {
    b = external_fn(arg2);
  } catch (...) {
    return a;
  }
  return a + b;
}

// CHECK:           .end	_Z4testll
// CHECK-NEXT: .Lfunc_end0:
// CHECK-NEXT:     .size	_Z4testll, .Lfunc_end0-_Z4testll
// CHECK-NEXT:     .cfi_endproc
/// For purecap we set the writable flag on .gcc_except_table:
// MIPS-NEXT:      .section	.gcc_except_table,"a",@progbits
// PURECAP-NEXT:   .section	.gcc_except_table,"aw",@progbits
// CHECK-NEXT:     .p2align	2
// CHECK-NEXT: GCC_except_table0:
// CHECK-NEXT: .Lexception0:
// CHECK-NEXT:     .byte	255                     # @LPStart Encoding = omit
// CHECK-NEXT:     .byte	155                     # @TType Encoding = indirect pcrel sdata4
// CHECK-NEXT:     .uleb128 .Lttbase0-.Lttbaseref0
// CHECK-NEXT: .Lttbaseref0:
// CHECK-NEXT:     .byte	1                       # Call site Encoding = uleb128
// CHECK-NEXT:     .uleb128 .Lcst_end0-.Lcst_begin0
// CHECK-NEXT: .Lcst_begin0:
// CHECK-NEXT:     .uleb128 .Ltmp0-.Lfunc_begin0   # >> Call Site 1 <<
// CHECK-NEXT:     .uleb128 .Ltmp1-.Ltmp0          #   Call between .Ltmp0 and .Ltmp1
/// Landing pads are not relative offsets but real capabilities:
// MIPS-NEXT:        .uleb128 .Ltmp2-.Lfunc_begin0               # jumps to .Ltmp2
// PURECAP-NEXT:     .chericap	_Z4testll + .Ltmp2-.Lfunc_begin0 #     jumps to .Ltmp2
// CHECK-NEXT:     .byte	1                       #   On action: 1
// CHECK-NEXT:     .uleb128 .Ltmp3-.Lfunc_begin0   # >> Call Site 2 <<
// CHECK-NEXT:     .uleb128 .Ltmp4-.Ltmp3          #   Call between .Ltmp3 and .Ltmp4
// MIPS-NEXT:        .uleb128 .Ltmp5-.Lfunc_begin0               # jumps to .Ltmp5
/// Landing pads are not relative offsets but real capabilities:
// PURECAP-NEXT:     .chericap	_Z4testll + .Ltmp5-.Lfunc_begin0 #     jumps to .Ltmp5
// CHECK-NEXT:     .byte	1                       #   On action: 1
// CHECK-NEXT:     .uleb128 .Ltmp4-.Lfunc_begin0   # >> Call Site 3 <<
// CHECK-NEXT:     .uleb128 .Lfunc_end0-.Ltmp4     #   Call between .Ltmp4 and .Lfunc_end0
/// For alignment/libc++abi implementation reasons, NULL landing pads can't use a single
/// zero byte but must use a full NULL CHERI capability
// MIPS-NEXT:      .byte	0               #     has no landing pad
// PURECAP-NEXT:   .chericap	0               #     has no landing pad
// CHECK-NEXT:     .byte	0                       #   On action: cleanup
// CHECK-NEXT: .Lcst_end0:
// CHECK-NEXT:     .byte	1                       # >> Action Record 1 <<
// CHECK-NEXT:                                     #   Catch TypeInfo 1
// CHECK-NEXT:     .byte	0                       #   No further actions
// CHECK-NEXT:     .p2align	2
// CHECK-NEXT:                             # >> Catch TypeInfos <<
// CHECK-NEXT:     .4byte	0                       # TypeInfo 1
// CHECK-NEXT: .Lttbase0:
// CHECK-NEXT:     .p2align	2
// CHECK-NEXT: # -- End function
// CHECK-NEXT:     .hidden	DW.ref.__gxx_personality_v0
// CHECK-NEXT:     .weak	DW.ref.__gxx_personality_v0
// CHECK-NEXT:     .section	.data.DW.ref.__gxx_personality_v0,"aGw",@progbits,DW.ref.__gxx_personality_v0,comdat
// MIPS-NEXT:      .p2align	3
// PURECAP-NEXT:   .p2align	{{4|5}}
// CHECK-NEXT:     .type	DW.ref.__gxx_personality_v0,@object
// MIPS-NEXT:      .size	DW.ref.__gxx_personality_v0, 8
// PURECAP-NEXT:   .size	DW.ref.__gxx_personality_v0, [[#CAP_SIZE]]
// CHECK-NEXT: DW.ref.__gxx_personality_v0:
// MIPS-NEXT:      .8byte	__gxx_personality_v0
// PURECAP-NEXT:   .chericap	__gxx_personality_v0
