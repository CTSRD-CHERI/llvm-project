// REQUIRES: clang, cheri_is_128

// RUN: %cheri_purecap_clang %s -c -o %t.o
// RUN: llvm-readobj -r %t.o | FileCheck -check-prefix OBJ-CAPRELOCS %s

// RUN: ld.lld -preemptible-caprelocs=legacy %t.o -static -o %t-static.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-EXE %s
// RUN: llvm-readobj -C -t -s -sd %t-static.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC %s

// same again for statically dynamically linked exe:
// RUN: %cheri_purecap_clang %S/Inputs/dummy_shlib.c -c -o %T/integrated_dummy_shlib.o
// RUN: ld.lld -preemptible-caprelocs=legacy -pie -Bdynamic %t.o -o %t-dynamic.exe -verbose 2>&1 | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-readobj -C -t -s -sd -r %t-dynamic.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC,DYNAMIC-RELOCS %s

// Look at shared libraries:
// RUN: ld.lld -preemptible-caprelocs=legacy %t.o -shared -o %t.so -verbose 2>&1 | FileCheck -check-prefixes LINKING-DYNAMIC %s
// RUN: llvm-readobj -C -t -s -sd -r %t.so | FileCheck -check-prefixes DUMP-CAPRELOCS,DYNAMIC,DYNAMIC-RELOCS %s

// RUN: %cheri_purecap_clang %legacy_caprelocs_flag %s -c -o %t-legacy.o
// RUN: ld.lld -preemptible-caprelocs=legacy %t-legacy.o -no-process-cap-relocs -static -o %t-static-external-capsizefix.exe
// RUN: %capsizefix %t-static-external-capsizefix.exe
// RUN: llvm-readobj -C -s -t -sd %t-static-external-capsizefix.exe | FileCheck -check-prefixes DUMP-CAPRELOCS,STATIC-EXTERNAL-CAPSIZEFIX %s


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

// OBJ-CAPRELOCS:       Section (8) .rela.data {
// OBJ-CAPRELOCS-NEXT:   0x0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE __error_unthreaded 0x0
// OBJ-CAPRELOCS-NEXT: }


// LINKING-EXE-NOT: warning
// LINKING-DYNAMIC-NOT: warning

// DUMP-CAPRELOCS-LABEL: Sections [
// DUMP-CAPRELOCS: Section {
// DUMP-CAPRELOCS:      Name: .global_sizes
// DUMP-CAPRELOCS:      Size: 8
// DUMP-CAPRELOCS:      SectionData (
// DUMP-CAPRELOCS-NEXT:  0000: 00000000 00000004
// DUMP-CAPRELOCS-NEXT: )

// DUMP-CAPRELOCS: Section {
// DUMP-CAPRELOCS:      Name: .data
// DUMP-CAPRELOCS:      SectionData (
// STATIC-NEXT:  0000: CACACACA CACACACA CACACACA CACACACA
// DYNAMIC-NEXT:  0000: CACACACA CACACACA CACACACA CACACACA
// external capsizefix doesn't initialize capabilities to 0xcacacacaca
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT:  0000: 00000000 00000000 00000000 00000000
// DUMP-CAPRELOCS-NEXT: )


// DYNAMIC-RELOCS-LABEL: Relocations [
// DYNAMIC-RELOCS-NEXT:   Section (8) .rel.dyn {
// DYNAMIC-RELOCS-NEXT:     0x30000 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:     0x30008 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
// DYNAMIC-RELOCS-NEXT:   }
// DYNAMIC-RELOCS-NEXT: ]


// DUMP-CAPRELOCS-LABEL: Symbols [
// DUMP-CAPRELOCS:       Symbol {
// DUMP-CAPRELOCS: Name: .size.errno (
// DUMP-CAPRELOCS-NEXT:    Value:
// DUMP-CAPRELOCS-NEXT:    Size: 8
// DUMP-CAPRELOCS-NEXT:    Binding: Weak (0x2)
// DUMP-CAPRELOCS-NEXT:    Type: Object (0x1)
// DUMP-CAPRELOCS-NEXT:    Other: 0
// DUMP-CAPRELOCS-NEXT:    Section: .global_sizes
// DUMP-CAPRELOCS-NEXT:  }
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
// STATIC-NEXT:                     0x120020000 (__error_selector) Base: 0x1200100e0 (__error_unthreaded+0) Length: 68 Perms: Function
// PIE exe and shlib should have dynamic relocations and only the offset values
// DYNAMIC-NEXT:                    0x020000 (__error_selector) Base: 0x100e0 (__error_unthreaded+0) Length: 68 Perms: Function
// The external capsizefix does okay static:
// STATIC-EXTERNAL-CAPSIZEFIX-NEXT: 0x120020000 (__error_selector) Base: 0x1200100e0 (__error_unthreaded+0) Length: 68 Perms: Function
// DUMP-CAPRELOCS-NEXT: ]

