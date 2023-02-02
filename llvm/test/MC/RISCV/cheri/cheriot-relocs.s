# RUN: llvm-mc %s -triple=riscv32 -mattr=+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck %s
hello:
        cincoffset ca0, cgp, %cheri_compartment_rel(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_rel(haha), kind: fixup_riscv_cheri_tiny_compartment_global
        csetbounds ca0, ca0, %cheri_size(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_size(haha), kind: fixup_riscv_cheri_tiny_size
