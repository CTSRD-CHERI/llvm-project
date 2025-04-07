# REQUIRES: riscv
# RUN: llvm-mc -triple=riscv64-unknown-freebsd -target-abi l64pc128 -mattr=+zcheripurecap,+cap-mode -position-independent -filetype=obj %s -o %t-rv64-purecap-main.o 
# RUN: llvm-mc -triple=riscv64-unknown-freebsd -target-abi lp64  -mattr=+zcherihybrid -position-independent -filetype=obj %s -o %t-rv64-hybrid-main.o 
# RUN: llvm-mc -triple=riscv32-unknown-freebsd -target-abi il32pc64 -mattr=+zcheripurecap,+cap-mode -position-independent -filetype=obj %s -o %t-rv32-purecap-main.o 
# RUN: llvm-mc -triple=riscv32-unknown-freebsd -target-abi ilp32  -mattr=+zcherihybrid -position-independent -filetype=obj %s -o %t-rv32-hybrid-main.o 

# RUN: llvm-readobj -h %t-rv64-purecap-main.o | FileCheck --check-prefix=RVCAP-FLAGS %s
# RUN: llvm-readobj -h %t-rv64-hybrid-main.o | FileCheck --check-prefix=RVHYB-FLAGS %s
# RUN: llvm-readobj -h %t-rv32-purecap-main.o | FileCheck --check-prefix=RVCAP-FLAGS %s
# RUN: llvm-readobj -h %t-rv32-hybrid-main.o | FileCheck --check-prefix=RVHYB-FLAGS %s

# RUN: ld.lld %t-rv64-hybrid-main.o -o /dev/null
# RUN: not ld.lld %t-rv64-hybrid-main.o -m elf64lriscv_cheri -o /dev/null 2>&1 | FileCheck %s --check-prefix=EMU_ERROR
# RUN: ld.lld %t-rv32-hybrid-main.o -o /dev/null
# RUN: not ld.lld %t-rv32-hybrid-main.o -m elf32lriscv_cheri -o /dev/null 2>&1 | FileCheck %s --check-prefix=EMU_ERROR
# EMU_ERROR: object file is non-CheriABI but emulation forces it


.global _start
.type   _start, @function
_start:
  nop
.size _start, . - _start

#RVCAP-FLAGS:       Machine: EM_RISCV (0xF3)
#RVCAP-FLAGS:         Flags [
#RVCAP-FLAGS-NEXT:      EF_RISCV_CAP_MODE  (0x20000)
#RVCAP-FLAGS-NEXT:      EF_RISCV_CHERIABI  (0x10000)
#RVCAP-FLAGS-NEXT:    ]

#RVHYB-FLAGS:       Machine: EM_RISCV (0xF3)
#RVHYB-FLAGS:         Flags [
#RVHYB-FLAGS-NEXT:    ]


