# RUN: llvm-mc %s -triple=riscv32 -mattr=+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-INST,CHECK-ENC %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+cap-mode -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST-ALIAS %s
#
# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+cap-mode -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-INST,CHECK-ENC %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+cap-mode < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+cap-mode -d - \
# RUN:     | FileCheck -check-prefix=CHECK-INST-ALIAS %s
#
##################################
# Debug Information Registers
##################################

# dpcc
# name
# CHECK-INST: csrrs ct1, dpcc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x10,0x7b]
# CHECK-INST-ALIAS: csrr ct1, dpcc
# uimm12
# CHECK-INST: csrrs ct2, dpcc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x10,0x7b]
# CHECK-INST-ALIAS: csrr ct2, dpcc
# name
csrrs ct1, dpcc, zero
# uimm12
csrrs ct2, 0x7b1, zero

# dscratch0c
# name
# CHECK-INST: csrrs ct1, dscratch0c, zero
# CHECK-ENC: encoding: [0x73,0x23,0x20,0x7b]
# CHECK-INST-ALIAS: csrr ct1, dscratch0c
# uimm12
# CHECK-INST: csrrs ct2, dscratch0c, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x20,0x7b]
# CHECK-INST-ALIAS: csrr ct2, dscratch0c
# name
csrrs ct1, dscratch0c, zero
# uimm12
csrrs ct2, 0x7b2, zero

# dscratch1c
# name
# CHECK-INST: csrrs ct1, dscratch1c, zero
# CHECK-ENC: encoding: [0x73,0x23,0x30,0x7b]
# CHECK-INST-ALIAS: csrr ct1, dscratch1c
# uimm12
# CHECK-INST: csrrs ct2, dscratch1c, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x30,0x7b]
# CHECK-INST-ALIAS: csrr ct2, dscratch1c
# name
csrrs ct1, dscratch1c, zero
# uimm12
csrrs ct2, 0x7b3, zero

# dinfc
# name
# CHECK-INST: csrrs ct1, dinfc, zero
# CHECK-ENC: encoding: [0x73,0x23,0xd0,0x7b]
# CHECK-INST-ALIAS: csrr ct1, dinfc
# uimm12
# CHECK-INST: csrrs ct2, dinfc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0xd0,0x7b]
# CHECK-INST-ALIAS: csrr ct2, dinfc
# name
csrrs ct1, dinfc, zero
# uimm12
csrrs ct2, 0x7bd, zero

# utidc
# name
# CHECK-INST: csrrs ct1, utidc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0xc8]
# CHECK-INST-ALIAS: csrr ct1, utidc
# uimm12
# CHECK-INST: csrrs ct2, utidc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0xc8]
# CHECK-INST-ALIAS: csrr ct2, utidc
# name
csrrs ct1, utidc, zero
# uimm12
csrrs ct2, 0xc80, zero

# stidc
# name
# CHECK-INST: csrrs ct1, stidc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0x58]
# CHECK-INST-ALIAS: csrr ct1, stidc
# uimm12
# CHECK-INST: csrrs ct2, stidc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0x58]
# CHECK-INST-ALIAS: csrr ct2, stidc
# name
csrrs ct1, stidc, zero
# uimm12
csrrs ct2, 0x580, zero


##################################
# Machine Information Registers
##################################

# mtvecc
# name
# CHECK-INST: csrrs ct1, mtvecc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x50,0x30]
# CHECK-INST-ALIAS: csrr ct1, mtvecc
# uimm12
# CHECK-INST: csrrs ct2, mtvecc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x50,0x30]
# CHECK-INST-ALIAS: csrr ct2, mtvecc
# name
csrrs ct1, mtvecc, zero
# uimm12
csrrs ct2, 0x305, zero


# mepcc
# name
# CHECK-INST: csrrs ct1, mepcc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x10,0x34]
# CHECK-INST-ALIAS: csrr ct1, mepcc
# uimm12
# CHECK-INST: csrrs ct2, mepcc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x10,0x34]
# CHECK-INST-ALIAS: csrr ct2, mepcc
# name
csrrs ct1, mepcc, zero
# uimm12
csrrs ct2, 0x341, zero

# mscratchc
# name
# CHECK-INST: csrrs ct1, mscratchc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0x34]
# CHECK-INST-ALIAS: csrr ct1, mscratchc
# uimm12
# CHECK-INST: csrrs ct2, mscratchc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0x34]
# CHECK-INST-ALIAS: csrr ct2, mscratchc
# name
csrrs ct1, mscratchc, zero
# uimm12
csrrs ct2, 0x340, zero

# mtidc
# name
# CHECK-INST: csrrs ct1, mtidc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0x78]
# CHECK-INST-ALIAS: csrr ct1, mtidc
# uimm12
# CHECK-INST: csrrs ct2, mtidc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0x78]
# CHECK-INST-ALIAS: csrr ct2, mtidc
# name
csrrs ct1, mtidc, zero
# uimm12
csrrs ct2, 0x780, zero

##################################
# Supervisor Information Registers
##################################

# stvecc
# name
# CHECK-INST: csrrs ct1, stvecc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x50,0x10]
# CHECK-INST-ALIAS: csrr ct1, stvecc
# uimm12
# CHECK-INST: csrrs ct2, stvecc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x50,0x10]
# CHECK-INST-ALIAS: csrr ct2, stvecc
# name
csrrs ct1, stvecc, zero
# uimm12
csrrs ct2, 0x105, zero

# sscratchc
# name
# CHECK-INST: csrrs ct1, sscratchc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0x14]
# CHECK-INST-ALIAS: csrr ct1, sscratchc
# uimm12
# CHECK-INST: csrrs ct2, sscratchc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0x14]
# CHECK-INST-ALIAS: csrr ct2, sscratchc
# name
csrrs ct1, sscratchc, zero
# uimm12
csrrs ct2, 0x140, zero

# sepcc
# name
# CHECK-INST: csrrs ct1, sepcc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x10,0x14]
# CHECK-INST-ALIAS: csrr ct1, sepcc
# uimm12
# CHECK-INST: csrrs ct2, sepcc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x10,0x14]
# CHECK-INST-ALIAS: csrr ct2, sepcc
# name
csrrs ct1, sepcc, zero
# uimm12
csrrs ct2, 0x141, zero

##################################
# Supervisor Information Registers
##################################

# jvtc
# name
# CHECK-INST: csrrs ct1, jvtc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x70,0x01]
# CHECK-INST-ALIAS: csrr ct1, jvtc
# uimm12
# CHECK-INST: csrrs ct2, jvtc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x70,0x01]
# CHECK-INST-ALIAS: csrr ct2, jvtc
# name
csrrs ct1, jvtc, zero
# uimm12
csrrs ct2, 0x017, zero

##################################
# Cheri Hybrid Registers
##################################

# dddc
# name
# CHECK-INST: csrrs ct1, dddc, zero
# CHECK-ENC: encoding: [0x73,0x23,0xc0,0x7b]
# CHECK-INST-ALIAS: csrr ct1, dddc
# uimm12
# CHECK-INST: csrrs ct2, dddc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0xc0,0x7b]
# CHECK-INST-ALIAS: csrr ct2, dddc
# name
csrrs ct1, dddc, zero
# uimm12
csrrs ct2, 0x7bc, zero

# mtdc
# name
# CHECK-INST: csrrs ct1, mtdc, zero
# CHECK-ENC: encoding: [0x73,0x23,0xc0,0x74]
# CHECK-INST-ALIAS: csrr ct1, mtdc
# uimm12
# CHECK-INST: csrrs ct2, mtdc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0xc0,0x74]
# CHECK-INST-ALIAS: csrr ct2, mtdc
# name
csrrs ct1, mtdc, zero
# uimm12
csrrs ct2, 0x74c, zero

# stdc
# name
# CHECK-INST: csrrs ct1, stdc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x30,0x16]
# CHECK-INST-ALIAS: csrr ct1, stdc
# uimm12
# CHECK-INST: csrrs ct2, stdc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x30,0x16]
# CHECK-INST-ALIAS: csrr ct2, stdc
# name
csrrs ct1, stdc, zero
# uimm12
csrrs ct2, 0x163, zero

# ddc
# name
# CHECK-INST: csrrs ct1, ddc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x60,0x41]
# CHECK-INST-ALIAS: csrr ct1, ddc
# uimm12
# CHECK-INST: csrrs ct2, ddc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x60,0x41]
# CHECK-INST-ALIAS: csrr ct2, ddc
# name
csrrs ct1, ddc, zero
# uimm12
csrrs ct2, 0x416, zero

##################################
# Hypervisor Registers
##################################

# htval2
# name
# CHECK-INST: csrrs t1, htval2, zero
# CHECK-ENC: encoding: [0x73,0x23,0xb0,0x64]
# CHECK-INST-ALIAS: csrr t1, htval2
# uimm12
# CHECK-INST: csrrs t2, htval2, zero
# CHECK-ENC: encoding: [0xf3,0x23,0xb0,0x64]
# CHECK-INST-ALIAS: csrr t2, htval2
# name
csrrs t1, htval2, zero
# uimm12
csrrs t2, 0x64b, zero

##################################
# Virtual Supervisor Registers
##################################

# vstvecc
# name
# CHECK-INST: csrrs ct1, vstvecc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x50,0x20]
# CHECK-INST-ALIAS: csrr ct1, vstvecc
# uimm12
# CHECK-INST: csrrs ct2, vstvecc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x50,0x20]
# CHECK-INST-ALIAS: csrr ct2, vstvecc
# name
csrrs ct1, vstvecc, zero
# uimm12
csrrs ct2, 0x205, zero

# vsscratchc
# name
# CHECK-INST: csrrs ct1, vsscratchc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0x24]
# CHECK-INST-ALIAS: csrr ct1, vsscratchc
# uimm12
# CHECK-INST: csrrs ct2, vsscratchc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0x24]
# CHECK-INST-ALIAS: csrr ct2, vsscratchc
# name
csrrs ct1, vsscratchc, zero
# uimm12
csrrs ct2, 0x240, zero

# vsepcc
# name
# CHECK-INST: csrrs ct1, vsepcc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x10,0x24]
# CHECK-INST-ALIAS: csrr ct1, vsepcc
# uimm12
# CHECK-INST: csrrs ct2, vsepcc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x10,0x24]
# CHECK-INST-ALIAS: csrr ct2, vsepcc
# name
csrrs ct1, vsepcc, zero
# uimm12
csrrs ct2, 0x241, zero

# vstdc
# name
# CHECK-INST: csrrs ct1, vstdc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x50,0x24]
# CHECK-INST-ALIAS: csrr ct1, vstdc
# uimm12
# CHECK-INST: csrrs ct2, vstdc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x50,0x24]
# CHECK-INST-ALIAS: csrr ct2, vstdc
# name
csrrs ct1, vstdc, zero
# uimm12
csrrs ct2, 0x245, zero

# vstval2
# name
# CHECK-INST: csrrs t1, vstval2, zero
# CHECK-ENC: encoding: [0x73,0x23,0xb0,0x24]
# CHECK-INST-ALIAS: csrr t1, vstval2
# uimm12
# CHECK-INST: csrrs t2, vstval2, zero
# CHECK-ENC: encoding: [0xf3,0x23,0xb0,0x24]
# CHECK-INST-ALIAS: csrr t2, vstval2
# name
csrrs t1, vstval2, zero
# uimm12
csrrs t2, 0x24b, zero

# vstidc
# name
# CHECK-INST: csrrs ct1, vstidc, zero
# CHECK-ENC: encoding: [0x73,0x23,0x00,0xa8]
# CHECK-INST-ALIAS: csrr ct1, vstidc
# uimm12
# CHECK-INST: csrrs ct2, vstidc, zero
# CHECK-ENC: encoding: [0xf3,0x23,0x00,0xa8]
# CHECK-INST-ALIAS: csrr ct2, vstidc
# name
csrrs ct1, vstidc, zero
# uimm12
csrrs ct2, 0xa80, zero
