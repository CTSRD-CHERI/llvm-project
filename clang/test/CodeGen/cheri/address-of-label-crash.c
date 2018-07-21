// Check that we don't crash when using computed goto:
// https://github.com/CTSRD-CHERI/clang/issues/163

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o - -O0 -emit-llvm %s | FileCheck --check-prefixes=COMMON,IR %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -o - -O0 -emit-llvm %s | FileCheck --check-prefixes=COMMON,LEGACY-IR %s

int addrof_label_in_static(void) {
  static void *b = &&label1;
  // COMMON: @addrof_label_in_static.b = internal addrspace(200) global i8 addrspace(200)* blockaddress(@addrof_label_in_static, %label1)
  // COMMON-LABEL: define i32 @addrof_label_in_static()
  // COMMON: load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* @addrof_label_in_static.b
  goto *b;
label1:
  return 2;
}

int addrof_label_in_local(void) {
  void *d = &&label2;
  // COMMON-LABEL: define i32 @addrof_label_in_local()
  // TODO we should derive this from $c12 instead of using a global
  // COMMON: store i8 addrspace(200)* blockaddress(@addrof_label_in_local, %label2), i8 addrspace(200)* addrspace(200)* [[D_ADDR:%.+]], align
  goto *d;
label2:
  return 2;
}
