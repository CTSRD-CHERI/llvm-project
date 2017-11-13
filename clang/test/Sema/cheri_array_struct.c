// RUN: %cheri_cc1 "-target-abi" "purecap" -fsyntax-only  %s -verify
// expected-no-diagnostics
typedef struct foo
{
		int b[42];
} foo_t;

