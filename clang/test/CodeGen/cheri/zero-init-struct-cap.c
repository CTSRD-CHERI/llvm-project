// This was crashing when I first added support for NULL-derived globals
// Keep it as a regression test
// RUN: %cheri_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %riscv64_cheri_cc1 -emit-llvm -o - %s | FileCheck %s
// RUN: %riscv64_cheri_purecap_cc1 -emit-llvm -o - %s | FileCheck %s
__uintcap_t a;
struct {
  __uintcap_t b;
} c = {};

char empty[2] = {};

// CHECK: %struct.anon = type { i8 addrspace(200)* }
// CHECK: @c = {{(addrspace\(200\) )?}}global %struct.anon zeroinitializer, align 16
// CHECK: @empty = {{(addrspace\(200\) )?}}global [2 x i8] zeroinitializer, align 1
// CHECK: @a = {{(addrspace\(200\) )?}}global i8 addrspace(200)* null, align 16
