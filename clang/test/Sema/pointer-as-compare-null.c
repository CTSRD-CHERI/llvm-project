// RUN: %clang_cc1 %s -fsyntax-only -verify -triple cheri-unknown-freebsd
// expected-no-diagnostics
#define NULL (void*)0
int func(char * __capability ptr)
{
        if (ptr == NULL)
                return (0);
        return (1);
}

