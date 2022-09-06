// RUN: %clang --target=riscv32-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32
// RUN: %clang --target=riscv64-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-NORVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NORVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-NORVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NORVC
// The -mxcheri-rvc flag to explicitly disable xcheri-norvc:
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-EXPLICIT-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-norvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-EXPLICIT-RVC

// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-v9-semantics -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-V9
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-v8-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-V8

// RV32: "target-features"="+32bit,+a,+c,+m,+relax,-save-restore"
// RV64: "target-features"="+64bit,+a,+c,+m,+relax,-save-restore"

// RV32-XCHERI: "target-features"="+32bit,+relax,+xcheri,-save-restore"
// RV64-XCHERI: "target-features"="+64bit,+relax,+xcheri,-save-restore"

// RV32-XCHERI-RVC: "target-features"="+32bit,+relax,+xcheri,-save-restore"
// RV64-XCHERI-RVC: "target-features"="+64bit,+relax,+xcheri,-save-restore"
// RV32-XCHERI-NORVC: "target-features"="+32bit,+relax,+xcheri,+xcheri-norvc,-save-restore"
// RV64-XCHERI-NORVC: "target-features"="+64bit,+relax,+xcheri,+xcheri-norvc,-save-restore"
// RV32-XCHERI-EXPLICIT-RVC: "target-features"="+32bit,+relax,+xcheri,-save-restore,-xcheri-norvc"
// RV64-XCHERI-EXPLICIT-RVC: "target-features"="+64bit,+relax,+xcheri,-save-restore,-xcheri-norvc"

// RV64-XCHERI-V8: "target-features"="+64bit,+relax,+xcheri,-save-restore,-xcheri-v9-semantics"
// RV64-XCHERI-V9: "target-features"="+64bit,+relax,+xcheri,+xcheri-v9-semantics,-save-restore"

// Dummy function
int foo(void){
  return  3;
}
