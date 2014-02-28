# RUN: llvm-mc %s -triple=mipsel-unknown-linux -show-encoding -mcpu=mips32r2 | \
# RUN: FileCheck -check-prefix=CHECK32  %s
# RUN: llvm-mc %s -triple=mips64el-unknown-linux -show-encoding -mcpu=mips64r2 | \
# RUN: FileCheck -check-prefix=CHECK64  %s
# XFAIL: *

# Check that the assembler can handle the documented syntax
# for floating point reciprocal and reciprocal square root instructions.
#------------------------------------------------------------------------------
# Reciprocal instructions
#------------------------------------------------------------------------------
# CHECK32:   recip.s                # encoding: [0x55,0x10,0x00,0x46]
# CHECK32:   recip.d                # encoding: [0x55,0x10,0x20,0x46]
# CHECK32:   rsqrt.s                # encoding: [0x56,0x10,0x00,0x46]
# CHECK32:   rsqrt.d                # encoding: [0x56,0x10,0x20,0x46]

# CHECK64:   recip.s                # encoding: [0x55,0x10,0x00,0x46]
# CHECK64:   recip.d                # encoding: [0x55,0x10,0x20,0x46]
# CHECK64:   rsqrt.s                # encoding: [0x56,0x10,0x00,0x46]
# CHECK64:   rsqrt.d                # encoding: [0x56,0x10,0x20,0x46]


	recip.s $f1, $f2
	recip.d $f1, $f2
	rsqrt.s $f1, $f2
	rsqrt.d $f1, $f2
	
