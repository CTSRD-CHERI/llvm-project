# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability pseudo-instructions.
#

# CHECK: cmove		$c1, $c2
# CHECK: encoding: [0x49,0xa1,0x10,0x00]
	cmove		$c1, $c2
