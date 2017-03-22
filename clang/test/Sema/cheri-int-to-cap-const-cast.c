// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi purecap

// expected-no-diagnostics
void * __capability x =  (void * __capability)0;
void * __capability y =  (void * __capability)-1;

// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only %s -target-abi purecap -emit-llvm -o - | FileCheck %s
// This is in Sema, but let's also test the codegen here:
// CHECK:  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* @x, align 32

// CHECK:      %0 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* null, i64 -1)
// CHECK-NEXT: store i8 addrspace(200)* %0, i8 addrspace(200)* addrspace(200)* @y, align 32
