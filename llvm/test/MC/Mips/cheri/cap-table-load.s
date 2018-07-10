# RUN: %cheri_llvm-mc %s -show-encoding | FileCheck %s
# RUN: not %cheri_llvm-mc %s -defsym=BAD=1 -show-encoding 2>&1 | FileCheck %s -check-prefix BAD -implicit-check-not "error:"

.ifndef BAD
	clcbi		$c1, 0($c1)
# CHECK:      clcbi	$c1, 0($c1) # encoding: [0x74,0x21,0x00,0x00]
	cscbi		$c1, 0($c1)
# CHECK:      cscbi	$c1, 0($c1) # encoding: [0x78,0x21,0x00,0x00]

# Maximum and minimum values:
	clcbi		$c1, -0x80000($c1)
# CHECK: 	clcbi	$c1, -524288($c1)       # encoding: [0x74,0x21,0x80,0x00]
	cscbi		$c1, -0x80000($c1)
# CHECK:	cscbi	$c1, -524288($c1)       # encoding: [0x78,0x21,0x80,0x00]
  clcbi		$c1, 0x7fff0($c1)
# CHECK:	clcbi	$c1, 524272($c1)        # encoding: [0x74,0x21,0x7f,0xff]
  cscbi		$c1, 0x7fff0($c1)
# CHECK: 	cscbi	$c1, 524272($c1)        # encoding: [0x78,0x21,0x7f,0xff]

# all ones:
  clcbi		$c1, -16($c1)
# CHECK:  clcbi	$c1, -16($c1)           # encoding: [0x74,0x21,0xff,0xff]
  cscbi		$c1, -16($c1)
# CHECK: cscbi	$c1, -16($c1)           # encoding: [0x78,0x21,0xff,0xff]

.else
cscbi		$c1, 1($c1)  # not multiple of 16
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
cscbi		$c1, 17($c1)  # not multiple of 16
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
cscbi		$c1, 0x80000($c1) # one out of range
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
cscbi		$c1, -0x80010($c1) # one out of range
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction

clcbi		$c1, 1($c1)  # not multiple of 16
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
clcbi		$c1, 17($c1)  # not multiple of 16
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
clcbi		$c1, 0x80000($c1) # one out of range
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction
clcbi		$c1, -0x80010($c1) # one out of range
# BAD: [[@LINE-1]]:13: error: invalid operand for instruction

# Check that jalx doesn't work
foo:
  jalx foo
# BAD: [[@LINE-1]]:3: error: instruction requires a CPU feature not currently enabled
  nop
.endif
