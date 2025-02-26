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

# CHECK-INST: {{[[:<:]]}}lr.d a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}lr.d a1, (ca0)
clr.d a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}lr.d.aq a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}lr.d.aq a1, (ca0)
clr.d.aq a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}lr.d.rl a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}lr.d.rl a1, (ca0)
clr.d.rl a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}lr.d.aqrl a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}lr.d.aqrl a1, (ca0)
clr.d.aqrl a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}sc.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}sc.d a2, a1, (ca0)
csc.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}sc.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}sc.d.aq a2, a1, (ca0)
csc.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}sc.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}sc.d.rl a2, a1, (ca0)
csc.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}sc.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}sc.d.aqrl a2, a1, (ca0)
csc.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoswap.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoswap.d a2, a1, (ca0)
camoswap.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoswap.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoswap.d.aq a2, a1, (ca0)
camoswap.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoswap.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoswap.d.rl a2, a1, (ca0)
camoswap.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoswap.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoswap.d.aqrl a2, a1, (ca0)
camoswap.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoadd.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoadd.d a2, a1, (ca0)
camoadd.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoadd.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoadd.d.aq a2, a1, (ca0)
camoadd.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoadd.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoadd.d.rl a2, a1, (ca0)
camoadd.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoadd.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoadd.d.aqrl a2, a1, (ca0)
camoadd.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoxor.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoxor.d a2, a1, (ca0)
camoxor.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoxor.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoxor.d.aq a2, a1, (ca0)
camoxor.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoxor.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoxor.d.rl a2, a1, (ca0)
camoxor.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoxor.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoxor.d.aqrl a2, a1, (ca0)
camoxor.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoand.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoand.d a2, a1, (ca0)
camoand.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoand.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoand.d.aq a2, a1, (ca0)
camoand.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoand.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoand.d.rl a2, a1, (ca0)
camoand.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoand.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoand.d.aqrl a2, a1, (ca0)
camoand.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoor.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoor.d a2, a1, (ca0)
camoor.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoor.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoor.d.aq a2, a1, (ca0)
camoor.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoor.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoor.d.rl a2, a1, (ca0)
camoor.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amoor.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amoor.d.aqrl a2, a1, (ca0)
camoor.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomin.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomin.d a2, a1, (ca0)
camomin.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomin.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomin.d.aq a2, a1, (ca0)
camomin.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomin.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomin.d.rl a2, a1, (ca0)
camomin.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomin.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomin.d.aqrl a2, a1, (ca0)
camomin.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomax.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomax.d a2, a1, (ca0)
camomax.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomax.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomax.d.aq a2, a1, (ca0)
camomax.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomax.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomax.d.rl a2, a1, (ca0)
camomax.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomax.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomax.d.aqrl a2, a1, (ca0)
camomax.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amominu.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amominu.d a2, a1, (ca0)
camominu.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amominu.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amominu.d.aq a2, a1, (ca0)
camominu.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amominu.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amominu.d.rl a2, a1, (ca0)
camominu.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amominu.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amominu.d.aqrl a2, a1, (ca0)
camominu.d.aqrl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomaxu.d a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomaxu.d a2, a1, (ca0)
camomaxu.d a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomaxu.d.aq a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomaxu.d.aq a2, a1, (ca0)
camomaxu.d.aq a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomaxu.d.rl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomaxu.d.rl a2, a1, (ca0)
camomaxu.d.rl a2, a1, 0(ca0)

# CHECK-INST: {{[[:<:]]}}amomaxu.d.aqrl a2, a1, (ca0)
# CHECK-ALIAS: {{[[:<:]]}}amomaxu.d.aqrl a2, a1, (ca0)
camomaxu.d.aqrl a2, a1, 0(ca0)
