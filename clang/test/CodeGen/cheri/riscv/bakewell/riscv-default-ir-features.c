// RUN: %clang --target=riscv32 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-NOCHERI '-DFEATURES=+32bit,+a,+c,+m,+relax'
// RUN: %clang --target=riscv64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-NOCHERI '-DFEATURES=+64bit,+a,+c,+m,+relax'

// Hybrid/Legacy
// RUN: %clang --target=riscv32 -march=rv32izcherihybrid -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIHYBRID '-DFEATURES=+32bit,+relax,+zcherihybrid,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcherihybrid -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIHYBRID '-DFEATURES=+64bit,+relax,+zcherihybrid,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcherihybrid -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIHYBRID,NO-RVC '-DFEATURES=+32bit,+relax,+zcherihybrid,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcherihybrid -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIHYBRID,NO-RVC '-DFEATURES=+64bit,+relax,+zcherihybrid,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcherihybrid -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIHYBRID '-DFEATURES=+32bit,+relax,+xcheri-norvc,+zcherihybrid,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcherihybrid -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIHYBRID '-DFEATURES=+64bit,+relax,+xcheri-norvc,+zcherihybrid,+zcheripurecap'


// Purecap Mode
// RUN: %clang --target=riscv32 -march=rv32izcheripurecap -mabi=il32pc64 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPURECAP '-DFEATURES=+32bit,+cap-mode,+relax,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheripurecap -mabi=l64pc128 -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcheripurecap -mabi=il32pc64 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPURECAP,NO-RVC '-DFEATURES=+32bit,+cap-mode,+relax,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheripurecap -mabi=l64pc128 -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPURECAP,NO-RVC '-DFEATURES=+64bit,+cap-mode,+relax,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcheripurecap -mabi=il32pc64 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPURECAP '-DFEATURES=+32bit,+cap-mode,+relax,+xcheri-norvc,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheripurecap -mabi=l64pc128 -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPURECAP '-DFEATURES=+64bit,+cap-mode,+relax,+xcheri-norvc,+zcheripurecap'


// PTE Mode
// RUN: %clang --target=riscv32 -march=rv32izcheri-pte -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPTE '-DFEATURES=+32bit,+relax,+zcheri-pte,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheri-pte -S -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPTE '-DFEATURES=+64bit,+relax,+zcheri-pte,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcheri-pte -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPTE,NO-RVC '-DFEATURES=+32bit,+relax,+zcheri-pte,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheri-pte -S -mxcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPTE,NO-RVC '-DFEATURES=+64bit,+relax,+zcheri-pte,+zcheripurecap'
// RUN: %clang --target=riscv32 -march=rv32izcheri-pte -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV32-ZCHERIPTE '-DFEATURES=+32bit,+relax,+xcheri-norvc,+zcheri-pte,+zcheripurecap'
// RUN: %clang --target=riscv64 -march=rv64izcheri-pte -S -mno-xcheri-rvc -emit-llvm %s -o - \
// RUN:   | FileCheck %s --check-prefixes=CHECK,RV64-ZCHERIPTE '-DFEATURES=+64bit,+relax,+xcheri-norvc,+zcheri-pte,+zcheripurecap'

// RV32-NOCHERI: target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
// RV64-NOCHERI: target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
// RV32-ZCHERIHYBRID: target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128"
// RV64-ZCHERIHYBRID: target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n32:64-S128"
// RV32-ZCHERIPURECAP: target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
// RV64-ZCHERIPURECAP: target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n32:64-S128-A200-P200-G200"
// RV32-ZCHERIPTE: target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128"
// RV64-ZCHERIPTE: target datalayout = "e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n32:64-S128"

// CHECK: "target-features"="[[FEATURES]]
// CHECK-SAME: -save-restore
// NO-RVC-SAME: -xcheri-norvc

// RV32-NOCHERI: !{i32 1, !"target-abi", !"ilp32"}
// RV64-NOCHERI: !{i32 1, !"target-abi", !"lp64"}
// RV32-ZCHERIHYBRID: !{i32 1, !"target-abi", !"ilp32"}
// RV64-ZCHERIHYBRID: !{i32 1, !"target-abi", !"lp64"}
// RV32-ZCHERIPURECAP: !{i32 1, !"target-abi", !"il32pc64"}
// RV64-ZCHERIPURECAP: !{i32 1, !"target-abi", !"l64pc128"}
// RV32-ZCHERIPTE: !{i32 1, !"target-abi", !"ilp32"}
// RV64-ZCHERIPTE: !{i32 1, !"target-abi", !"lp64"}

int test(void){
    return 1;
}
