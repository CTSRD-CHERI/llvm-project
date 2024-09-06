# REQUIRES: riscv
# RUN: split-file %s %t

# RUN: %riscv64_cheri_purecap_llvm-mc -filetype=obj %t/a.s -o %t/a.o
# RUN: ld.lld --shared -T %t/lds --compartment-policy=%t/compartments.json %t/a.o -o %t/liba.so
# RUN: llvm-readelf -Ss %t/liba.so | FileCheck %s

## Verify that, when using a linker script, sections not in the default
## compartment get combined as if no linker script were present.

# CHECK: {{\[ *}}[[#TEXT_ONE_IDX:]]{{\]}} .text.one PROGBITS [[#%.16x,TEXT_ONE_ADDR:]]
# CHECK: {{\[ *}}[[#TEXT_TWO_IDX:]]{{\]}} .text.two PROGBITS [[#%.16x,TEXT_TWO_ADDR:]]

# CHECK: [[#TEXT_ONE_ADDR]]   0 FUNC GLOBAL DEFAULT [[#TEXT_ONE_IDX]] foo
# CHECK: [[#TEXT_ONE_ADDR+4]] 0 FUNC GLOBAL DEFAULT [[#TEXT_ONE_IDX]] bar
# CHECK: [[#TEXT_TWO_ADDR]]   0 FUNC GLOBAL DEFAULT [[#TEXT_TWO_IDX]] baz

#--- a.s

.section .text.foo, "ax"
.type foo, @function
.global foo
foo:
  ret

.section .text.bar, "ax"
.type bar, @function
.global bar
bar:
  ret

.section .text.baz, "ax"
.type baz, @function
.global baz
baz:
  ret

#--- lds

SECTIONS {}

#--- compartments.json

{
  "compartments": {
    "one": { "symbols": ["foo", "bar"] },
    "two": { "symbols": ["baz"] }
  }
}
