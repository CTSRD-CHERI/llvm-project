// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,unix,alpha.cheri.Allocation,cheri.CheriAPIModelling

typedef __typeof__(sizeof(int)) size_t;
extern void * malloc(size_t);
void foo(void*);

struct S1 {
  int *a[3];
  int *d[3];
};

struct S2 {
  int x[3];
  int *px;
};

struct S2 * test_1(int n1, int n2) {
  struct S1 *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  struct S2 *p2 = (struct S2 *)(p1+n1);
  return p2; // expected-warning{{Pointer to suballocation returned from function}}
}

double buf[100] __attribute__((aligned(_Alignof(void*)))); // expected-note{{Original allocation}}
void test_2(int n1) {
  struct S1 *p1 = (struct S1 *)buf; // ?
  struct S2 *p2 = (struct S2 *)(p1+n1);
  foo(p2); // expected-warning{{Pointer to suballocation passed to function}}
}

void test_3(int n1, int n2) {
  struct S1 *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  struct S2 *p2 = (struct S2 *)(p1+n1);
  foo(p2); // expected-warning{{Pointer to suballocation passed to function}}
}

void array(int i, int j) {
  int a[100][200];
  int (*p1)[200] = &a[i];
  int *p2 = p1[j];
  foo(p2); // no warn
}

struct S3 {
  struct S2 s2;
  int y;
};

struct S2 * first_field(void *p, int n1) {
  struct S3 *p3 = p;
  struct S2 *p2 = (struct S2 *)(p3+n1);
  return p2;  // no warn
}

struct S4 {
  long len;
  int buf[];
};

int* flex_array(int len) {
  struct S4 *p = malloc(sizeof(struct S4) + len*sizeof(int));
  int *pB = (int*)(p + 1);
  return pB; // no warn
}

void test_4(struct S2 *pS2) {
  double a[100];  // expected-note{{Original allocation}}
  double *p1 = a;
  pS2->px = (int*)(p1 + 10); // expected-warning{{Pointer to suballocation escaped on assign}}
}

void test_5(int n1, int n2) {
  int *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  unsigned *p2 = (unsigned*)(p1+n1);
  foo(p2); // no warn
}

void test_6(int n1, int n2) {
  struct S1 **p1 = malloc(sizeof(struct S1*)*n1 + sizeof(struct S2*)*n2);
  struct S2 **p2 = (struct S2 **)(p1+n1);
  foo(p2); // no warn
}

void *cheri_bounds_set(void *c, size_t x);
void test_7(int n1, int n2) {
  struct S1 *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  struct S2 *p2 = (struct S2 *)(p1+n1);
  struct S2 *p3 = cheri_bounds_set(p2, sizeof(struct S2)*n2);
  foo(p3); // no-warn
}
