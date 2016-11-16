// RUN: %clang_cc1 -triple cheri-unknown-freebsd -fsyntax-only -verify %s -target-abi sandbox

// expected-no-diagnostics
__capability void *x=  (__capability void*)-1;
