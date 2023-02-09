// RUN: %clang --target=riscv32 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+a,+c,+m,+relax,-save-restore'
// RUN: %clang --target=riscv64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,-save-restore'

// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+relax,+xcheri,+xcheri-rvc,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,+xcheri-rvc,-save-restore'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,-save-restore,-xcheri-rvc'

/// Same checks for purecap
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+cap-mode,+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+cap-mode,+relax,+xcheri,+xcheri-rvc,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:    | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,+xcheri-rvc,-save-restore'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:    | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+cap-mode,+relax,+xcheri,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,-save-restore,-xcheri-rvc'

/// FIXME: Enabling -mxcheri-rvc with a non-CHERI triple shouldn't enable the CHERI datalayout
// RUN: not %clang --target=riscv32 -mxcheri-rvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+a,+c,+m,+relax,+xcheri-rvc,-save-restore'
// FIXME: %clang --target=riscv32 -mxcheri-rvc -S -emit-llvm %s -o - \
// FIXME:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+a,+c,+m,+relax,+xcheri-rvc,-save-restore'
// RUN: not %clang --target=riscv64 -mxcheri-rvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,+xcheri-rvc,-save-restore'
// FIXME: %clang --target=riscv64 -mxcheri-rvc -S -emit-llvm %s -o - \
// FIXME:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,+xcheri-rvc,-save-restore'
// RUN: %clang --target=riscv32 -mno-xcheri-rvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+a,+c,+m,+relax,-save-restore,-xcheri-rvc'
// RUN: %clang --target=riscv64 -mno-xcheri-rvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,-save-restore,-xcheri-rvc'

// RV32-NOCHERI: target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
// RV64-NOCHERI: target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n64-S128"
// RV32-XCHERI: target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128"
// RV64-XCHERI: target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128"
// RV32-XCHERI-PURECAP: target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
// RV64-XCHERI-PURECAP: target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128-A200-P200-G200"

// CHECK: "target-features"="[[FEATURES]]"

// RV32-NOCHERI: !{i32 1, !"target-abi", !"ilp32"}
// RV64-NOCHERI: !{i32 1, !"target-abi", !"lp64"}
// RV32-XCHERI: !{i32 1, !"target-abi", !"ilp32"}
// RV64-XCHERI: !{i32 1, !"target-abi", !"lp64"}
// RV32-XCHERI-PURECAP: !{i32 1, !"target-abi", !"il32pc64"}
// RV64-XCHERI-PURECAP: !{i32 1, !"target-abi", !"l64pc128"}

int test(void) {
  return 1;
}
