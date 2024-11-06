# RUN: not llvm-mc -triple riscv32 -mattr=+zicbom,+zcheripurecap,+cap-mode < %s 2>&1 | FileCheck %s
# RUN: not llvm-mc -triple riscv64 -mattr=+zicbom,+zcheripurecap,+cap-mode < %s 2>&1 | FileCheck %s

# Must have a single register argument.
cbo.clean # CHECK: :[[@LINE]]:1: error: too few operands for instruction
cbo.flush # CHECK: :[[@LINE]]:1: error: too few operands for instruction
cbo.inval # CHECK: :[[@LINE]]:1: error: too few operands for instruction

cbo.clean 1 # CHECK: :[[@LINE]]:13: error: expected '(' after optional integer offset
cbo.flush 2 # CHECK: :[[@LINE]]:13: error: expected '(' after optional integer offset
cbo.inval 3 # CHECK: :[[@LINE]]:13: error: expected '(' after optional integer offset

cbo.clean ct0, ct1 # CHECK: :[[@LINE]]:11: error: expected '(' or optional integer offset
cbo.flush ct0, ct1 # CHECK: :[[@LINE]]:11: error: expected '(' or optional integer offset
cbo.inval ct0, ct1 # CHECK: :[[@LINE]]:11: error: expected '(' or optional integer offset

# Non-zero offsets are not supported.
cbo.clean 1(ct0) # CHECK: :[[@LINE]]:11: error: optional integer offset must be 0
cbo.flush 2(ct0) # CHECK: :[[@LINE]]:11: error: optional integer offset must be 0
cbo.inval 3(ct0) # CHECK: :[[@LINE]]:11: error: optional integer offset must be 0

# Instructions from other zicbo* extensions aren't available without enabling
# the appropriate -mattr flag.
cbo.zero (ct0) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicboz' (Cache-Block Zero Instructions){{$}}
prefetch.i 0(ct3) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbop' (Cache-Block Prefetch Instructions){{$}}
prefetch.r 0(ct4) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbop' (Cache-Block Prefetch Instructions){{$}}
prefetch.w 0(ct5) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbop' (Cache-Block Prefetch Instructions){{$}}
