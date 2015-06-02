// RUN: %clang_cc1 "-target-abi" "sandbox" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// expected-no-diagnostics
typedef struct foo
{
		int b[42];
} foo_t;

