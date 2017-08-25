# RUN: llvm-mc -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri %s -target-abi purecap | FileCheck %s
# CHECK: cld	$1, $zero, 0($c0)
cld $1, $zero, 0($ddc)
# CHECK: cld	$1, $zero, 0($c11)
cld $1, $zero, 0($csp)
# CHECK: cld	$1, $zero, 0($c25)
cld $1, $zero, 0($cbp)
# CHECK: cld	$1, $zero, 0($c24)
cld $1, $zero, 0($cfp)
