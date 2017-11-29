// RUN: %cheri_cc1 %s -fsyntax-only -verify  -target-abi purecap
// expected-no-diagnostics
#define NULL (void*)0

int foo(int i);

int
bar(int x)
{
        int (*func)(int);

        func = x == 0 ? foo : NULL;

        if (func != NULL)
                return (func(x));
        else
                return (-1);
}
