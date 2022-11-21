// RUN: %clang -target riscv32-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32
// RUN: %clang -target riscv64-unknown-elf -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64

// RUN: %clang --target=riscv32-unknown-elf -march=rv32ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV32-XCHERI
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI

// RV32: "target-features"="+a,+c,+m,+relax,-save-restore"
// RV64: "target-features"="+64bit,+a,+c,+m,+relax,-save-restore"

// RV32-XCHERI: "target-features"="+relax,+xcheri,-save-restore,-xcheri-rvc
// RV64-XCHERI: "target-features"="+64bit,+relax,+xcheri,-save-restore,-xcheri-rvc"

// Dummy function
int foo(){
  return  3;
}
