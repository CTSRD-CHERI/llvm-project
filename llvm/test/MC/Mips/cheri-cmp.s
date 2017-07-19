# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

ceq $4, $c5, $c6
# CHECK: ceq	$4, $c5, $c6            # encoding: [0x49,0xc4,0x29,0x80]
cne $4, $c5, $c6
# CHECK: cne	$4, $c5, $c6            # encoding: [0x49,0xc4,0x29,0x81]
clt $4, $c5, $c6
# CHECK: cgt	 $4, $c6, $c5           # encoding: [0x49,0xc4,0x29,0x82]
cge $4, $c5, $c6
# CHECK: cge	 $4, $c5, $c6           # encoding: [0x49,0xc4,0x31,0x43]
cltu $4, $c5, $c6
# CHECK: cgtu	 $4, $c6, $c5           # encoding: [0x49,0xc4,0x29,0x84]
cgeu $4, $c5, $c6
# CHECK: cgeu	 $4, $c5, $c6           # encoding: [0x49,0xc4,0x31,0x45]
cexeq $4, $c5, $c6
# CHECK: cexeq	$4, $c5, $c6            # encoding: [0x49,0xc4,0x29,0x86]
cnexeq $4, $c5, $c6
# CHECK: cnexeq	$4, $c5, $c6            # encoding: [0x49,0xc4,0x29,0x87]
