// Check that we don't crash when using computed goto:
// https://github.com/CTSRD-CHERI/clang/issues/163

// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o - -O0 -emit-llvm %s | FileCheck --check-prefixes=COMMON,IR %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -o - -O0 -emit-llvm %s | FileCheck --check-prefixes=COMMON,LEGACY-IR %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -o - -O0 -S %s | FileCheck --check-prefixes=ASM,ASM-NEW %s
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -o - -O0 -S %s | FileCheck --check-prefixes=ASM,ASM-LEGACY %s

int a() {
  static void *b = &&label1;
  // COMMON: @a.b = internal addrspace(200) global i8 addrspace(200)* blockaddress(@a, %label1)
  // COMMON-LABEL: define i32 @a()
  // COMMON: load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* @a.b
  // Create a global containing the address of the label:
  // ASM-LABEL: .ent a
  // ASM-NEW:      clcbi	$c12, %captab20(a.b)($c12)
  // ASM-LEGACY: ld	$1, %got_page(a.b)($1)
  // ASM-LEGACY-NEXT: daddiu	$1, $1, %got_ofst(a.b)
  // ASM-LEGACY-NEXT: cfromddc	$c12, $1
  // ASM-LEGACY-NEXT: csetbounds	$c12, $c12, 16
  // ASM-LEGACY-NEXT: clc	$c12, $zero, 0($c12)
  // ASM:     .Ltmp0:
  // ASM-NEXT: .LBB0_1:
  // ASM: .end	a
  goto *b;
label1:
  return 2;
}

int c() {
  void *d = &&label2;
  // COMMON-LABEL: define i32 @c()
  // TODO we should derive this from $c12 instead of using a global
  // COMMON: store i8 addrspace(200)* blockaddress(@c, %label2), i8 addrspace(200)* addrspace(200)* [[D_ADDR:%.+]], align
  // ASM-LABEL: .ent c
  // ASM-NEW:      cmove	[[CAPTABLE:\$c[0-9]+]],  $c26
  // ASM-NEW:      clcbi	$c12, %capcall20(.Ltmp1)([[CAPTABLE]])
  // ASM-LEGACY: ld	$1, %got_page(.Ltmp1)($1)
  // ASM-LEGACY: daddiu	$1, $1, %got_ofst(.Ltmp1)
  // ASM-LEGACY: cgetpccsetoffset $c1, $1
  // ASM:      .Ltmp1:
  // ASM-NEXT: .LBB1_1:
  // ASM: .end	c
  goto *d;
label2:
  return 2;
}

// ASM-LABEL: a.b:
// ASM-NEXT: .chericap	.Ltmp0
