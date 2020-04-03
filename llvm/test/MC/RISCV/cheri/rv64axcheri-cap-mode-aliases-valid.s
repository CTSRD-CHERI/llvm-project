# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode -riscv-no-aliases \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s
# RUN: llvm-mc %s -triple=riscv64 -mattr=+a,+xcheri,+cap-mode \
# RUN:     | FileCheck -check-prefix=CHECK-ALIAS %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -d --mattr=+a,+xcheri,+cap-mode -M no-aliases - \
# RUN:     | FileCheck -check-prefix=CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple riscv64 -mattr=+a,+xcheri,+cap-mode < %s \
# RUN:     | llvm-objdump -d --mattr=+a,+xcheri,+cap-mode - \
# RUN:     | FileCheck -check-prefix=CHECK-ALIAS %s

# The below tests for clr.d, csc.d and camo*.d, using `0(reg)` are actually
# implemented using a custom parser, but we test them as if they're aliases.

# CHECK-INST: clr.d a1, (ca0)
# CHECK-ALIAS: clr.d a1, (ca0)
clr.d a1, 0(ca0)

# CHECK-INST: clr.d.aq a1, (ca0)
# CHECK-ALIAS: clr.d.aq a1, (ca0)
clr.d.aq a1, 0(ca0)

# CHECK-INST: clr.d.rl a1, (ca0)
# CHECK-ALIAS: clr.d.rl a1, (ca0)
clr.d.rl a1, 0(ca0)

# CHECK-INST: clr.d.aqrl a1, (ca0)
# CHECK-ALIAS: clr.d.aqrl a1, (ca0)
clr.d.aqrl a1, 0(ca0)

# CHECK-INST: csc.d a2, a1, (ca0)
# CHECK-ALIAS: csc.d a2, a1, (ca0)
csc.d a2, a1, 0(ca0)

# CHECK-INST: csc.d.aq a2, a1, (ca0)
# CHECK-ALIAS: csc.d.aq a2, a1, (ca0)
csc.d.aq a2, a1, 0(ca0)

# CHECK-INST: csc.d.rl a2, a1, (ca0)
# CHECK-ALIAS: csc.d.rl a2, a1, (ca0)
csc.d.rl a2, a1, 0(ca0)

# CHECK-INST: csc.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: csc.d.aqrl a2, a1, (ca0)
csc.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camoswap.d a2, a1, (ca0)
# CHECK-ALIAS: camoswap.d a2, a1, (ca0)
camoswap.d a2, a1, 0(ca0)

# CHECK-INST: camoswap.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camoswap.d.aq a2, a1, (ca0)
camoswap.d.aq a2, a1, 0(ca0)

# CHECK-INST: camoswap.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camoswap.d.rl a2, a1, (ca0)
camoswap.d.rl a2, a1, 0(ca0)

# CHECK-INST: camoswap.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camoswap.d.aqrl a2, a1, (ca0)
camoswap.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camoadd.d a2, a1, (ca0)
# CHECK-ALIAS: camoadd.d a2, a1, (ca0)
camoadd.d a2, a1, 0(ca0)

# CHECK-INST: camoadd.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camoadd.d.aq a2, a1, (ca0)
camoadd.d.aq a2, a1, 0(ca0)

# CHECK-INST: camoadd.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camoadd.d.rl a2, a1, (ca0)
camoadd.d.rl a2, a1, 0(ca0)

# CHECK-INST: camoadd.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camoadd.d.aqrl a2, a1, (ca0)
camoadd.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camoxor.d a2, a1, (ca0)
# CHECK-ALIAS: camoxor.d a2, a1, (ca0)
camoxor.d a2, a1, 0(ca0)

# CHECK-INST: camoxor.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camoxor.d.aq a2, a1, (ca0)
camoxor.d.aq a2, a1, 0(ca0)

# CHECK-INST: camoxor.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camoxor.d.rl a2, a1, (ca0)
camoxor.d.rl a2, a1, 0(ca0)

# CHECK-INST: camoxor.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camoxor.d.aqrl a2, a1, (ca0)
camoxor.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camoand.d a2, a1, (ca0)
# CHECK-ALIAS: camoand.d a2, a1, (ca0)
camoand.d a2, a1, 0(ca0)

# CHECK-INST: camoand.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camoand.d.aq a2, a1, (ca0)
camoand.d.aq a2, a1, 0(ca0)

# CHECK-INST: camoand.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camoand.d.rl a2, a1, (ca0)
camoand.d.rl a2, a1, 0(ca0)

# CHECK-INST: camoand.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camoand.d.aqrl a2, a1, (ca0)
camoand.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camoor.d a2, a1, (ca0)
# CHECK-ALIAS: camoor.d a2, a1, (ca0)
camoor.d a2, a1, 0(ca0)

# CHECK-INST: camoor.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camoor.d.aq a2, a1, (ca0)
camoor.d.aq a2, a1, 0(ca0)

# CHECK-INST: camoor.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camoor.d.rl a2, a1, (ca0)
camoor.d.rl a2, a1, 0(ca0)

# CHECK-INST: camoor.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camoor.d.aqrl a2, a1, (ca0)
camoor.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camomin.d a2, a1, (ca0)
# CHECK-ALIAS: camomin.d a2, a1, (ca0)
camomin.d a2, a1, 0(ca0)

# CHECK-INST: camomin.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camomin.d.aq a2, a1, (ca0)
camomin.d.aq a2, a1, 0(ca0)

# CHECK-INST: camomin.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camomin.d.rl a2, a1, (ca0)
camomin.d.rl a2, a1, 0(ca0)

# CHECK-INST: camomin.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camomin.d.aqrl a2, a1, (ca0)
camomin.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camomax.d a2, a1, (ca0)
# CHECK-ALIAS: camomax.d a2, a1, (ca0)
camomax.d a2, a1, 0(ca0)

# CHECK-INST: camomax.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camomax.d.aq a2, a1, (ca0)
camomax.d.aq a2, a1, 0(ca0)

# CHECK-INST: camomax.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camomax.d.rl a2, a1, (ca0)
camomax.d.rl a2, a1, 0(ca0)

# CHECK-INST: camomax.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camomax.d.aqrl a2, a1, (ca0)
camomax.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camominu.d a2, a1, (ca0)
# CHECK-ALIAS: camominu.d a2, a1, (ca0)
camominu.d a2, a1, 0(ca0)

# CHECK-INST: camominu.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camominu.d.aq a2, a1, (ca0)
camominu.d.aq a2, a1, 0(ca0)

# CHECK-INST: camominu.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camominu.d.rl a2, a1, (ca0)
camominu.d.rl a2, a1, 0(ca0)

# CHECK-INST: camominu.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camominu.d.aqrl a2, a1, (ca0)
camominu.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: camomaxu.d a2, a1, (ca0)
# CHECK-ALIAS: camomaxu.d a2, a1, (ca0)
camomaxu.d a2, a1, 0(ca0)

# CHECK-INST: camomaxu.d.aq a2, a1, (ca0)
# CHECK-ALIAS: camomaxu.d.aq a2, a1, (ca0)
camomaxu.d.aq a2, a1, 0(ca0)

# CHECK-INST: camomaxu.d.rl a2, a1, (ca0)
# CHECK-ALIAS: camomaxu.d.rl a2, a1, (ca0)
camomaxu.d.rl a2, a1, 0(ca0)

# CHECK-INST: camomaxu.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: camomaxu.d.aqrl a2, a1, (ca0)
camomaxu.d.aqrl a2, a1, 0(ca0)
