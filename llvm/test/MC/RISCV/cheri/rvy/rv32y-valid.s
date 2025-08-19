# RUN: llvm-mc %s -triple=riscv32 -mattr=+y -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+y < %s \
# RUN:     | llvm-objdump --mattr=+y -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s
#
# RUN: llvm-mc %s -triple=riscv64 -mattr=+y -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM,CHECK-ASM-AND-OBJ %s
# RUN: llvm-mc -filetype=obj -triple=riscv64 -mattr=+y < %s \
# RUN:     | llvm-objdump --mattr=+y -M no-aliases -d -r - \
# RUN:     | FileCheck -check-prefixes=CHECK-ASM-AND-OBJ %s

# CHECK-ASM-AND-OBJ: ytagr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x10]
ytagr a0, ca0
# CHECK-ASM-AND-OBJ: ytagr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x10]
gctag a0, ca0
# CHECK-ASM-AND-OBJ: ytagr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x10]
cgettag a0, ca0

# CHECK-ASM-AND-OBJ: ypermr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x15,0x10]
ypermr a0, ca0
# CHECK-ASM-AND-OBJ: ypermr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x15,0x10]
gcperm a0, ca0
# CHECK-ASM-AND-OBJ: ypermr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x15,0x10]
cgetperm a0, ca0

# CHECK-ASM-AND-OBJ: yhir a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x45,0x10]
yhir a0, ca0
# CHECK-ASM-AND-OBJ: yhir a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x45,0x10]
gchi a0, ca0
# CHECK-ASM-AND-OBJ: yhir a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x45,0x10]
cgethigh a0, ca0

# CHECK-ASM-AND-OBJ: ybaser a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x55,0x10]
ybaser a0, ca0
# CHECK-ASM-AND-OBJ: ybaser a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x55,0x10]
gcbase a0, ca0
# CHECK-ASM-AND-OBJ: ybaser a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x55,0x10]
cgetbase a0, ca0

# CHECK-ASM-AND-OBJ: ylenr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x65,0x10]
ylenr a0, ca0
# CHECK-ASM-AND-OBJ: ylenr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x65,0x10]
gclen a0, ca0
# CHECK-ASM-AND-OBJ: ylenr a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x65,0x10]
cgetlen a0, ca0

# CHECK-ASM-AND-OBJ: ytyper a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x25,0x10]
ytyper a0, ca0
# CHECK-ASM-AND-OBJ: ytyper a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x25,0x10]
gctype a0, ca0
# CHECK-ASM-AND-OBJ: ytyper a0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x25,0x10]
cgettype a0, ca0

# CHECK-ASM-AND-OBJ: yaddrw ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x15,0xb5,0x0c]
yaddrw ca0, ca0, a1
# CHECK-ASM-AND-OBJ: yaddrw ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x15,0xb5,0x0c]
scaddr ca0, ca0, a1
# CHECK-ASM-AND-OBJ: yaddrw ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x15,0xb5,0x0c]
csetaddr ca0, ca0, a1

# CHECK-ASM-AND-OBJ: ypermc ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x25,0xa5,0x0c]
ypermc ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ypermc ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x25,0xa5,0x0c]
acperm ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ypermc ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x25,0xa5,0x0c]
candperm ca0, ca0, a0

# CHECK-ASM-AND-OBJ: yhiw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x35,0xa5,0x0c]
yhiw ca0, ca0, a0
# CHECK-ASM-AND-OBJ: yhiw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x35,0xa5,0x0c]
schi ca0, ca0, a0
# CHECK-ASM-AND-OBJ: yhiw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x35,0xa5,0x0c]
csethigh ca0, ca0, a0

# CHECK-ASM-AND-OBJ: addy ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x05,0xb5,0x0c]
addy ca0, ca0, a1
# CHECK-ASM-AND-OBJ: addy ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x05,0xb5,0x0c]
cadd ca0, ca0, a1
# CHECK-ASM-AND-OBJ: addy ca0, ca0, a1
# CHECK-ASM: encoding: [0x33,0x05,0xb5,0x0c]
cincoffset ca0, ca0, a1

# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
addiy ca0, ca0, 12
# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
add ca0, ca0, 12
# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
cadd ca0, ca0, 12
# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
caddi ca0, ca0, 12
# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
cincoffset ca0, ca0, 12
# CHECK-ASM-AND-OBJ: addiy ca0, ca0, 12
# CHECK-ASM: encoding: [0x1b,0x25,0xc5,0x00]
cincoffsetimm ca0, ca0, 12

# CHECK-ASM-AND-OBJ: ysentry ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x85,0x10]
ysentry ca0, ca0
# CHECK-ASM-AND-OBJ: ysentry ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x85,0x10]
sentry ca0, ca0
# CHECK-ASM-AND-OBJ: ysentry ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x85,0x10]
csealentry ca0, ca0

# CHECK-ASM-AND-OBJ: ybld ca0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x55,0xa5,0x0c]
ybld ca0, ca0, ca0
# CHECK-ASM-AND-OBJ: ybld ca0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x55,0xa5,0x0c]
cbld ca0, ca0, ca0
# CHECK-ASM-AND-OBJ: ybld ca0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x55,0xa5,0x0c]
cbuildcap ca0, ca0, ca0

# CHECK-ASM-AND-OBJ: ybndsw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x05,0xa5,0x0e]
ybndsw ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ybndsw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x05,0xa5,0x0e]
scbnds ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ybndsw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x05,0xa5,0x0e]
csetboundsexact ca0, ca0, a0

# CHECK-ASM-AND-OBJ: ybndsrw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x15,0xa5,0x0e]
ybndsrw ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ybndsrw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x15,0xa5,0x0e]
scbndsr ca0, ca0, a0
# CHECK-ASM-AND-OBJ: ybndsrw ca0, ca0, a0
# CHECK-ASM: encoding: [0x33,0x15,0xa5,0x0e]
csetbounds ca0, ca0, a0

# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
ybndsiw ca0, ca0, 12
# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
ybndsw ca0, ca0, 12
# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
scbndsi ca0, ca0, 12
# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
scbnds ca0, ca0, 12
# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
csetbounds ca0, ca0, 12
# CHECK-ASM-AND-OBJ: ybndsiw ca0, ca0, 12
# CHECK-ASM: encoding: [0x13,0x55,0xc5,0x04]
csetboundsimm ca0, ca0, 12

# CHECK-ASM-AND-OBJ: ymv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
ymv ca0, ca0
# CHECK-ASM-AND-OBJ: ymv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
mv ca0, ca0
# CHECK-ASM-AND-OBJ: ymv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
cmv ca0, ca0
# CHECK-ASM-AND-OBJ: ymv ca0, ca0
# CHECK-ASM: encoding: [0x33,0x05,0x05,0x0c]
cmove ca0, ca0

# CHECK-ASM-AND-OBJ: yamask a0, a0
# CHECK-ASM: encoding: [0x33,0x05,0x75,0x10]
yamask a0, a0
# CHECK-ASM-AND-OBJ: yamask a0, a0
# CHECK-ASM: encoding: [0x33,0x05,0x75,0x10]
cram a0, a0
# CHECK-ASM-AND-OBJ: yamask a0, a0
# CHECK-ASM: encoding: [0x33,0x05,0x75,0x10]
crepresentablealignmentmask a0, a0

# CHECK-ASM-AND-OBJ: ylt a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x65,0xa5,0x0c]
ylt a0, ca0, ca0
# CHECK-ASM-AND-OBJ: ylt a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x65,0xa5,0x0c]
scss a0, ca0, ca0
# CHECK-ASM-AND-OBJ: ylt a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x65,0xa5,0x0c]
ctestsubset a0, ca0, ca0

# CHECK-ASM-AND-OBJ: syeq a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x45,0xa5,0x0c]
syeq a0, ca0, ca0
# CHECK-ASM-AND-OBJ: syeq a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x45,0xa5,0x0c]
sceq a0, ca0, ca0
# CHECK-ASM-AND-OBJ: syeq a0, ca0, ca0
# CHECK-ASM: encoding: [0x33,0x45,0xa5,0x0c]
csetequalexact a0, ca0, ca0

# CHECK-ASM-AND-OBJ: ly ca0, 0(a0)
# CHECK-ASM: encoding: [0x0f,0x45,0x05,0x00]
ly ca0, 0(a0)
# CHECK-ASM-AND-OBJ: ly ca0, 0(a0)
# CHECK-ASM: encoding: [0x0f,0x45,0x05,0x00]
lc ca0, 0(a0)
# CHECK-ASM-AND-OBJ: ly ca0, 0(a0)
# CHECK-ASM: encoding: [0x0f,0x45,0x05,0x00]
ly ca0, (a0)
# CHECK-ASM-AND-OBJ: ly ca0, 0(a0)
# CHECK-ASM: encoding: [0x0f,0x45,0x05,0x00]
lc ca0, (a0)

# CHECK-ASM-AND-OBJ: sy ca0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x40,0xa5,0x00]
sy ca0, 0(a0)
# CHECK-ASM-AND-OBJ: sy ca0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x40,0xa5,0x00]
sc ca0, 0(a0)
# CHECK-ASM-AND-OBJ: sy ca0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x40,0xa5,0x00]
sy ca0, (a0)
# CHECK-ASM-AND-OBJ: sy ca0, 0(a0)
# CHECK-ASM: encoding: [0x23,0x40,0xa5,0x00]
sc ca0, (a0)
