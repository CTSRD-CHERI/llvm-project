# RUN: llvm-mc -triple riscv32 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding < %s \
# RUN:   | FileCheck -check-prefixes=CHECK,CHECK32,CHECK-ALIAS,CHECK-ALIASASM %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding \
# RUN:   -riscv-no-aliases < %s | FileCheck -check-prefixes=CHECK,CHECK32,CHECK-INST,CHECK-INSTASM %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+zcheripurecap,+cap-mode -d - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-BYTES32,CHECK-ALIAS,CHECK-ALIASOBJ32 %s
# RUN: llvm-mc -triple riscv32 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv32 --mattr=+c,+zcheripurecap,+cap-mode -d -M no-aliases - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-BYTES32,CHECK-INST,CHECK-INSTOBJ32 %s

# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding < %s \
# RUN:   | FileCheck -check-prefixes=CHECK,CHECK64,CHECK-ALIAS,CHECK-ALIASASM %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -show-encoding \
# RUN:   -riscv-no-aliases < %s | FileCheck -check-prefixes=CHECK,CHECK64,CHECK-INST,CHECK-INSTASM %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv64 --mattr=+c,+zcheripurecap,+cap-mode -d - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-BYTES64,CHECK-ALIAS,CHECK-ALIASOBJ64 %s
# RUN: llvm-mc -triple riscv64 -mattr=+c,+zcheripurecap,+cap-mode -filetype=obj < %s \
# RUN:   | llvm-objdump  --triple=riscv64 --mattr=+c,+zcheripurecap,+cap-mode -d -M no-aliases - \
# RUN:   | FileCheck -check-prefixes=CHECK-BYTES,CHECK-BYTES64,CHECK-INST,CHECK-INSTOBJ64 %s

# Tests instructions available in purecap + C, rv32 and rv64.

# CHECK-BYTES: e0 1f
# CHECK-ALIAS: caddi cs0, csp, 1020
# CHECK-INST: c.cincoffset4cspn cs0, csp, 1020
# CHECK: # encoding:  [0xe0,0x1f]
caddi cs0, csp, 1020

# CHECK-BYTES: e0 5f
# CHECK-ALIAS: lw s0, 124(ca5)
# CHECK-INST: c.clw s0, 124(ca5)
# CHECK: # encoding: [0xe0,0x5f]
lw s0, 124(ca5)

# CHECK-BYTES: e0 df
# CHECK-ALIAS: sw s0, 124(ca5)
# CHECK-INST: c.csw s0, 124(ca5)
# CHECK: # encoding: [0xe0,0xdf]
sw s0, 124(ca5)

# CHECK-BYTES: 01 00
# CHECK-ALIAS: nop
# CHECK-INST: c.nop
# CHECK: # encoding: [0x01,0x00]
nop

# CHECK-BYTES: 81 10
# CHECK-ALIAS: addi ra, ra, -32
# CHECK-INST: c.addi ra, -32
# CHECK: # encoding:  [0x81,0x10]
addi ra, ra, -32

# CHECK-BYTES: 85 50
# CHECK-ALIAS: li ra, -31
# CHECK-INST: c.li ra, -31
# CHECK: # encoding: [0x85,0x50]
li ra, -31

# CHECK-BYTES: 39 71
# CHECK-ALIAS: addi csp, csp, -64
# CHECK-INST: c.cincoffset16csp csp, -64
# CHECK:  # encoding: [0x39,0x71]
caddi csp, csp, -64

# CHECK-BYTES: fd 61
# CHECK-ALIAS: lui gp, 31
# CHECK-INST: c.lui gp, 31
# CHECK: # encoding:  [0xfd,0x61]
lui gp, 31

# CHECK-BYTES: 7d 80
# CHECK-ALIAS: srli s0, s0, 31
# CHECK-INST: c.srli s0, 31
# CHECK: # encoding:  [0x7d,0x80]
srli s0, s0, 31

# CHECK-BYTES: 7d 84
# CHECK-ALIAS: srai s0, s0, 31
# CHECK-INST: c.srai s0, 31
# CHECK: # encoding: [0x7d,0x84]
srai s0, s0, 31

# CHECK-BYTES: 7d 88
# CHECK-ALIAS: andi s0, s0, 31
# CHECK-INST: c.andi s0, 31
# CHECK: # encoding: [0x7d,0x88]
andi s0, s0, 31

# CHECK-BYTES: 1d 8c
# CHECK-ALIAS: sub s0, s0, a5
# CHECK-INST: c.sub s0, a5
# CHECK: # encoding: [0x1d,0x8c]
sub s0, s0, a5

# CHECK-BYTES: 3d 8c
# CHECK-ALIAS: xor s0, s0, a5
# CHECK-INST: c.xor s0, a5
# CHECK: # encoding: [0x3d,0x8c]
xor s0, s0, a5

# CHECK-BYTES: 3d 8c
# CHECK-ALIAS: xor s0, s0, a5
# CHECK-INST: c.xor s0, a5
# CHECK: # encoding: [0x3d,0x8c]
xor s0, a5, s0

# CHECK-BYTES: 5d 8c
# CHECK-ALIAS: or s0, s0, a5
# CHECK-INST: c.or s0, a5
# CHECK: # encoding:  [0x5d,0x8c]
or s0, s0, a5

# CHECK-BYTES: 45 8c
# CHECK-ALIAS: or s0, s0, s1
# CHECK-INST: c.or s0, s1
# CHECK:  # encoding: [0x45,0x8c]
or  s0, s1, s0

# CHECK-BYTES: 7d 8c
# CHECK-ALIAS: and s0, s0, a5
# CHECK-INST: c.and s0, a5
# CHECK: # encoding: [0x7d,0x8c]
and s0, s0, a5

# CHECK-BYTES: 7d 8c
# CHECK-ALIAS: and s0, s0, a5
# CHECK-INST: c.and s0, a5
# CHECK: # encoding: [0x7d,0x8c]
and s0, a5, s0

# CHECK-BYTES: 01 b0
# CHECK-ALIASASM: j -2048
# CHECK-ALIASOBJ32: j 0xfffff824
# CHECK-ALIASOBJ64: j 0xfffffffffffff824
# CHECK-INSTASM: c.j -2048
# CHECK-INSTOBJ32: c.j 0xfffff824
# CHECK-INSTOBJ64: c.j 0xfffffffffffff824
# CHECK:  # encoding: [0x01,0xb0]
j -2048

# CHECK-BYTES: 01 d0
# CHECK-ALIASASM: beqz s0, -256
# CHECK-ALIASOBJ32: beqz s0, 0xffffff26
# CHECK-ALIASOBJ64: beqz s0, 0xffffffffffffff26
# CHECK-INSTASM: c.beqz s0, -256
# CHECK-INSTOBJ32: c.beqz s0, 0xffffff26
# CHECK-INSTOBJ64: c.beqz s0, 0xffffffffffffff26
# CHECK: # encoding: [0x01,0xd0]
beq s0, zero, -256

# CHECK-BYTES: 7d ec
# CHECK-ALIASASM: bnez s0, 254
# CHECK-ALIASOBJ32: bnez s0, 0x126
# CHECK-ALIASOBJ64: bnez s0, 0x126
# CHECK-INSTASM: c.bnez s0, 254
# CHECK-INSTOBJ32: c.bnez s0, 0x126
# CHECK-INSTOBJ64: c.bnez s0, 0x126
# CHECK: # encoding: [0x7d,0xec]
bne s0, zero, 254

# CHECK-BYTES: 7e 04
# CHECK-ALIAS: slli s0, s0, 31
# CHECK-INST: c.slli s0, 31
# CHECK: # encoding:  [0x7e,0x04]
slli s0, s0, 31

# CHECK-BYTES: fe 50
# CHECK-ALIAS: lw ra, 252(csp)
# CHECK-INST: c.clwcsp  ra, 252(csp)
# CHECK: # encoding:  [0xfe,0x50]
lw ra, 252(csp)

# CHECK-BYTES: 82 80
# CHECK-ALIAS: ret
# CHECK-INST: c.cjr cra
# CHECK: # encoding:  [0x82,0x80]
jalr cnull, 0(cra)

# CHECK-BYTES: 02 90
# CHECK-ALIAS: ebreak
# CHECK-INST: c.ebreak
# CHECK: # encoding: [0x02,0x90]
ebreak

# CHECK-BYTES: 02 94
# CHECK-ALIAS: jalr cs0
# CHECK-INST: c.cjalr cs0
# CHECK: # encoding: [0x02,0x94]
jalr cra, 0(cs0)

# CHECK-BYTES: 3e 94
# CHECK-ALIAS: add s0, s0, a5
# CHECK-INST: c.add s0, a5
# CHECK: # encoding:  [0x3e,0x94]
add s0, a5, s0

# CHECK-BYTES: 3e 94
# CHECK-ALIAS: add s0, s0, a5
# CHECK-INST: c.add s0, a5
# CHECK: # encoding:  [0x3e,0x94]
add s0, s0, a5

# CHECK-BYTES: 82 df
# CHECK-ALIAS: sw zero, 252(csp)
# CHECK-INST: c.cswcsp zero, 252(csp)
# CHECK: # encoding: [0x82,0xdf]
sw zero, 252(csp)

# CHECK-BYTES: 00 00
# CHECK-ALIAS: unimp
# CHECK-INST: c.unimp
# CHECK: # encoding: [0x00,0x00]
unimp

# CHECK-BYTES32: a0 7b
# CHECK-BYTES64: a0 3b
# CHECK-ALIAS: lc cs0, 112(ca5)
# CHECK-INST: c.clc
# CHECK32: # encoding: [0xa0,0x7b]
# CHECK64: # encoding: [0xa0,0x3b]
lc cs0, 112(ca5)

# CHECK-BYTES32: ce 70
# CHECK-BYTES64: ce 30
# CHECK-ALIAS: lc cra, 240(csp)
# CHECK-INST: c.clccsp
# CHECK32: # encoding: [0xce,0x70]
# CHECK64: # encoding: [0xce,0x30]
lc cra, 240(csp)

# CHECK-BYTES32: a0 fb
# CHECK-BYTES64: a0 bb
# CHECK-ALIAS: sc cs0, 112(ca5)
# CHECK-INST: c.csc
# CHECK32: # encoding: [0xa0,0xfb]
# CHECK64: # encoding: [0xa0,0xbb]
sc cs0, 112(ca5)

# CHECK-BYTES32: 86 f9
# CHECK-BYTES64: 86 b9
# CHECK-ALIAS: sc cra, 240(csp)
# CHECK-INST: c.csccsp
# CHECK32: # encoding: [0x86,0xf9]
# CHECK64: # encoding: [0x86,0xb9]
sc cra, 240(csp)
