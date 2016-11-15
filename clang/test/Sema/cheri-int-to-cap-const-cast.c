// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi sandbox

// expected-no-diagnostics
void * __capability x=  (void * __capability)-1;
