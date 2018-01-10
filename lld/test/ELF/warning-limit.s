# REQUIRES: x86
# RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t1.o

# RUN: ld.lld %t1.o -o %t5 --warn-unresolved-symbols -warning-limit=5 2>&1 | \
# RUN: FileCheck -check-prefix=LIMITED %s
# LIMITED: warning: undefined symbol: undef
# LIMITED: warning: undefined symbol: undef2
# LIMITED: warning: too many warnings emitted, stopping now (use -warning-limit=0 to see all warnings)

# RUN: ld.lld %t1.o -o %t5 --warn-unresolved-symbols -warning-limit=0 2>&1 | \
# RUN: FileCheck -check-prefix=UNLIMITED %s
# UNLIMITED: warning: undefined symbol: undef
# UNLIMITED: warning: undefined symbol: undef1
# UNLIMITED: warning: undefined symbol: undef2
# UNLIMITED: warning: undefined symbol: undef3
# UNLIMITED: warning: undefined symbol: undef4
# UNLIMITED: warning: undefined symbol: undef5

.globl _start
_start:

.globl _shared
_shared:
 callq undef@PLT
 callq undef1@PLT
 callq undef2@PLT
 callq undef3@PLT
 callq undef4@PLT
 callq undef5@PLT
