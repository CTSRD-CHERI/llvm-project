# RUN: llvm-mc %s -triple=riscv32 -mcpu=cheriot -mattr=+xcheri -riscv-no-aliases -show-encoding \
# RUN:     | FileCheck %s
hello:
        cincoffset ca0, cgp, %cheri_compartment_cgprel_lo_i(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_cgprel_lo_i(haha), kind: fixup_riscv_cheri_compartment_cgprel_lo_i
        clw a0, %cheri_compartment_cgprel_lo_i(haha)(cgp)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_cgprel_lo_i(haha), kind: fixup_riscv_cheri_compartment_cgprel_lo_i
        csw a0, %cheri_compartment_cgprel_lo_s(haha)(cgp)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_cgprel_lo_s(haha), kind: fixup_riscv_cheri_compartment_cgprel_lo_s
        auicgp  ca1, %cheri_compartment_cgprel_hi(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_cgprel_hi(haha), kind: fixup_riscv_cheri_compartment_cgprel_hi
        cincoffset ca0, cgp, %cheri_compartment_cgprel_lo_i(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_cgprel_lo_i(haha), kind: fixup_riscv_cheri_compartment_cgprel_lo_i
        csetbounds ca0, ca0, %cheri_compartment_size(haha)
        # CHECK: fixup A - offset: 0, value: %cheri_compartment_size(haha), kind: fixup_riscv_cheri_compartment_size
