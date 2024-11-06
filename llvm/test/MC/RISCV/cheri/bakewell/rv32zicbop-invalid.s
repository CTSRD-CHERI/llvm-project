# RUN: not llvm-mc -triple riscv32 -mattr=+zicbop,+zcheripurecap,+cap-mode < %s 2>&1 | FileCheck %s
# RUN: not llvm-mc -triple riscv64 -mattr=+zicbop,+zcheripurecap,+cap-mode < %s 2>&1 | FileCheck %s

# Memory operand not formatted correctly.
prefetch.i ca0, 32 # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.r 32, ca0 # CHECK: :[[@LINE]]:16: error: invalid operand for instruction
prefetch.w ca0(32) # CHECK: :[[@LINE]]:15: error: unexpected token

# Out of range offset.
prefetch.i -2080(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.r 2048(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.w 2050(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]

# Offsets that aren't multiples of 32.
prefetch.i 31(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.r -31(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.w 2047(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]

# Symbols should not be accepted.
prefetch.i foo(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.r %lo(foo)(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]
prefetch.w %pcrel_lo(foo)(ca0) # CHECK: :[[@LINE]]:12: error: immediate must be a multiple of 32 bytes in the range [-2048, 2016]

# Instructions from other zicbo* extensions aren't available without enabling
# the appropriate -mattr flag.
cbo.clean (ct0) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbom' (Cache-Block Management Instructions){{$}}
cbo.flush (ct1) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbom' (Cache-Block Management Instructions){{$}}
cbo.inval (ct2) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicbom' (Cache-Block Management Instructions){{$}}
cbo.zero (ct0) # CHECK: :[[@LINE]]:1: error: instruction requires the following: 'Zicboz' (Cache-Block Zero Instructions){{$}}
