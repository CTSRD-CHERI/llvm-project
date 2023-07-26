// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,alpha.cheri.CapabilityAlignmentChecker

typedef __uintcap_t uintptr_t;

double a[2048], *next = a;

#define	roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))

#define NULL (void*)0
uintptr_t *u;

void foo(void *v) {
  char *p0 = (char*)a;
  *(void**)p0 = v; // expected-warning{{Cast increases required alignment: 8 -> 16}}
  char *p1 = (char*)roundup2((uintptr_t)p0, sizeof(void*));
  *(void**)p1 = v; // no warn
  char *p2 = p1 + 5*sizeof(double);
  *(void**)p2 = v; // expected-warning{{Cast increases required alignment: 8 -> 16}}

  if (u == NULL)
    return; // no warning
}

