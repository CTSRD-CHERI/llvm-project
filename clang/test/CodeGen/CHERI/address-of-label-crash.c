// Check that we don't crash when using computed goto:
// XFAIL: *
// https://github.com/CTSRD-CHERI/clang/issues/163

// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -o - -O0 -S %s
extern int x;

int a() {
  static void *b = &&label;
  // CHECK: @a.b = internal addrspace(200) global i8 addrspace(200)* addrspacecast (i8* blockaddress(@a, %label) to i8 addrspace(200)*)
  if (x)
    goto *b;
  else
    return 1;
label:
  return 2;
}
