# RUN: llvm-mc -triple=mips-unknown-freebsd -show-encoding -mcpu=mips3 %s | FileCheck %s
	j __cerror
# CHECK: fixup A - offset: 0, value: __cerror, kind: fixup_Mips_26
