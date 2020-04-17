// Constructors should be in AS200:
// RUN: %cheri_purecap_cc1 %s -o - -emit-llvm -o - -O0 | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 %s -o - -emit-llvm -o - -O0 | FileCheck %s
// Previously, using a mips64- triple would crash the compiler:
// RUN: %clang_cc1 -triple mips64c128-unknown-freebsd13.0-purecap -target-abi purecap -o - -emit-llvm -O0 %s | FileCheck %s

__attribute__((__constructor__)) void a() {}

// CHECK: @llvm.global_ctors = appending addrspace(200) global [1 x { i32, void () addrspace(200)*, i8 addrspace(200)* }]
// CHECK-SAME: [{ i32, void () addrspace(200)*, i8 addrspace(200)* } { i32 65535, void () addrspace(200)* @a, i8 addrspace(200)* null }]
