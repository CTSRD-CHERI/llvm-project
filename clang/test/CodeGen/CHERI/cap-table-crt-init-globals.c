// RUN: %cheri_purecap_cc1 %s -mllvm -mxgot -mllvm -cheri-cap-table -x c -S -O2 -o - | FileCheck %s
// RUN: false

#include <cheri_init_globals.h>

void __start(void) {
  SETUP_GLOBAL_CAP_REGISTER();
  // crt_init_globals();

  // CHECK:      lui	$2, %highest(__cap_table_start)
  // CHECK-NEXT: lui	$1, %hi(__cap_table_start)
  // CHECK-NEXT: daddiu	$2, $2, %higher(__cap_table_start)
  // CHECK-NEXT: daddiu	$1, $1, %lo(__cap_table_start)
  // CHECK-NEXT: dsll32	$2, $2, 0
  // CHECK-NEXT: daddu	$2, $2, $1
  // CHECK-NEXT: lui	$3, %highest(__cap_table_end)
  // CHECK-NEXT: lui	$1, %hi(__cap_table_end)
  // CHECK-NEXT: daddiu	$3, $3, %higher(__cap_table_end)
  // CHECK-NEXT: daddiu	$1, $1, %lo(__cap_table_end)
  // CHECK-NEXT: dsll32	$3, $3, 0
  // CHECK-NEXT: daddu	$3, $3, $1

  // CHECK:      cgetdefault	 $c1
  // CHECK-NEXT: csetoffset	$c1, $c1, $2
  // CHECK-NEXT: dsubu	$1, $3, $2
  // CHECK-NEXT: csetbounds	$c1, $c1, $1
  // CHECK:      cmove	$c26,  $c1
}
