// RUN: %cheri_cc1 -fsyntax-only  %s -verify

struct astruct {
        int     member1;
        int     member2;
};


int * __capability
func(struct astruct * __capability asp)
{
        return &(asp->member2); // expected-no-diagnostics
}
