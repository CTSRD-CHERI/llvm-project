// RUN: %clang --target=riscv32-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32
// RUN: %clang --target=riscv64-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV32-XCHERI,XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV64-XCHERI,XCHERI
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV32-XCHERI-NORVC,XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV64-XCHERI-NORVC,XCHERI
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV32-XCHERI-NORVC,XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV64-XCHERI-NORVC,XCHERI
// The -mxcheri-rvc flag to explicitly disable xcheri-norvc:
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV32-XCHERI,XCHERI,XCHERI-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV64-XCHERI,XCHERI,XCHERI-RVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV32-XCHERI,XCHERI,XCHERI-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s --check-prefixes=RV64-XCHERI,XCHERI,XCHERI-RVC

// RV32: "target-features"="+32bit,+a,+c,+m,+relax,
// RV32-SAME: -save-restore
// RV64: "target-features"="+64bit,+a,+c,+m,+relax,
// RV64-SAME: -save-restore

// RV32-XCHERI: "target-features"="+32bit,+relax,+xcheri,
// RV32-XCHERI-SAME: -save-restore
// RV64-XCHERI: "target-features"="+64bit,+relax,+xcheri,
// RV64-XCHERI-SAME: -save-restore

// RV32-XCHERI-RVC: "target-features"="+32bit,+relax,+xcheri
// RV64-XCHERI-RVC: "target-features"="+64bit,+relax,+xcheri
// RV32-XCHERI-NORVC: "target-features"="+32bit,+relax,+xcheri,+xcheri-norvc
// RV32-XCHERI-NORVC: -save-restore
// RV64-XCHERI-NORVC: "target-features"="+64bit,+relax,+xcheri,+xcheri-norvc
// RV64-XCHERI-NORVC: -save-restore
// RV32-XCHERI-EXPLICIT-RVC: "target-features"="+32bit,+relax,+xcheri
// RV32-XCHERI-EXPLICIT-RVC-SAME -save-restore
// XCHERI-RVC-SAME: ,-xcheri-norvc,
// RV64-XCHERI-EXPLICIT-RVC: "target-features"="+64bit,+relax,+xcheri

// XCHERI-NOT: xcheri

// Dummy function
int foo(void){
  return  3;
}
