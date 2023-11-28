# RUN: llvm-mc -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid -show-encoding < %s \
# RUN:    | FileCheck -check-prefixes=CHECK-MC %s

# RUN: llvm-mc -filetype=obj -triple=riscv32 -mattr=+zcheripurecap,+zcherihybrid < %s \
# RUN:    | llvm-objdump --mattr=+zcheripurecap,+zcherihybrid -M no-aliases -d -r - \
# RUN:    | FileCheck -check-prefixes=CHECK-OBJ %s

# CHECK-MC: .text
# CHECK-OBJ: Disassembly of section .text:

# CHECK-MC: auipc a0, 180
# CHECK-OBJ: [[ADDR1:[0-9]+]] <$x<nocapmode>.[[#INDEX:]]>:
# CHECK-OBJ: [[#LINE:]]: 17 45 0b 00   auipc   a0, 180
auipc a0, 180

# CHECK-MC: .option capmode
# CHECK-MC: auipc ca0, 180
# CHECK-OBJ: [[ADDR2:[0-9]+]] <$x<capmode>.[[#INDEX+1]]>:
# CHECK-OBJ: [[#LINE:]]: 17 45 0b 00   auipc   ca0, 180
.option capmode
auipc ca0, 180

# CHECK-MC: .option nocapmode
# CHECK-MC: auipc a0, 180
# CHECK-OBJ: [[ADDR3:[0-9]+]] <$x<nocapmode>.[[#INDEX+2]]>:
# CHECK-OBJ: [[#LINE:]]: 17 45 0b 00   auipc   a0, 180
.option nocapmode
auipc a0, 180
