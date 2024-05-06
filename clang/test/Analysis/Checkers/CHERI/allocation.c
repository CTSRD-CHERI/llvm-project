// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyzer-checker=core,unix,alpha.cheri.Allocation

typedef __typeof__(sizeof(int)) size_t;
extern void * malloc(size_t);


struct S1 {
  int *a[3];
  int *d[1];
};

struct S2 {
  int x[3];
  int *px;
};

struct S2 * test_1(int n1, int n2) {
  struct S1 *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  struct S2 *p2 = (struct S2 *)(p1+n1); // expected-warning{{Allocation partition}}
  return p2;
}

double buf[100] __attribute__((aligned(_Alignof(void*))));
struct S2 * test_2(int n1) {
  struct S1 *p1 = (struct S1 *)buf; // ?
  struct S2 *p2 = (struct S2 *)(p1+n1); // expected-warning{{Allocation partition}}
  return p2;
}

struct S2 * test_3(int n1, int n2) {
  struct S1 *p1 = malloc(sizeof(struct S1)*n1 + sizeof(struct S2)*n2);
  struct S2 *p2 = (struct S2 *)(p1+n1); // expected-warning{{Allocation partition}}
  return p2;
}

void array(int i, int j) {
  int a[100][200];
  int (*p1)[200] = &a[i];
  int *p2 = p1[j]; // no warn
  *p2 = 42;
}

struct S3 {
  struct S2 s2;
  int y;
};

struct S2 * first_field(void *p, int n1) {
  struct S3 *p3 = p;
  struct S2 *p2 = (struct S2 *)(p3+n1); // no warn
  return p2;
}

struct S4 {
  long len;
  int buf[];
};

int* flex_array(int len) {
  struct S4 *p = malloc(sizeof(struct S4) + len*sizeof(int));
  int *pB = (int*)(p + 1); // no warn
  return pB;
}
