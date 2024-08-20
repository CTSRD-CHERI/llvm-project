;; Generate ELF attributes from llc.

; RUN: llc -mtriple=riscv32 -mattr=+xcheri %s -o - \
; RUN:   | FileCheck --check-prefix=RV32IXCHERI %s
; RUN: llc -mtriple=riscv64 -mattr=+xcheri %s -o - \
; RUN:   | FileCheck --check-prefix=RV64IXCHERI %s

; RV32IXCHERI: .attribute 5, "rv32i2p1_xcheri0p0"
; RV64IXCHERI: .attribute 5, "rv64i2p1_xcheri0p0"

define i32 @addi(i32 %a) {
  %1 = add i32 %a, 1
  ret i32 %1
}
