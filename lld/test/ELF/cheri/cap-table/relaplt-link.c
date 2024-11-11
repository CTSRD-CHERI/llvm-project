// REQUIRES: clang, x86, mips
// Fix running llvm-strip on CheriABI binaries with a .rel.plt section.
// Check that set sh_info correctly for .rel.plt since otherwise llvm-strip will
// fail. Since all .rel.plt relocations will affect the captable sh_info should
// point to .cap_table. This test also checks that the sh_info value in .rel.dyn
// is valid if if .captable is missing.

// FIXME: this test should be using assembly input and not C

// Check x86_64 since MIPS n64 doesn't seem to use .rel.plt:
// RUN: %clang_cc1 -triple=x86_64-unknown-freebsd -emit-obj -O2 %s -o %t-amd64.o
// RUN: llvm-readobj -r %t-amd64.o | FileCheck --check-prefix AMD64-OBJ %s
// AMD64-OBJ: Relocations [
// AMD64-OBJ-NEXT: Section ({{.+}}) .rela.text {
// AMD64-OBJ-NEXT:     0x3 R_X86_64_GOTPCREL fn_ptr 0xFFFFFFFFFFFFFFFC
// AMD64-OBJ-NEXT:     0x8 R_X86_64_PLT32 extern_function 0xFFFFFFFFFFFFFFFC
// AMD64-OBJ-NEXT:  }
// RUN: ld.lld -shared -o %t-amd64.so %t-amd64.o
// RUN: llvm-readobj --dynamic-table -r --sections --file-headers %t-amd64.so | FileCheck --check-prefix AMD64-SHLIB %s

// AMD64-SHLIB:      SectionHeaderCount: 16
// AMD64-SHLIB:  Section {
// AMD64-SHLIB:      Index: [[DYNSYM_INDEX:1]]
// AMD64-SHLIB-NEXT: Name: .dynsym

// AMD64-SHLIB:      Section {
// AMD64-SHLIB:         Index: 5
// AMD64-SHLIB:         Name: .rela.dyn
// AMD64-SHLIB-NEXT:    Type: SHT_RELA (0x4)
// AMD64-SHLIB-NEXT:    Flags [ (0x2)
// AMD64-SHLIB-NEXT:      SHF_ALLOC (0x2)
// AMD64-SHLIB-NEXT:    ]
// AMD64-SHLIB-NEXT:    Address: [[PLT_DYN_ADDR:.+]]
// AMD64-SHLIB-NEXT:    Offset: [[PLT_DYN_ADDR]]
// AMD64-SHLIB-NEXT:    Size: 24
// This links to .dynsym
// AMD64-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// AMD64-SHLIB-NEXT:    Info: 0
// AMD64-SHLIB-NEXT:    AddressAlignment: 8
// AMD64-SHLIB-NEXT:    EntrySize: 24

// AMD64-SHLIB:      Section {
// AMD64-SHLIB:         Index: 6
// AMD64-SHLIB:         Name: .rela.plt
// AMD64-SHLIB-NEXT:    Type: SHT_RELA (0x4)
// AMD64-SHLIB-NEXT:    Flags [ (0x42)
// AMD64-SHLIB-NEXT:      SHF_ALLOC (0x2)
// AMD64-SHLIB-NEXT:      SHF_INFO_LINK (0x40)
// AMD64-SHLIB-NEXT:    ]
// AMD64-SHLIB-NEXT:    Address: [[PLT_REL_ADDR:.+]]
// AMD64-SHLIB-NEXT:    Offset: [[PLT_REL_ADDR]]
// AMD64-SHLIB-NEXT:    Size: 24
// This also links to .dynsym
// AMD64-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// AMD64-SHLIB-NEXT:    Info: [[GOTPLT_INDEX:11]]
// AMD64-SHLIB-NEXT:    AddressAlignment: 8
// AMD64-SHLIB-NEXT:    EntrySize: 24
// AMD64-SHLIB-NEXT:  }

// AMD64-SHLIB:  Section {
// AMD64-SHLIB:      Index: [[GOTPLT_INDEX]]
// AMD64-SHLIB-NEXT: Name: .got.plt

// AMD64-SHLIB-LABEL: DynamicSection [
// AMD64-SHLIB-NEXT: Tag                Type                 Name/Value
// AMD64-SHLIB-NEXT: 0x0000000000000007 RELA                 [[PLT_DYN_ADDR]]
// AMD64-SHLIB-NEXT: 0x0000000000000008 RELASZ               24 (bytes)
// AMD64-SHLIB-NEXT: 0x0000000000000009 RELAENT              24 (bytes)
// AMD64-SHLIB-NEXT: 0x0000000000000017 JMPREL               [[PLT_REL_ADDR]]
// AMD64-SHLIB-NEXT: 0x0000000000000002 PLTRELSZ             24 (bytes)
// AMD64-SHLIB-NEXT: 0x0000000000000003 PLTGOT               0x{{.+}}
// AMD64-SHLIB-NEXT: 0x0000000000000014 PLTREL               RELA

// AMD64-SHLIB-LABEL: Relocations [
// AMD64-SHLIB-NEXT: Section ({{.+}}) .rela.dyn {
// AMD64-SHLIB-NEXT:    0x{{.+}} R_X86_64_GLOB_DAT fn_ptr 0x0
// AMD64-SHLIB-NEXT: }
// AMD64-SHLIB-NEXT: Section ({{.+}}) .rela.plt {
// AMD64-SHLIB-NEXT:    0x{{.+}} R_X86_64_JUMP_SLOT extern_function 0x0
// AMD64-SHLIB-NEXT: }

// RUN: llvm-strip -o - %t-amd64.so | llvm-readobj --file-headers - | FileCheck %s --check-prefix AMD64-STRIPPED
// AMD64-STRIPPED: SectionHeaderCount: 13


// Check that purecap also has the same link value and can be stripped

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=plt -emit-obj -O2 %s -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck --check-prefix PURECAP-OBJ %s
// PURECAP-OBJ: Relocations [
// PURECAP-OBJ-NEXT: Section ({{.+}}) .rela.text {
// PURECAP-OBJ-NEXT:    0x10 R_MIPS_CHERI_CAPTAB20/R_MIPS_NONE/R_MIPS_NONE fn_ptr 0x0
// PURECAP-OBJ-NEXT:    0x14 R_MIPS_CHERI_CAPCALL20/R_MIPS_NONE/R_MIPS_NONE extern_function 0x0
// PURECAP-OBJ-NEXT:  }
// RUN: ld.lld -shared -o %t.so %t.o
// RUN: llvm-readobj --dynamic-table --file-headers -r --sections %t.so | FileCheck --check-prefix PURECAP-SHLIB %s
// PURECAP-SHLIB:      SectionHeaderCount: 20
// PURECAP-SHLIB:  Section {
// PURECAP-SHLIB:      Index: [[DYNSYM_INDEX:3]]
// PURECAP-SHLIB-NEXT: Name: .dynsym

// PURECAP-SHLIB:      Section {
// PURECAP-SHLIB:         Index: 7
// PURECAP-SHLIB-NEXT:    Name: .rel.dyn
// PURECAP-SHLIB-NEXT:    Type: SHT_REL (0x9)
// PURECAP-SHLIB-NEXT:    Flags [ (0x2)
// PURECAP-SHLIB-NEXT:      SHF_ALLOC (0x2)
// PURECAP-SHLIB-NEXT:    ]
// PURECAP-SHLIB-NEXT:    Address: [[PLT_DYN_ADDR:.+]]
// PURECAP-SHLIB-NEXT:    Offset: [[PLT_DYN_ADDR]]
// PURECAP-SHLIB-NEXT:    Size: 16
// PURECAP-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// PURECAP-SHLIB-NEXT:    Info: 0
// PURECAP-SHLIB-NEXT:    AddressAlignment: 8
// PURECAP-SHLIB-NEXT:    EntrySize: 16
// PURECAP-SHLIB-NEXT:  }

// PURECAP-SHLIB:      Section {
// PURECAP-SHLIB:         Index: 8
// PURECAP-SHLIB-NEXT:    Name: .rel.plt
// PURECAP-SHLIB-NEXT:    Type: SHT_REL (0x9)
// PURECAP-SHLIB-NEXT:    Flags [ (0x42)
// PURECAP-SHLIB-NEXT:      SHF_ALLOC (0x2)
// PURECAP-SHLIB-NEXT:      SHF_INFO_LINK (0x40)
// PURECAP-SHLIB-NEXT:    ]
// PURECAP-SHLIB-NEXT:    Address: [[PLT_REL_ADDR:.+]]
// PURECAP-SHLIB-NEXT:    Offset: [[PLT_REL_ADDR]]
// PURECAP-SHLIB-NEXT:    Size: 16
// PURECAP-SHLIB-NEXT:    Link: [[DYNSYM_INDEX]]
// PURECAP-SHLIB-NEXT:    Info: [[CAPTABLE_INDEX:11]]
// PURECAP-SHLIB-NEXT:    AddressAlignment: 8
// PURECAP-SHLIB-NEXT:    EntrySize: 16
// PURECAP-SHLIB-NEXT:  }

// PURECAP-SHLIB:  Section {
// PURECAP-SHLIB:      Index: [[CAPTABLE_INDEX]]
// PURECAP-SHLIB-NEXT: Name: .captable

// PURECAP-SHLIB-LABEL: DynamicSection [
// PURECAP-SHLIB-NEXT: Tag                Type                 Name/Value
// PURECAP-SHLIB-NEXT: 0x0000000000000011 REL                  [[PLT_DYN_ADDR]]
// PURECAP-SHLIB-NEXT: 0x0000000000000012 RELSZ                16 (bytes)
// PURECAP-SHLIB-NEXT: 0x0000000000000013 RELENT               16 (bytes)
// PURECAP-SHLIB-NEXT: 0x0000000000000017 JMPREL               [[PLT_REL_ADDR:.+]]
// PURECAP-SHLIB-NEXT: 0x0000000000000002 PLTRELSZ             16 (bytes)
// PURECAP-SHLIB-NEXT: 0x0000000070000032 MIPS_PLTGOT          0x0
// PURECAP-SHLIB-NEXT: 0x0000000000000014 PLTREL               REL

// PURECAP-SHLIB: Relocations [
// PURECAP-SHLIB-NEXT: Section ({{.+}}) .rel.dyn {
// PURECAP-SHLIB-NEXT:    R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE fn_ptr{{$}}
// PURECAP-SHLIB-NEXT:  }
// PURECAP-SHLIB-NEXT: Section ({{.+}}) .rel.plt {
// PURECAP-SHLIB-NEXT:    R_MIPS_CHERI_CAPABILITY_CALL/R_MIPS_NONE/R_MIPS_NONE extern_function{{$}}
// PURECAP-SHLIB-NEXT:  }

// RUN: llvm-strip -o - %t.so | llvm-readobj --file-headers - | FileCheck %s --check-prefix PURECAP-STRIPPED
// PURECAP-STRIPPED: SectionHeaderCount: 15


extern void* extern_function(void *);

extern void* fn_ptr(void *);

void *call(void) {
  return extern_function(&fn_ptr);
}
