
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi sandbox \
# RUN:          -mcpu=cheri %s -o %t-cheri256-main.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi sandbox \
# RUN:          -mcpu=cheri128 %s -o %t-cheri128-main.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi n64 \
# RUN:          -mcpu=cheri %s -o %t-cheri256-hybrid-main.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi n64 \
# RUN:          -mcpu=cheri128 %s -o %t-cheri128-hybrid-main.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi sandbox \
# RUN:          -mcpu=cheri %S/Inputs/mips-dynamic.s -o %t-cheri256-lib.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi sandbox \
# RUN:          -mcpu=cheri128 %S/Inputs/mips-dynamic.s -o %t-cheri128-lib.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi n64 \
# RUN:          -mcpu=cheri %S/Inputs/mips-dynamic.s -o %t-cheri256-hybrid-lib.o
# RUN: llvm-mc -filetype=obj -triple=cheri-unknown-freebsd -target-abi n64 \
# RUN:          -mcpu=cheri128 %S/Inputs/mips-dynamic.s -o %t-cheri128-hybrid-lib.o
# RUN: llvm-readobj -h %t-cheri128-main.o | FileCheck -check-prefix=CHERI128-FLAGS %s
# RUN: llvm-readobj -h %t-cheri256-main.o | FileCheck -check-prefix=CHERI256-FLAGS %s
# RUN: llvm-readobj -h %t-cheri128-hybrid-main.o | FileCheck -check-prefix=CHERI128-HYBRID-FLAGS %s
# RUN: llvm-readobj -h %t-cheri256-hybrid-main.o | FileCheck -check-prefix=CHERI256-HYBRID-FLAGS %s


# Now check that lld does not allow to link incompatible CHERI files

# RUN: llvm-mc -filetype=obj -triple=mips64-unknown-freebsd \
# RUN:         %S/Inputs/mips-dynamic.s -o %t-mips64.o
# RUN: not ld.lld %t-cheri256.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-MIPS %s
# RUN: not ld.lld %t-cheri128.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-MIPS %s
# RUN: not ld.lld %t-cheri256.o %t-cheri128-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-CHERI128 %s
# RUN: not ld.lld %t-cheri128.o %t-cheri256-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-CHERI256 %s
# RUN: not ld.lld %t-cheri256.o %t-cheri256-hybrid-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-vs-CHERI256-HYBRID %s
# RUN: not ld.lld %t-cheri128.o %t-cheri128-hybrid-lib.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-vs-CHERI128-HYBRID %s
# RUN: not ld.lld %t-cheri256-hybrid.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI256-HYBRID-vs-MIPS %s
# RUN: not ld.lld %t-cheri128-hybrid.o %t-mips64.o -o %t.exe 2>&1 | FileCheck -check-prefix=CHERI128-HYBRID-vs-MIPS %s

# REQUIRES: mips

  .option pic0
  .text
  .global  __start
__start:
  nop

# CHERI256-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI256-FLAGS:      Flags [ (0x30C25006)
# CHERI256-FLAGS-NEXT:    EF_MIPS_ABI_CHERIABI (0x5000)
# CHERI256-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI256-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI256-FLAGS-NEXT:    EF_MIPS_MACH_CHERI256 (0xC20000)
# CHERI256-FLAGS-NEXT:    EF_MIPS_PIC (0x2)
# CHERI256-FLAGS-NEXT:  ]
# CHERI128-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI128-FLAGS:      Flags [ (0x30C15006)
# CHERI128-FLAGS-NEXT:    EF_MIPS_ABI_CHERIABI (0x5000)
# CHERI128-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI128-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI128-FLAGS-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
# CHERI128-FLAGS-NEXT:    EF_MIPS_PIC (0x2)
# CHERI128-FLAGS-NEXT:  ]

# CHERI256-HYBRID-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI256-HYBRID-FLAGS:      Flags [ (0x30C20006)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_MACH_CHERI256 (0xC20000)
# CHERI256-HYBRID-FLAGS-NEXT:    EF_MIPS_PIC (0x2)
# CHERI256-HYBRID-FLAGS-NEXT:  ]
# CHERI128-HYBRID-FLAGS:      Machine: EM_MIPS (0x8)
# CHERI128-HYBRID-FLAGS:      Flags [ (0x30C10006)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_ARCH_4 (0x30000000)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_CPIC (0x4)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
# CHERI128-HYBRID-FLAGS-NEXT:    EF_MIPS_PIC (0x2)
# CHERI128-HYBRID-FLAGS-NEXT:  ]

# CHERI256-vs-MIPS: target ABI 'sandbox' is incompatible with 'n64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o
# CHERI128-vs-MIPS: target ABI 'sandbox' is incompatible with 'n64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o
# CHERI256-vs-CHERI128: target ISA 'cheri256' is incompatible with 'cheri128': {{.*}}/cheri-elf-flags-err.s.tmp-cheri128-lib.o
# CHERI128-vs-CHERI256: target ISA 'cheri128' is incompatible with 'cheri256': {{.*}}/cheri-elf-flags-err.s.tmp-cheri256-lib.o
# CHERI256-vs-CHERI256-HYBRID: target ABI 'sandbox' is incompatible with 'n64': {{.*}}/cheri-elf-flags-err.s.tmp-cheri256-hybrid-lib.o
# CHERI128-vs-CHERI128-HYBRID: target ABI 'sandbox' is incompatible with 'n64': {{.*}}/cheri-elf-flags-err.s.tmp-cheri128-hybrid-lib.o

# TODO: should those files actually link?
# CHERI256-HYBRID-vs-MIPS: target ISA 'cheri256' is incompatible with 'mips64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o
# CHERI128-HYBRID-vs-MIPS: target ISA 'cheri128' is incompatible with 'mips64': {{.*}}/cheri-elf-flags-err.s.tmp-mips64.o
