// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,alpha.cheri.CapabilityAlignmentChecker

typedef __uintcap_t uintptr_t;
typedef __intcap_t intptr_t;
typedef __typeof__(sizeof(int)) size_t;

double a[2048], *next = a;

#define	roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))

#define NULL (void*)0
#define MINUS_ONE (void*)-1
uintptr_t *u;

void foo(void *v, int *pi, void *pv) {
  char *p0 = (char*)a;
  *(void**)p0 = v; // expected-warning{{Cast increases required alignment: 8 -> 16}}
  char *p1 = (char*)roundup2((uintptr_t)p0, sizeof(void*));
  *(void**)p1 = v; // no warning
  char *p2 = p1 + 5*sizeof(double);
  *(void**)p2 = v; // expected-warning{{Cast increases required alignment: 8 -> 16}}

  *(void**)pi = v; // expected-warning{{Cast increases required alignment: 4 -> 16}}
  *(void**)pv = v; // no warning
  *(void**)next = v; // expected-warning{{Cast increases required alignment: 8 -> 16}}

  if (u == NULL || u == MINUS_ONE) // no warning
    return;
}

#define align_offset(A) \
  ((((uintptr_t)(A)&7) == 0) \
       ? 0              \
       : ((8 - ((uintptr_t)(A)&7)) & 7))

uintptr_t* bar(uintptr_t *p) {
  uintptr_t offset = align_offset((char*)(p) + 2 *sizeof(size_t));
  p = (uintptr_t*)((char*)p + offset); // no warning
  return p;
}

struct S {
  intptr_t u[10];
  int i[10];
  int i_aligned[10] __attribute__((aligned(16)));
};
uintptr_t* struct_field(struct S *s) {
  uintptr_t* p1 = (uintptr_t*)&s->u[3];  // no warning
  uintptr_t* p2 = (uintptr_t*)&s->i[6];  // expected-warning{{Cast increases required alignment: 8 -> 16}}
  uintptr_t* p3 = (uintptr_t*)&s->i_aligned[6];  // FIXME: expected-warning{{Cast increases required alignment: 8 -> 16}}
  return p3 + (p2 - p1);
}

void local_var(void) {
  char buf[4];
  char buf_underaligned[4] __attribute__((aligned(2)));
  char buf_aligned[4] __attribute__((aligned(4)));
  *(int*)buf = 42; // expected-warning{{Cast increases required alignment: 1 -> 4}}
  *(int*)buf_underaligned = 42; // expected-warning{{Cast increases required alignment: 2 -> 4}}
  *(int*)buf_aligned = 42; // no warning
}

char st_buf[4];
char st_buf_aligned[4] __attribute__((aligned(_Alignof(int*))));
void static_var(void) {
  *(int*)st_buf = 42; // expected-warning{{Cast increases required alignment: 1 -> 4}}
  *(int*)st_buf_aligned = 42; // no warning
}
