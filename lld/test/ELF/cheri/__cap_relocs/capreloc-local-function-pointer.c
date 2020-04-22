// REQUIRES: clang, cheri_is_128

// RUN: %cheri_purecap_cc1 -emit-obj -mllvm -cheri-cap-table-abi=pcrel %s -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck --check-prefix OBJ-CAPRELOCS %s

// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o -static -o %t-static.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-EXE %s
// RUN: llvm-readobj --cap-relocs -t -s --sd %t-static.exe | FileCheck --check-prefixes DUMP-CAPRELOCS,STATIC %s

// same again for statically dynamically linked exe:
// RUN: %cheri_purecap_clang %S/Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld -preemptible-caprelocs=legacy -pie -Bdynamic %t.o -o %t-dynamic.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-readobj --cap-relocs -t -s --sd -r %t-dynamic.exe | FileCheck --check-prefixes DUMP-CAPRELOCS,DYNAMIC,DYNAMIC-RELOCS %s

// Look at shared libraries:
// RUN: ld.lld -preemptible-caprelocs=legacy --no-relative-cap-relocs %t.o -shared -o %t.so -verbose 2>&1 | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-readobj --cap-relocs -t -s --sd -r %t.so | FileCheck --check-prefixes DUMP-CAPRELOCS,SHLIB,SHLIB-RELOCS %s

// FIXME: it would be good if we could set bounds here instead of having it as -1

int errno;

static int *
__error_unthreaded(void)
{

  return (&errno);
}

static int *(*__error_selector)(void) = __error_unthreaded;

void
__set_error_selector(int *(*arg)(void))
{

  __error_selector = arg;
}

__attribute__((visibility("protected"))) int *
__error(void)
{

  return (__error_selector());
}

void __start(void) {}

// OBJ-CAPRELOCS-NOT: .eh_frame
// OBJ-CAPRELOCS:       Section (8) .rela.data {
// OBJ-CAPRELOCS-NEXT:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __error_unthreaded 0x0
// OBJ-CAPRELOCS-NEXT: }
// OBJ-CAPRELOCS-NOT: .eh_frame


// LINKING-EXE-NOT: warning
// LINKING-DYNAMIC-NOT: warning

// DUMP-CAPRELOCS: Section {
// DUMP-CAPRELOCS:      Name: .data
// DUMP-CAPRELOCS:      SectionData (
// STATIC-NEXT:  0000: CACACACA CACACACA CACACACA CACACACA
// DYNAMIC-NEXT:  0000: CACACACA CACACACA CACACACA CACACACA
// SHLIB-NEXT:  0000: CACACACA CACACACA CACACACA CACACACA
// DUMP-CAPRELOCS-NEXT: )


// DUMP-CAPRELOCS-LABEL: Symbols [
// DUMP-CAPRELOCS:  Symbol {
// DUMP-CAPRELOCS:         Name: errno (
// DUMP-CAPRELOCS-NEXT:    Value:
// DUMP-CAPRELOCS-NEXT:    Size: 4
// DUMP-CAPRELOCS-NEXT:    Binding: Global (0x1)
// DUMP-CAPRELOCS-NEXT:    Type: Object (0x1)
// DUMP-CAPRELOCS-NEXT:    Other: 0
// DUMP-CAPRELOCS-NEXT:    Section: .bss
// DUMP-CAPRELOCS-NEXT:  }



// DUMP-CAPRELOCS-LABEL: CHERI __cap_relocs [
// STATIC:                     0x1200{{.+}} (__error_selector) Base: 0x120010{{.+}} (__error_unthreaded+0) Length: 24 Perms: Function
// PIE exe and shlib should have dynamic relocations and only the offset values
// DYNAMIC:                    0x030{{.+}} (__error_selector) Base: 0x10{{.+}} (__error_unthreaded+0) Length: 24 Perms: Function
// SHLIB:                      0x030{{.+}} (__error_selector) Base: 0x10{{.+}} (__error_unthreaded+0) Length: 24 Perms: Function
// DUMP-CAPRELOCS: ]

