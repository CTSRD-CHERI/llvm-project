// RUN: %clang -target riscv32-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32
// RUN: %clang -target riscv64-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-RVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mxcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-RVC
// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI-NORVC
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -mno-xcheri-rvc -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-NORVC

// RV32: "target-features"="+a,+c,+m,+relax,-save-restore"
// RV64: "target-features"="+64bit,+a,+c,+m,+relax,-save-restore"

// RV32-XCHERI: "target-features"="+relax,+xcheri,-save-restore"
// RV64-XCHERI: "target-features"="+64bit,+relax,+xcheri,-save-restore"

// RV32-XCHERI-RVC: "target-features"="+relax,+xcheri,+xcheri-rvc,-save-restore"
// RV64-XCHERI-RVC: "target-features"="+64bit,+relax,+xcheri,+xcheri-rvc,-save-restore"
// RV32-XCHERI-NORVC: "target-features"="+relax,+xcheri,-save-restore,-xcheri-rvc"
// RV64-XCHERI-NORVC: "target-features"="+64bit,+relax,+xcheri,-save-restore,-xcheri-rvc"

// Dummy function
int foo(){
  return  3;
}
