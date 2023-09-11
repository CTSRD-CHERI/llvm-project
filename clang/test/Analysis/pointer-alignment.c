// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,optin.portability.PointerAlignment

typedef __uintcap_t uintptr_t;
typedef __intcap_t intptr_t;
typedef __typeof__(sizeof(int)) size_t;

double a[2048], // expected-note{{Original allocation of type 'double[2048]'}}
    *next = a;

#define	roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))

#define NULL (void*)0
#define MINUS_ONE (void*)-1
uintptr_t *u;

void foo(void *v, int *pi, void *pv) {
  char *p0 = (char*)a;
  *(void**)p0 = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with required capability alignment 16 bytes}}
  char *p1 = (char*)roundup2((uintptr_t)p0, sizeof(void*));
  *(void**)p1 = v; // no warning
  char *p2 = p1 + 5*sizeof(double);
  *(void**)p2 = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with required capability alignment 16 bytes}}

  *(void**)pi = v; // expected-warning{{Pointer value aligned to a 4 byte boundary cast to type 'void * __capability * __capability' with required capability alignment 16 bytes}}
  *(void**)pv = v; // no warning
  *(void**)next = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with required capability alignment 16 bytes}}

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
  intptr_t u[40];
  int i[40]; // expected-note{{Original allocation}}
  int i_aligned[40] __attribute__((aligned(16))); // expected-note{{Original allocation}}
};
int struct_field(struct S *s) {
  uintptr_t* p1 = (uintptr_t*)&s->u[3];  // no warning
  uintptr_t* p2 = (uintptr_t*)&s->i[8];  // expected-warning{{Pointer value aligned to a 4 byte boundary cast to type 'uintptr_t * __capability' with required capability alignment 16 bytes}}
  uintptr_t* p3 = (uintptr_t*)&s->i_aligned[6];  // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'uintptr_t * __capability' with required capability alignment 16 bytes}}
  uintptr_t* p4 = (uintptr_t*)&s->i_aligned[4];  // no warning
  return (p4 - p3) + (p2 - p1);
}

void local_var(void) {
  char buf[4]; // expected-note{{Original allocation}}
  char buf_underaligned[4] __attribute__((aligned(2))); // expected-note{{Original allocation}}
  char buf_aligned[4] __attribute__((aligned(4)));
  *(int*)buf = 42; // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with required alignment 4 bytes}}
  *(int*)buf_underaligned = 42; // expected-warning{{Pointer value aligned to a 2 byte boundary cast to type 'int * __capability' with required alignment 4 bytes}}
  *(int*)buf_aligned = 42; // no warning
}

char st_buf[4]; // expected-note{{Original allocation}}
char st_buf_aligned[4] __attribute__((aligned(_Alignof(int*))));
void static_var(void) {
  *(int*)st_buf = 42; // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with required alignment 4 bytes}}
  *(int*)st_buf_aligned = 42; // no warning
}

int voidptr_cast(int *ip1, int *ip2) {
  intptr_t w = (intptr_t)(ip2) | 1;
  int b1 = (ip1 == (int*)w);  // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with required alignment 4 bytes}}
  int b2 = (ip1 == (void*)w); // no-warn
  return b1 || b2;
}


