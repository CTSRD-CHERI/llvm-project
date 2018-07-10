// RUN: %cheri_cc1 -o - -O0 %s
// RUN: %cheri_cc1 -o - -O2 %s
// RUN: %cheri_cc1 -o - -O3 %s
// Check that this doesn't crash the compiler at any optimisation level.
int foo(__capability void *a)
{
	if (a)
		return 42;
	return 45;
}

int bar(__capability void *a, __capability void *b)
{
	if (a && b)
		return 42;
	return 45;
}

__capability int *
foo2bar(__capability int *afoo)
{
	       if (!afoo)
			                  return((void * __capability)0);
		          return afoo;
}

