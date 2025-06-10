# RUN: llvm-mc -triple riscv64 -mattr=+xcheri,+cap-mode %s -o - | FileCheck %s --check-prefix=XCHERI
# RUN: llvm-mc -triple riscv64 -mattr=+zcheripurecap,+cap-mode %s -o - | FileCheck %s --check-prefix=ZCHERI

## Allow the use of cheri-v9 mnemonics/aliases to compile existing code.

# CHECK: auipc ca0, 2
auipcc ca0, 2

# CHECK: tail foo
ctail foo@plt

# CHECK: ret
cret

# CHECK: call foo
ccall foo@plt

# CHECK: call ca0, foo
ccall ca0, foo@plt

# CHECK-LABEL: .Lpcrel_hi0:
# CHECK-NEXT: auipc ct0, %got_pcrel_hi(foo)
# CHECK-NEXT: lc ct0, %pcrel_lo(.Lpcrel_hi0)(ct0)
clgc ct0, foo

# CHECK-LABEL: .Lpcrel_hi1:
# CHECK-NEXT:     auipc ct0, %pcrel_hi(foo)
# XCHERI:    cincoffset ct0, ct0, %pcrel_lo(.Lpcrel_hi1)
# ZCHERI:    caddi      ct0, ct0, %pcrel_lo(.Lpcrel_hi1)
cllc ct0, foo

