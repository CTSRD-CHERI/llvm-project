// REQUIRES: clang

// RUN: %clang_cheri_purecap_legacy_capreloc %s -c -o %t.o
// RUN: %clang_cheri_purecap_legacy_capreloc -DLIBBAR %s -c -o %t_bar.o
// RUN: %clang_cheri_purecap_legacy_capreloc %S/../Inputs/external_lib.c -c -o %t_external.o



// RUN: ld.lld -process-cap-relocs %t.o %t_external.o %t_bar.o -static -o %t-static.exe -verbose-cap-relocs
// RUN: llvm-objdump -t -s -h %t-static.exe | FileCheck -check-prefixes DUMP-EXE,GLOBAL_SIZES %s

// Look at shared libraries .global_sizes:
// RUN: ld.lld -process-cap-relocs %t.o %t_external.o %t_bar.o -shared -o %t.so -verbose-cap-relocs
// RUN: llvm-objdump -t -s -h %t.so | FileCheck -check-prefixes DUMP-SHLIB,GLOBAL_SIZES %s



// check external capsizefix (results in different addresses so only test the .global_sizes section contents)
// RUN: ld.lld -no-process-cap-relocs %t.o %t_external.o %t_bar.o -static -o %t-static-external-capsizefix.exe
// RUN: %capsizefix %t-static-external-capsizefix.exe
// RUN: llvm-objdump -s -t -h %t-static-external-capsizefix.exe | FileCheck -check-prefixes GLOBAL_SIZES %s


// RUN: ld.lld -no-process-cap-relocs %t.o %t_external.o %t_bar.o -shared -o %t-external-capsizefix.so
// RUN: %capsizefix %t-external-capsizefix.so
// RUN: llvm-objdump -s -t -h %t-external-capsizefix.so | FileCheck -check-prefixes GLOBAL_SIZES %s


// .size.bar is defined in both object files

#ifdef LIBBAR

char bar[0xbaa];
char other_var[0xff];

int use_bar() {
  char *x = &bar;
  return *x;
}

int use_other_var() {
  return other_var[2];
}

#else

int foo;
extern char bar[0xabc];

// defined in Inputs/external_lib.c
extern char external_buffer[];
extern void* __capability external_cap;
extern int external_func(int x);


void __start(void) {
  int* ptr1 = &foo;
  int* ptr2 = &bar;
  char* ptr3 = external_buffer;
  void** ptr4 = &external_cap;
  int (*ptr5)(int) = &external_func;
}

#endif

// DUMP-EXE-LABEL: Sections:
// DUMP-EXE: .global_sizes 00000030 0000000120000158 DATA
// DUMP-SHLIB-LABEL: Sections:
// DUMP-SHLIB: .global_sizes 00000030 0000000000000190 DATA

// GLOBAL_SIZES-LABEL: Contents of section .global_sizes:
// .size.external_cap then .size.external_buffer (0x10 or 0x20)
// GLOBAL_SIZES-NEXT: 00000000 000000{{2|1}}0 00000000 000010e1
// .size.bar (should pick up the value from bar.o) then .size.foo
// GLOBAL_SIZES-NEXT: 00000000 00000baa 00000000 00000004
// .size.bar from bar.o (not used because .size.bar points to the first one) and then .size.other_var
// GLOBAL_SIZES-NEXT: 00000000 00000000 00000000 000000ff
// GLOBAL_SIZES-NEXT: Contents of section .MIPS.abiflags:

// DUMP-EXE-LABEL: SYMBOL TABLE:
// DUMP-EXE: 0000000120000168 gw      .global_sizes           00000008 .size.bar
// DUMP-EXE: 0000000120000160 gw      .global_sizes           00000008 .size.external_buffer
// DUMP-EXE: 0000000120000158 gw      .global_sizes           00000008 .size.external_cap
// DUMP-EXE: 0000000120000170 gw      .global_sizes           00000008 .size.foo
// DUMP-EXE: 0000000120000180 gw      .global_sizes           00000008 .size.other_var

// DUMP-SHLIB-LABEL: SYMBOL TABLE:
// DUMP-SHLIB: 00000000000001a0 gw      .global_sizes           00000008 .size.bar
// DUMP-SHLIB: 0000000000000198 gw      .global_sizes           00000008 .size.external_buffer
// DUMP-SHLIB: 0000000000000190 gw      .global_sizes           00000008 .size.external_cap
// DUMP-SHLIB: 00000000000001a8 gw      .global_sizes           00000008 .size.foo
// DUMP-SHLIB: 00000000000001b8 gw      .global_sizes           00000008 .size.other_var
