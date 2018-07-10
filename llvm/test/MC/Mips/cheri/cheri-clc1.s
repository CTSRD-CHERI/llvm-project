# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
  cldc1	$f7, $zero, 0($c2)
# CHECK: cld	$1, $zero, 0($c2)
# CHECK: dmtc1	$1, $f7
  clwc1	$f8, $zero, 8($c2)
# CHECK: clw	$1, $zero, 8($c2)
# CHECK: mtc1	$1, $f8
  csdc1	$f7, $zero, 0($c2)
# CHECK: dmfc1	$1, $f7
# CHECK: csd	$1, $zero, 0($c2)
  cswc1	$f8, $zero, 8($c2)
# CHECK: mfc1	$1, $f8
# CHECK: csw	$1, $zero, 8($c2)
