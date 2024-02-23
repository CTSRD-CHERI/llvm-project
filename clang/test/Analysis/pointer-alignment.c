// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,optin.portability.PointerAlignment

typedef __uintcap_t uintptr_t;
typedef __intcap_t intptr_t;
typedef __typeof__(sizeof(int)) size_t;
extern void * malloc(size_t);

double a[2048], // expected-note{{Original allocation of type 'double[2048]'}}
    *next = a;

#define	roundup2(x, y)	(((x)+((y)-1))&(~((y)-1)))

#define NULL (void*)0
#define MINUS_ONE (void*)-1
uintptr_t *u;

void foo(void *v, int *pi, void *pv) {
  char *p0 = (char*)a;
  *(void**)p0 = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with 16-byte capability alignment}}
  char *p1 = (char*)roundup2((uintptr_t)p0, sizeof(void*));
  *(void**)p1 = v; // no warning
  char *p2 = p1 + 5*sizeof(double);
  *(void**)p2 = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with 16-byte capability alignment}}

  *(void**)pi = v; // expected-warning{{Pointer value aligned to a 4 byte boundary cast to type 'void * __capability * __capability' with 16-byte capability alignment}}
  *(void**)pv = v; // no warning
  *(void**)next = v; // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'void * __capability * __capability' with 16-byte capability alignment}}

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
  uintptr_t* p2 = (uintptr_t*)&s->i[8];  // expected-warning{{Pointer value aligned to a 4 byte boundary cast to type 'uintptr_t * __capability' with 16-byte capability alignment}}
  uintptr_t* p3 = (uintptr_t*)&s->i_aligned[6];  // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'uintptr_t * __capability' with 16-byte capability alignment}}
  uintptr_t* p4 = (uintptr_t*)&s->i_aligned[4];  // no warning
  return (p4 - p3) + (p2 - p1);
}

void local_var(void) {
  char buf[4]; // expected-note{{Original allocation}}
  char buf_underaligned[4] __attribute__((aligned(2))); // expected-note{{Original allocation}}
  char buf_aligned[4] __attribute__((aligned(4)));
  *(int*)buf = 42; // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with 4-byte alignment}}
  *(int*)buf_underaligned = 42; // expected-warning{{Pointer value aligned to a 2 byte boundary cast to type 'int * __capability' with 4-byte alignment}}
  *(int*)buf_aligned = 42; // no warning
}

char st_buf[4]; // expected-note{{Original allocation}}
char st_buf_aligned[4] __attribute__((aligned(_Alignof(int*))));
void static_var(void) {
  *(int*)st_buf = 42; // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with 4-byte alignment}}
  *(int*)st_buf_aligned = 42; // no warning
}

int voidptr_cast(int *ip1, int *ip2) {
  intptr_t w = (intptr_t)(ip2) | 1;
  int b1 = (ip1 == (int*)w);  // expected-warning{{Pointer value aligned to a 1 byte boundary cast to type 'int * __capability' with 4-byte alignment}}
  int b2 = (ip1 == (void*)w); // no-warn
  return b1 || b2;
}

intptr_t param(int *pI, char* pC) {
  intptr_t *ipI = (intptr_t*)pI; // expected-warning{{Pointer value aligned to a 4 byte boundary cast to type 'intptr_t * __capability' with 16-byte capability alignment}}
  intptr_t *ipC = (intptr_t*)pC; // no warn
  return ipI - ipC;
}

typedef struct B {
  long *ptr;
  long flex[1]; // expected-note{{Original allocation}}
} B;

B* blob(size_t n) {
  size_t s = sizeof(B) + n * sizeof(long) + n * sizeof(B);
  B *p = malloc(s);
  p->ptr = (long*)&p[1];
  return (B*)(&p->ptr[n]); // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'B * __capability' with 16-byte capability alignment}}
}
B* flex(size_t n) {
  size_t s = sizeof(B) + (n-1) * sizeof(long) + n * sizeof(B);
  B *p = malloc(s);
  return (B*)(&p->flex[n]); // expected-warning{{Pointer value aligned to a 8 byte boundary cast to type 'B * __capability' with 16-byte capability alignment}}
}

char c_buf[100]; // expected-note{{Original allocation}} expected-note{{Original allocation}} expected-note{{Original allocation}}
void implicit_cap_storage(void **impl_cap_ptr) {
  *impl_cap_ptr = &c_buf[0];
  // expected-warning@-1{{Pointer value aligned to a 1 byte boundary stored as value of type 'void * __capability'. Memory pointed by it may be used to hold capabilities, for which 16-byte capability alignment will be required}}
}

char c_buf_aligned[100] __attribute__((aligned(_Alignof(void*)))); // expected-note{{Capabilities stored in 'char[100]'}}
extern void *memcpy(void *dest, const void *src, size_t n);
void copy_through_unaligned(intptr_t *src, void *dst, size_t n) {
  void *s = src, *d = dst;
  memcpy(c_buf_aligned, s, n * sizeof(intptr_t)); // no warn
  memcpy(c_buf, c_buf_aligned, n * sizeof(intptr_t));
  // expected-warning@-1{{Copied memory object of type 'char[100]' contains capabilities that require 16-byte capability alignment. Destination address alignment is 1. Storing a capability at an underaligned address leads to tag stripping}}
  memcpy(d, c_buf, n * sizeof(intptr_t));
  // expected-warning@-1{{Destination memory object pointed by 'void * __capability' pointer may be supposed to contain capabilities that require 16-byte capability alignment. Source address alignment is 1, which means that copied object may have its capabilities tags stripped earlier due to underaligned storage}}
}

void after_cap_data(int n, int D) {
  int **p = malloc(100);
  p[0] = &((int*)&p[D])[0];  // 2D matrix
  memcpy(c_buf, p[0], n); // no warn
}

// ----
char a1[100], a2[100], a3[100], a4[100], a5[100], a6[100]; // expected-note{{Original allocation}} expected-note{{}} expected-note{{}} expected-note{{}} expected-note{{}}

struct T {
  void *pVoid;
  intptr_t *pCap;
} *gS;

void copy(void *dst, void* src, size_t n) {
  memcpy(dst, src, n); // no-warn
}

void gen_storage(struct T *pT, void *p, size_t n) {
  memcpy(a1, p, n);
  //expected-warning@-1{{Copied memory object pointed by 'void * __capability' pointer may contain capabilities that require 16-byte capability alignment. Destination address alignment is 1. Storing a capability at an underaligned address leads to tag stripping}}
  memcpy(pT->pVoid, a2, n);
  //expected-warning@-1{{Destination memory object pointed by 'void * __capability' pointer may be supposed to contain capabilities that require 16-byte capability alignment. Source address alignment is 1, which means that copied object may have its capabilities tags stripped earlier due to underaligned storage}}

  struct T *mT = malloc(sizeof(struct T));
  memcpy(a3, mT->pVoid, n);
  //expected-warning@-1{{Copied memory object pointed by 'void * __capability' pointer may contain capabilities that require 16-byte capability alignment. Destination address alignment is 1. Storing a capability at an underaligned address leads to tag stripping}}

  memcpy(mT->pCap, a4, n);
  //expected-warning@-1{{Destination memory object pointed by 'intptr_t * __capability' pointer is supposed to contain capabilities that require 16-byte capability alignment. Source address alignment is 1, which means that copied object may have its capabilities tags stripped earlier due to underaligned storage}}

  memcpy(gS->pCap, a5, n);
  //expected-warning@-1{{Destination memory object pointed by 'intptr_t * __capability' pointer is supposed to contain capabilities that require 16-byte capability alignment. Source address alignment is 1, which means that copied object may have its capabilities tags stripped earlier due to underaligned storage}}

  void *m = malloc(n);
  memcpy(a6, m, n); // no-warn
  copy(a6, m, n);


  int x;
  memcpy(&x, p, sizeof(int)); // no warn
}

// ----
char extra[100]; // expected-note{{Original allocation of type 'char[100]' has an alignment requirement 1 byte}}
void *gP;

void alloc(void** p) {
  *p = extra;
  //expected-warning@-1{{Pointer value aligned to a 1 byte boundary stored as value of type 'intptr_t * __capability'. Memory pointed by it is supposed to hold capabilities, for which 16-byte capability alignment will be required}}
}

void test_assign(struct T *pT) {
  intptr_t *cp; // expected-note{{Memory pointed by '__intcap * __capability' value is supposed to hold capabilities}}
  alloc((void**)&cp);
  gP = cp; // no duplicate warning

  pT->pVoid = (void*)"string"; // no warning
}


#define offsetof(T,F) __builtin_offsetof(T, F)

struct S2 {
  char c[10];
  short sh;
  int x;
};
int test_offsetof(char *pC, short *pSh, struct S2 *pS2) {
  struct S2 *q = (struct S2*)((char*)pSh - offsetof(struct S2, sh)); // no-warn
  short* pSh2 = &pS2->sh;
  struct S2 *q2 = (struct S2*)((char*)pSh2 - offsetof(struct S2, sh));
  struct S2 *q3 = (struct S2*)((char*)pC - offsetof(struct S2, c)); // no-warn
  return q->x + q2->x + q3->x;
}
