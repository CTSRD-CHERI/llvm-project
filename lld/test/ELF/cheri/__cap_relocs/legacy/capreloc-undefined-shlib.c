// REQUIRES: clang

// RUN: %cheri_purecap_clang %legacy_caprelocs_flag %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix READOBJ %s

// By default building a shared library should not warn about unresolved symbols (but still report them)
// RUN: ld.lld --no-relative-cap-relocs -shared --enable-new-dtags -o %t.so --fatal-warnings %t.o | FileCheck %s -check-prefix NOTE
// NOTE: warning: cap_reloc against undefined symbol: foo
// RUN: llvm-readobj -r %t.so | FileCheck %s -check-prefix SHLIB-RELOCS
// First reloc the is __cap_reloc target which has a load address relocation
// Second is the target, which is foo
// SHLIB-RELOCS:      Relocations [
// SHLIB-RELOCS-NEXT:  Section (7) .rel.dyn {
// SHLIB-RELOCS-NEXT:     0x10000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
//                           ^--- Location for foo_ptr
// SHLIB-RELOCS-NEXT:     0x10028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
//                           ^--- Location for bar_ptr
// SHLIB-RELOCS-NEXT:     0x10030 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
//                           ^--- Target for bar_ptr (since it is not preemptible this is resolved against the load address)
// SHLIB-RELOCS-NEXT:     0x10008 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
//                           ^--- Target for foo_ptr
// SHLIB-RELOCS-NEXT:     0x10018 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE foo 0x0 (real addend unknown)
//                           ^--- Size for foo_ptr
// SHLIB-RELOCS-NEXT:  }
// SHLIB-RELOCS-NEXT:]
// RUN: llvm-objdump --cap-relocs -r -s -t -h %t.so | FileCheck %s -check-prefixes CHECK,%cheri_type
// CHECK-LABEL: CAPABILITY RELOCATION RECORDS:
// 10000 is the address of foo_ptr
// CHECK-NEXT: 0x0000000000020000 Base: <Unnamed symbol> (0x0000000000000000)     Offset: 0x0000000000000000      Length: 0x0000000000000000      Permissions: 0x00000000
// CHECK-NEXT: 0x00000000000200{{2|4}}0 Base: bar (0x00000000000200{{1|2}}0)  Offset: 0x0000000000000000      Length: 0x0000000000000004      Permissions: 0x00000000
// CHECK-LABEL: Sections:
// CHECK:  __cap_relocs  00000050 0000000000010000 DATA
// CHERI128:  .data         00000030 0000000000020000 DATA
// CHERI256:  .data         00000060 0000000000020000 DATA
// 20000 is the address in rel.dyn -> correct
// CHECK-LABEL: SYMBOL TABLE:
// CHECK: 00000000000200{{1|2}}0 l     O .data  00000004 bar
// CHECK: 00000000000200{{2|4}}0 g     O .data  000000{{1|2}}0 bar_ptr
// CHECK: 0000000000000000               *UND*  00000000 foo
// CHECK: 0000000000020000       g     O .data  000000{{1|2}}0 foo_ptr

// But it should with --unresolved-symbols=report-all
// RUN: not ld.lld --no-relative-cap-relocs -shared --unresolved-symbols=report-all -o %t.so %t.o 2>&1 | FileCheck %s -check-prefix ERR
// And when building executables
// RUN: not ld.lld --no-relative-cap-relocs -o %t.exe %t.o 2>&1 | FileCheck %s -check-prefix ERR
// ERR: error: cap_reloc against undefined symbol: foo

// READOBJ-LABEL:Section ({{.+}}) .rela__cap_relocs {
// READOBJ-NEXT:   0x0 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x0
// READOBJ-NEXT:   0x8 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE foo 0x0
// READOBJ-NEXT:   0x28 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x{{2|4}}0
// READOBJ-NEXT:   0x30 R_MIPS_64/R_MIPS_NONE/R_MIPS_NONE .data 0x{{1|2}}0
// READOBJ-NEXT: }

extern int foo;

static int bar = 1;

int* __capability foo_ptr = &foo;
int* __capability bar_ptr = &bar;
