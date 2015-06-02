// RUN: %clang_cc1 "-target-abi" "sandbox" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// expected-no-diagnostics
void
foo(void)
{
	int *b = (int[]){0,1,2};
	int c[2] = {1,2};
}
