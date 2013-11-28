# RUN: llvm-mc %s -triple=cheri-unknown-freebsd -show-encoding -mcpu=cheri | FileCheck %s
#
# Check that the assembler is able to handle capability pseudo-instructions.
#

# CHECK: cmove		$c1, $c2
	cmove		$c1, $c2
