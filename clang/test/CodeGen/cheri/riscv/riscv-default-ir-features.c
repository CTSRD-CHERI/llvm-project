// RUN: %clang --target=riscv32 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+32bit,+a,+c,+m,+relax,-save-restore'
// RUN: %clang --target=riscv64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,-save-restore'

// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+32bit,+relax,+xcheri,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,-save-restore'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+32bit,+relax,+xcheri,-save-restore,-xcheri-norvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,-save-restore,-xcheri-norvc'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI '-DFEATURES=+32bit,+relax,+xcheri,+xcheri-norvc,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI '-DFEATURES=+64bit,+relax,+xcheri,+xcheri-norvc,-save-restore'

/// Same checks for purecap
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+32bit,+cap-mode,+relax,+xcheri,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,-save-restore'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+32bit,+cap-mode,+relax,+xcheri,-save-restore,-xcheri-norvc'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:    | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,-save-restore,-xcheri-norvc'
// RUN: %clang --target=riscv32 -march=rv32ixcheri -mabi=il32pc64 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:    | FileCheck %s --check-prefixes=CHECK,RV32-XCHERI-PURECAP '-DFEATURES=+32bit,+cap-mode,+relax,+xcheri,+xcheri-norvc,-save-restore'
// RUN: %clang --target=riscv64 -march=rv64ixcheri -mabi=l64pc128 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-XCHERI-PURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri,+xcheri-norvc,-save-restore'

/// Enabling -mxcheri-norvc with a non-CHERI triple shouldn't enable the CHERI datalayout
// RUN: %clang --target=riscv32 -mxcheri-norvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+32bit,+a,+c,+m,+relax,+xcheri-norvc,-save-restore'
// RUN: %clang --target=riscv64 -mxcheri-norvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,+xcheri-norvc,-save-restore'
// RUN: %clang --target=riscv32 -mno-xcheri-norvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+32bit,+a,+c,+m,+relax,-save-restore,-xcheri-norvc'
// RUN: %clang --target=riscv64 -mno-xcheri-norvc -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax,-save-restore,-xcheri-norvc'

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
