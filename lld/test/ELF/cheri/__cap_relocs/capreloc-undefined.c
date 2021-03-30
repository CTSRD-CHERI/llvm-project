// REQUIRES: clang

// RUN: %cheri_purecap_clang %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck --check-prefix READOBJ %s
// READOBJ-LABEL:  Section (5) .rela.data {
// READOBJ-NEXT:     0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE foo 0x0
// READOBJ-NEXT:   }


// By default building a shared library should not warn about unresolved symbols (but still report them)
// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs -shared --enable-new-dtags -o %t.so --fatal-warnings %t.o | FileCheck %s -check-prefix NOTE
// NOTE: warning: cap_reloc against undefined symbol: foo
// RUN: llvm-readobj -r -t %t.so | FileCheck %s --check-prefix SHLIB-RELOCS
// First reloc the is __cap_reloc target which has a load address relocation
// Second is the target, which is foo
// SHLIB-RELOCS:      Relocations [
// SHLIB-RELOCS-NEXT:  Section (7) .rel.dyn {
// SHLIB-RELOCS-NEXT:    0x20490 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE -{{$}}
// SHLIB-RELOCS-NEXT:    0x20498 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE foo{{$}}
// SHLIB-RELOCS-NEXT:    0x204A8 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE foo{{$}}
// SHLIB-RELOCS-NEXT:  }
// SHLIB-RELOCS-NEXT:]
// RUN: llvm-objdump --cap-relocs -r -s -t -h %t.so | FileCheck %s --check-prefixes CHECK
// CHECK-LABEL: Sections:
// CHECK:     __cap_relocs  00000028 0000000000020490 DATA
// CHECK:  .data         00000010 00000000000304c0 DATA
// CHECK-LABEL: SYMBOL TABLE:
// CHECK: 0000000000000000         *UND*		 0000000000000000 foo
// CHECK: 00000000000304c0 g     O .data		 00000000000000{{1|2}}0 foo_ptr
// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// 10000 is the address of foo_ptr
// CHECK-NEXT: 0x00000000000304c0	Base: <Unnamed symbol> (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000

// But it should with --unresolved-symbols=report-all
// RUN: not ld.lld -shared --unresolved-symbols=report-all -o %t.so %t.o 2>&1 | FileCheck %s -check-prefix ERR
// And when building executables
// RUN: not ld.lld -o %t.exe %t.o 2>&1 | FileCheck %s -check-prefix ERR
// ERR: error: undefined symbol: foo

extern int foo;

int* __capability foo_ptr = &foo;
