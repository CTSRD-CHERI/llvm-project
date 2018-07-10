// RUN: %cheri_purecap_cc1 %s -mllvm -mxgot -mllvm -cheri-cap-table-abi=plt -x c -S -O2 -o -
// RUN: %cheri_purecap_cc1 %s -mllvm -mxgot -mllvm -cheri-cap-table-abi=plt -x c -S -O2 -o - | FileCheck %s

#include <cheri_init_globals.h>

DEFINE_CHERI_START_FUNCTION(_start)

int _start(void) {
  // crt_init_globals();

  // CHECK:      lui	$2, %highest(__cap_table_start)
  // CHECK-NEXT: lui	$1, %hi(__cap_table_start)
  // CHECK-NEXT: daddiu	$2, $2, %higher(__cap_table_start)
  // CHECK-NEXT: daddiu	$1, $1, %lo(__cap_table_start)
  // CHECK-NEXT: dsll32	$2, $2, 0
  // CHECK-NEXT: daddu	$2, $2, $1
  // CHECK-NEXT: beqz	$2, .Lskip_cgp_setup
  // CHECK-NEXT: nop
  // CHECK-NEXT: lui	$3, %highest(__cap_table_end)
  // CHECK-NEXT: lui	$1, %hi(__cap_table_end)
  // CHECK-NEXT: daddiu	$3, $3, %higher(__cap_table_end)
  // CHECK-NEXT: daddiu	$1, $1, %lo(__cap_table_end)
  // CHECK-NEXT: dsll32	$3, $3, 0
  // CHECK-NEXT: daddu	$3, $3, $1
  // CHECK-NEXT: creadhwr $c26, $chwr_ddc
  // CHECK-NEXT: csetoffset	$c26, $c26, $2
  // CHECK-NEXT: dsubu	$1, $3, $2
  // CHECK-NEXT: csetbounds	$c26, $c26, $1
  // CHECK-NEXT: .Lskip_cgp_setup:
  // CHECK-NEXT: .set    noat
  // CHECK-NEXT: .protected      _start
  // CHECK-NEXT: lui     $1, %pcrel_hi(_start-8)
  // CHECK-NEXT: daddiu  $1, $1, %pcrel_lo(_start-4)
  // CHECK-NEXT: cgetpcc $c12
  // CHECK-NEXT: cincoffset      $c12, $c12, $1
  // CHECK-NEXT: cjr     $c12
  // CHECK-NEXT: nop
  // CHECK-NEXT: .end    __start



  return 0;
}
