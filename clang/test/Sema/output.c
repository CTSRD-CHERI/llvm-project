// RUN: %clang_cc1 -triple cheri-unknown-freebsd -o - %s -fsyntax-only -verify

int readFail(__output int * __capability x)
{
	*x = 12;
	(*x)++; // expected-error {{write-only variable is not readable}}
	return *x; // expected-error {{write-only variable is not readable}}
}

int caller(int * __capability x)
{
	return readFail(x);
}

