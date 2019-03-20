// RUN: %cheri_cc1 "-target-abi" "purecap" -fsyntax-only  %s -verify
// expected-no-diagnostics
void
foo(void)
{
	int *b = (int[]){0,1,2};
	int c[2] = {1,2};
}

int
a(void *p);

int
b(void)
{
	return (a(&(int){1}));
}

