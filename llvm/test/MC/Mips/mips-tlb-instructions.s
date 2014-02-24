# RUN: llvm-mc %s -triple=mipsel-unknown-linux -show-encoding -mcpu=mips32r2 | \
# RUN: FileCheck -check-prefix=CHECK32  %s
# RUN: llvm-mc %s -triple=mips64el-unknown-linux -show-encoding -mcpu=mips64r2 | \
# RUN: FileCheck -check-prefix=CHECK64  %s

# Check that the assembler can handle the documented syntax
# for memory-management instructions.
#------------------------------------------------------------------------------
# TLB instructions
#------------------------------------------------------------------------------
# CHECK32:   tlbwi                  # encoding: [0x02,0x00,0x00,0x42]
# CHECK32:   tlbwr                  # encoding: [0x06,0x00,0x00,0x42]
# CHECK32:   tlbp                   # encoding: [0x08,0x00,0x00,0x42]
# CHECK32:   cache 0x01, 0($t0)     # encoding: [0x00,0x00,0x01,0xbd]

# CHECK64:   tlbwi                  # encoding: [0x02,0x00,0x00,0x42]
# CHECK64:   tlbwr                  # encoding: [0x06,0x00,0x00,0x42]
# CHECK64:   tlbp                   # encoding: [0x08,0x00,0x00,0x42]
# CHECK64:   cache 0x01, 0($t0)     # encoding: [0x00,0x00,0x01,0xbd]

.set noreorder

        tlbwi
	tlbwr
	tlbp

        cache 0x01, 0($t0)

end_of_code:
