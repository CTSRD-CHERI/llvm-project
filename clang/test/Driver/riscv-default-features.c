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

/// The standards-compatible mode can be enable either using the arch string or the -mxcheri-std-compat flag
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-std-compat -S -emit-llvm %s -o - 2>&1 | FileCheck %s -check-prefix=RV64-XCHERI-STD-COMPAT
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri_xcheri-std-compat -S -emit-llvm %s -o - 2>&1 | FileCheck %s -check-prefix=RV64-XCHERI-STD-COMPAT
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri-std-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-STD-COMPAT
/// In the case of rv64i -mxcheri-std-compat, +xcheri is not inferred, but this will be fixed in future upstream merges
// RUN: %clang --target=riscv64-unknown-elf -march=rv64i -mxcheri-std-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI-STD-COMPAT-ONLY
/// Check that we can override the flag with -mno-xcheri-std-compat
// RUN: %clang --target=riscv64-unknown-elf -march=rv64ixcheri -mxcheri-std-compat -mno-xcheri-std-compat -S -emit-llvm %s -o - | FileCheck %s -check-prefix=RV64-XCHERI


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
// XCHERI-SAME: -xcheri-std-compat,
// RV64-XCHERI-EXPLICIT-RVC: "target-features"="+64bit,+relax,+xcheri

// XCHERI-NOT: xcheri

// RV64-XCHERI-STD-COMPAT: "target-features"="+64bit,+relax,+xcheri,+xcheri-std-compat,
// RV64-XCHERI-STD-COMPAT-ONLY: "target-features"="+64bit,+relax,+xcheri-std-compat,

// Dummy function
int foo(void){
  return  3;
}
