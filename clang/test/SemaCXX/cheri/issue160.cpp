// RUN: %cheri_cc1 -target-abi n64 -fsyntax-only -std=c++11 -o - -emit-llvm %s | FileCheck %s

// See Sema/cheri/issue160.c
// In C++, functions that differ only by __capability are allowed because
// __capability-qualified pointers and references are mangled differently.

struct a;

void cap(struct a *) {} // CHECK: _Z3capP1a
void cap(struct a * __capability c) {} // CHECK: _Z3capU3capP1a

void ref(struct a &) {} // CHECK: _Z3refR1a
void ref(struct a & __capability) {} // CHECK: _Z3refU3capR1a

void rref(struct a &&) {} // CHECK: _Z4rrefO1a
void rref(struct a && __capability) {} // CHECK: _Z4rrefU3capO1a
