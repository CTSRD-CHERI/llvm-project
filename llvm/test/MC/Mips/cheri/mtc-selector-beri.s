# Since https://reviews.llvm.org/D41662 non-zero mtc0 selectors are rejected for pre-MIPS32 ISAs
# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=cheri128 | FileCheck %s
# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=mips4 -mattr=+cheri128 | FileCheck %s
# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=beri | FileCheck %s

mtc0      $4, $5, 1
mfc0      $4, $5, 1
mtc2      $4, $5, 1
mfc2      $4, $5, 1

# CHECK: mtc0	$4, $5, 1               # encoding: [0x40,0x84,0x28,0x01]
# CHECK: mfc0	$4, $5, 1               # encoding: [0x40,0x04,0x28,0x01]
# CHECK: mtc2	$4, $5, 1               # encoding: [0x48,0x84,0x28,0x01]
# CHECK: mfc2	$4, $5, 1               # encoding: [0x48,0x04,0x28,0x01]
