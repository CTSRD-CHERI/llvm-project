# RUN: llvm-mc %s -triple=mipsel-unknown-linux -show-encoding -mcpu=mips32r2 | \
# RUN: FileCheck -check-prefix=CHECK32  %s
# RUN: llvm-mc %s -triple=mips64el-unknown-linux -show-encoding -mcpu=mips64r2 | \
# RUN: FileCheck -check-prefix=CHECK64  %s
# TODO: move back into MC/Mips/mips-jump-instructions.s

#------------------------------------------------------------------------------
# Branch likely instructions
#------------------------------------------------------------------------------

# CHECK32:   bc1fl 1332             # encoding: [0x4d,0x01,0x02,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bc1tl 1332             # encoding: [0x4d,0x01,0x03,0x45]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   beql $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x51]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgezl $6, 1332         # encoding: [0x4d,0x01,0xc3,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgezall $6, 1332       # encoding: [0x4d,0x01,0xd3,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bltzall $6, 1332       # encoding: [0x4d,0x01,0xd2,0x04]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bgtzl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x5c]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   blezl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x58]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK32:   bnel $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x55]
# CHECK32:   nop                    # encoding: [0x00,0x00,0x00,0x00]

# CHECK64:   bc1fl 1332             # encoding: [0x4d,0x01,0x02,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bc1tl 1332             # encoding: [0x4d,0x01,0x03,0x45]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   beql $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x51]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgezl $6, 1332         # encoding: [0x4d,0x01,0xc3,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgezall $6, 1332       # encoding: [0x4d,0x01,0xd3,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bltzall $6, 1332       # encoding: [0x4d,0x01,0xd2,0x04]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bgtzl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x5c]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   blezl $6, 1332         # encoding: [0x4d,0x01,0xc0,0x58]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]
# CHECK64:   bnel $9, $6, 1332      # encoding: [0x4d,0x01,0x26,0x55]
# CHECK64:   nop                    # encoding: [0x00,0x00,0x00,0x00]

        bc1fl 1332
        nop
	bc1tl 1332
        nop
        beql $9, $6, 1332
        nop
        bgezl $6, 1332
        nop
        bgezall $6, 1332
        nop
	bltzall $6, 1332
        nop
        bgtzl $6, 1332
        nop
        blezl $6, 1332
        nop
        bnel $9, $6, 1332
        nop

