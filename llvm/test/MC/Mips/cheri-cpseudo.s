# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck -check-prefix=CHECK256 %s
# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri128 | FileCheck -check-prefix=CHECK128 %s
#
# Check that the assembler is able to handle capability pseudo-instructions.
#

# CHECK256: cincbase		$c1, $c2, $zero
# CHECK256: encoding: [0x48,0x81,0x10,0x02]
# CHECK128: cincoffset		$c1, $c2, $zero
# CHECK128: encoding: [0x49,0xa1,0x10,0x00]
	cmove		$c1, $c2
