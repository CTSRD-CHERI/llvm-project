# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability pseudo-instructions.
#

# CHECK: cmove		$c1, $c2
# CHECK: encoding: [0x48,0x81,0x10,0x02]
	cmove		$c1, $c2
