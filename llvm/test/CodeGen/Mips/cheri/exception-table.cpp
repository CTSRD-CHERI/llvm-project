// REQUIRES: clang

// RUN: %cheri_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -S -o - | FileCheck %s --check-prefixes MIPS,CHECK
// RUN: %cheri_purecap_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -S -o - | %cheri_FileCheck %s --check-prefixes PURECAP,CHECK
// RUN: %cheri_purecap_cc1 -O2 -masm-verbose -mframe-pointer=none -fcxx-exceptions -fexceptions %s -emit-obj -o - | llvm-readobj --sections --section-data -r - | FileCheck %s --check-prefix OBJ-RELOCS

// OBJ-RELOCS-LABEL:  Section {
// OBJ-RELOCS:        Index:
// OBJ-RELOCS:        Name: .gcc_except_table
// OBJ-RELOCS-NEXT:   Type: SHT_PROGBITS (0x1)
// OBJ-RELOCS-NEXT:   Flags [ (0x3)
// OBJ-RELOCS-NEXT:     SHF_ALLOC (0x2)
// OBJ-RELOCS-NEXT:     SHF_WRITE (0x1)
// OBJ-RELOCS-NEXT:   ]
// OBJ-RELOCS-NEXT:   Address: 0x0
// OBJ-RELOCS-NEXT:   Offset: 0x110
// OBJ-RELOCS-NEXT:   Size: 76
// OBJ-RELOCS-NEXT:   Link: 0
// OBJ-RELOCS-NEXT:   Info: 0
// OBJ-RELOCS-NEXT:   AddressAlignment: 16
// OBJ-RELOCS-NEXT:   EntrySize: 0
// OBJ-RELOCS-NEXT:   SectionData (
/// Ensure that the capabilities are correctly aligned and preceded by 0xc:
// OBJ-RELOCS-NEXT:     0000: FF9B4901 40240C0C 00000000 00000000  |..I.@$..........|
// OBJ-RELOCS-NEXT:     0010: CACACACA CACACACA CACACACA CACACACA  |................|
// OBJ-RELOCS-NEXT:     0020: 01380C0C 00000000 00000000 00000000  |.8..............|
// OBJ-RELOCS-NEXT:     0030: CACACACA CACACACA CACACACA CACACACA  |................|
// OBJ-RELOCS-NEXT:     0040: 01446800 00010000 00000000           |.Dh.........|
// OBJ-RELOCS-NEXT:   )

// OBJ-RELOCS-LABEL: Section ({{.+}}) .rela.gcc_except_table {
// OBJ-RELOCS-NEXT:   0x10 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll 0x8C
// OBJ-RELOCS-NEXT:   0x30 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE _Z4testll 0x6C
// OBJ-RELOCS-NEXT: }
// OBJ-RELOCS-NEXT: Section ({{.+}}) .rela.data.DW.ref.__gxx_personality_v0 {
// OBJ-RELOCS-NEXT:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __gxx_personality_v0 0x0
// OBJ-RELOCS-NEXT: }

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
// PURECAP-NEXT:     .byte 12                                    # (landing pad is a capability)
// PURECAP-NEXT:     .chericap	_Z4testll + .Ltmp2-.Lfunc_begin0 #     jumps to .Ltmp2
// CHECK-NEXT:     .byte	1                       #   On action: 1
// CHECK-NEXT:     .uleb128 .Ltmp3-.Lfunc_begin0   # >> Call Site 2 <<
// CHECK-NEXT:     .uleb128 .Ltmp4-.Ltmp3          #   Call between .Ltmp3 and .Ltmp4
// MIPS-NEXT:        .uleb128 .Ltmp5-.Lfunc_begin0               # jumps to .Ltmp5
/// Landing pads are not relative offsets but real capabilities:
// PURECAP-NEXT:     .byte 12                                    # (landing pad is a capability)
// PURECAP-NEXT:     .chericap	_Z4testll + .Ltmp5-.Lfunc_begin0 #     jumps to .Ltmp5
// CHECK-NEXT:     .byte	1                       #   On action: 1
// CHECK-NEXT:     .uleb128 .Ltmp4-.Lfunc_begin0   # >> Call Site 3 <<
// CHECK-NEXT:     .uleb128 .Lfunc_end0-.Ltmp4     #   Call between .Ltmp4 and .Lfunc_end0
/// NULL landing pads are still use a single zero byte (since we indicate real ones with the 0xc ULEB128 value)
// CHECK-NEXT:     .byte	0               #     has no landing pad
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
