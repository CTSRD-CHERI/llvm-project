# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
cldc1	$f7, $zero, 0($c2)
# CHECK: cld	$fp, $zero, 0($c2)
# CHECK: dmtc1	$fp, $f7
	clwc1	$f8, $zero, 8($c2)
# CHECK: clw	$fp, $zero, 8($c2)
# CHECK: mtc1	$fp, $f8
	csdc1	$f7, $zero, 0($c2)
# CHECK: dmfc1	$fp, $f7
# CHECK: csd	$fp, $zero, 0($c2)
	cswc1	$f8, $zero, 8($c2)
# CHECK: mfc1	$fp, $f8
# CHECK: csw	$fp, $zero, 8($c2)
