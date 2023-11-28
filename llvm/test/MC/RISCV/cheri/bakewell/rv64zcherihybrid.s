# RUN: llvm-mc %s -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:     | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: gchi a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x45,0x10]
gchi a0, ca0

# CHECK-ASM-AND-OBJ: schi ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x35,0xa5,0x0c]
schi ca0, ca0, a0

# CHECK-ASM-AND-OBJ: jalr.mode ca0, 0(ca0)
# CHECK-ASM: encoding: [0x67,0x15,0x05,0x00]
jalr ca0, 0(ca0)

# CHECK-ASM-AND-OBJ: jalr.mode ca0, 0(ca0)
# CHECK-ASM: encoding: [0x67,0x15,0x05,0x00]
jalr ca0, ca0

# CHECK-ASM-AND-OBJ: jalr.mode cra, 0(ca0)
# CHECK-ASM: encoding: [0xe7,0x10,0x05,0x00]
jalr ca0

# CHECK-ASM-AND-OBJ: jalr.mode cnull, 0(ca0)
# CHECK-ASM: encoding: [0x67,0x10,0x05,0x00]
jr ca0

# CHECK-ASM-AND-OBJ: jalr.mode cnull, 0(cra)
# CHECK-ASM: encoding: [0x67,0x90,0x00,0x00]
ret.mode

# CHECK-ASM-AND-OBJ: cmv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
cmv ca0, ca0

# CHECK-ASM-AND-OBJ: cmv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
mv ca0, ca0

# CHECK-ASM-AND-OBJ: cadd ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x05,0xb5,0x0c]
cadd ca0, ca0, a1

# CHECK-ASM-AND-OBJ: cadd ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x05,0xb5,0x0c]
add ca0, ca0, a1

# CHECK-ASM-AND-OBJ: caddi ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
caddi ca0, ca0, 12

# CHECK-ASM-AND-OBJ: caddi ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
add ca0, ca0, 12

# CHECK-ASM-AND-OBJ: scaddr ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x15,0xb5,0x0c]
scaddr ca0, ca0, a1

# CHECK-ASM-AND-OBJ: acperm ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x25,0xa5,0x0c]
acperm ca0, ca0, a0

# CHECK-ASM-AND-OBJ: sceq a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x45,0xa5,0x0c]
sceq a0, ca0, ca0

# CHECK-ASM-AND-OBJ: sentry ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x85,0x10]
sentry ca0, ca0

# CHECK-ASM-AND-OBJ: scbndsi ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
scbnds ca0, ca0, 12

# CHECK-ASM-AND-OBJ: scbndsi ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
scbndsi ca0, ca0, 12

# CHECK-ASM-AND-OBJ: scbndsi ca0, ca0, 48
# CHECK-ASM: encoding: [0x13,0x55,0x35,0x06]
scbnds ca0, ca0, 48

# CHECK-ASM-AND-OBJ: scbndsi ca0, ca0, 496
# CHECK-ASM: encoding: [0x13,0x55,0xf5,0x07]
scbnds ca0, ca0, 496

# CHECK-ASM-AND-OBJ: scbnds ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x05,0xa5,0x0e]
scbnds ca0, ca0, a0

# CHECK-ASM-AND-OBJ: scss a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x65,0xa5,0x0c]
scss a0, ca0, ca0

# CHECK-ASM-AND-OBJ: cbld ca0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x55,0xa5,0x0c]
cbld ca0, ca0, ca0

# CHECK-ASM-AND-OBJ: gctag a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x10]
gctag a0, ca0

# CHECK-ASM-AND-OBJ: gcperm a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x15,0x10]
gcperm a0, ca0

# CHECK-ASM-AND-OBJ: gcbase a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x55,0x10]
gcbase a0, ca0

# CHECK-ASM-AND-OBJ: gclen a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x65,0x10]
gclen a0, ca0

# CHECK-ASM-AND-OBJ: cram a0, a0  
# CHECK-ASM: encoding: [0x33,0x05,0x75,0x10]
cram a0, a0

# CHECK-ASM-AND-OBJ: lc ca0, 0(a0)
# CHECK-ASM: encoding: [0x0f,0x25,0x05,0x00]
lc ca0, 0(a0)

# CHECK-ASM-AND-OBJ: sc ca0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x40,0xa5,0x00]
sc ca0, 0(a0)

# CHECK-ASM-AND-OBJ: modesw
# CHECK-ASM: encoding: [0x33,0x10,0x00,0x12]
modesw

# CHECK-ASM-AND-OBJ: scmode
# CHECK-ASM: encoding: [0x33,0x75,0xb5,0x0c]
scmode ca0, ca0, a1
