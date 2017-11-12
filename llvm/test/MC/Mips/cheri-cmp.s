# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
#
# Check that the assembler is able to handle capability get instructions.
#

ceq $4, $c5, $c6
# CHECK: ceq	$4, $c5, $c6            # encoding: [0x48,0x04,0x29,0x94]
cne $4, $c5, $c6
# CHECK: cne	$4, $c5, $c6            # encoding: [0x48,0x04,0x29,0x95]
clt $4, $c5, $c6
# CHECK: cge	 $4, $c6, $c5           # encoding: [0x48,0x04,0x29,0x96]
cge $4, $c6, $c5
# CHECK: cge	 $4, $c6, $c5           # encoding: [0x48,0x04,0x29,0x96]
cltu $4, $c5, $c6
# CHECK: cgeu	 $4, $c6, $c5           # encoding: [0x48,0x04,0x29,0x98]
cgeu $4, $c6, $c5
# CHECK: cgeu	 $4, $c6, $c5           # encoding: [0x48,0x04,0x29,0x98]
cexeq $4, $c5, $c6
# CHECK: cexeq	$4, $c5, $c6            # encoding: [0x48,0x04,0x29,0x9a]
cnexeq $4, $c5, $c6
# CHECK: cnexeq	$4, $c5, $c6            # encoding: [0x48,0x04,0x29,0xa1]
