# REQUIRES: riscv
# RUN: %riscv32_cheri_purecap_llvm-mc -filetype=obj %s -o %t.32.o
# RUN: ld.lld %t.32.o -z separate-code -o %t.32
# RUN: llvm-readobj -r --cap-relocs -x .got.plt %t.32 | FileCheck --check-prefix=RELOC32 %s
# RUN: llvm-readelf -x .got.plt %t.32 | FileCheck --check-prefix=GOTPLT32 %s
# RUN: llvm-readelf -S -s %t.32 | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.32 | FileCheck --check-prefixes=DIS %s

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %s -o %t.64.o
# RUN: ld.lld %t.64.o -z separate-code -o %t.64
# RUN: llvm-readelf -S -s %t.64 | FileCheck --check-prefixes=SEC,NM %s
# RUN: llvm-readobj -r --cap-relocs -x .got.plt %t.64 | FileCheck --check-prefix=RELOC64 %s
# RUN: llvm-readelf -x .got.plt %t.64 | FileCheck --check-prefix=GOTPLT64 %s
# RUN: llvm-objdump -d --no-show-raw-insn %t.64 | FileCheck --check-prefixes=DIS %s

# SEC: .iplt PROGBITS {{0*}}00011010

## Canonical PLT in .iplt
# NM: {{0*}}00011010 16 FUNC GLOBAL DEFAULT {{.*}} func

# RELOC32:      Relocations [
# RELOC32-NEXT: ]
# RELOC32:      __cap_relocs {
# RELOC32-NEXT:   0x12000 IFUNC - 0x11000 [0x10400-0x12200]
# RELOC32-NEXT:   0x12200 FUNC - 0x11010 [0x10400-0x12200]
# RELOC32-NEXT: }
# GOTPLT32:      section '.got.plt':
# GOTPLT32-NEXT: 0x00012000 00000000 00000000

# RELOC64:      Relocations [
# RELOC64-NEXT: ]
# RELOC64:      __cap_relocs {
# RELOC64-NEXT:   0x12000 IFUNC - 0x11000 [0x10190-0x12010]
# RELOC64-NEXT:   0x12010 FUNC - 0x11010 [0x10190-0x12010]
# RELOC64-NEXT: }
# GOTPLT64:      section '.got.plt':
# GOTPLT64-NEXT: 0x00012000 00000000 00000000 00000000 00000000

# DIS:      <_start>:
## func - . = 0x11010-0x11004 = 12
# DIS-NEXT: 11004: auipcc ca0, 0
# DIS-NEXT:        cincoffset ca0, ca0, 12

# DIS:      Disassembly of section .iplt:
# DIS:      <func>:
## &.got.plt[func]-. = 0x12000-0x11010 = 4096*1-16
# DIS-NEXT: 11010: auipcc ct3, 1
# DIS-NEXT:        lc ct3, -16(ct3)
# DIS-NEXT:        jalr ct1, ct3
# DIS-NEXT:        nop

.text
.globl func
.type func, @gnu_indirect_function
func:
  ret
  .size func, . - func

.globl _start
.type _start, @function
_start:
  cllc ca0, func
  .size _start, . - _start

.data
.globl fptr
.type fptr, @object
fptr:
  .chericap func
  .size fptr, . - fptr
