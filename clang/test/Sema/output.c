// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify

int readFail(__capability __output int *x)
{
	*x = 12;
	(*x)++; // expected-error {{write-only variable is not readable}}
	return *x; // expected-error {{write-only variable is not readable}}
}

int caller(__capability int *x)
{
	return readFail(x);
}

