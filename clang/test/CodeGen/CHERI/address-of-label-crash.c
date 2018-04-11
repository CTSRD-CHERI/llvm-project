// Check that we don't crash when using computed goto:
// https://github.com/CTSRD-CHERI/clang/issues/163

// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm %s | FileCheck --check-prefix=IR %s
// RUN: %cheri_purecap_cc1 -o - -O0 -S %s | FileCheck --check-prefix=ASM %s

int a() {
  static void *b = &&label1;
  // IR: @a.b = internal addrspace(200) global i8 addrspace(200)* addrspacecast (i8* blockaddress(@a, %label1) to i8 addrspace(200)*)
  // ASM: a.b:
  // ASM-NEXT: .chericap	.Ltmp0
  goto *b;
label1:
  return 2;
}

int c() {
  void *d = &&label2;
  // IR: [[TMP:%[0-9]+]] = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  // IR-NEXT: call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* [[TMP]], i64 ptrtoint (i8* blockaddress(@c, %label2) to i64))
  goto *d;
label2:
  return 2;
}
