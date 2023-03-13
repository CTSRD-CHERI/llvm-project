// RUN: %cheri_purecap_cc1 -analyze -analyzer-checker=core,alpha.cheri.CapabilityCopyChecker -verify %s

// RUN: %cheri_purecap_cc1 -analyze -analyzer-checker=core,alpha.cheri.ProvenanceSourceChecker,alpha.cheri.CapabilityCopyChecker -verify %s

typedef __intcap_t intptr_t;
typedef __uintcap_t uintptr_t;

#define BLOCK_TYPE uintptr_t
#define BLOCK_SIZE sizeof(BLOCK_TYPE)

typedef __typeof__(sizeof(int)) size_t;
extern void * malloc(size_t);
extern void *memmove(void *dest, const void *src, size_t n);
extern void free(void *ptr);

static int x;

// =====     Tag-stripping copy     =====
void copy_intptr_byte(int **ppy) {
  int *px = &x;
  int **ppx = &px;
  void *q = ppx;
  char *s = (char*)q; 
  *(char*)ppy = *s; // expected-warning{{Tag-stripping store of a capability}}
}

void copy_intptr_byte2(int *px, int **ppy) {
  int **ppx = &px;
  void *q = ppx;
  char *s = (char*)q; 
  *(char*)ppy = *s; // expected-warning{{Tag-stripping store of a capability}}
}

void copy_int_byte(int *py) {
  int *px = &x;
  void *q = px;
  char *s = (char*)q;
  *(char*)py = *s;  // no-warning: copying int value
}

void copy_intptr(int **ppy) {
  int *px = &x;
  int **ppx = &px;
  void *q = ppx;
  int **v = (int**)q;
  *ppy = *v; // no-warning: copy as int*
}

static void swapfunc(void *a, void *b, int n) {
  long i = n;
  char *pi = (char *)(a);
  char *pj = (char *)(b);
  do {
    char t = *pi;
    *pi++ = *pj; // expected-warning{{Tag-stripping store of a capability}}
    *pj++ = t; // expected-warning{{Tag-stripping store of a capability}}
  } while (--i > 0);
}

void *realloc_impl(void *ptr, size_t size) {
  void *dst = malloc(size);
  if (size <= sizeof(size)) {
    size_t *mcsrc = (size_t *)(ptr);
    size_t *mcdst = (size_t *)(dst);
    *mcdst = *mcsrc; // expected-warning{{Tag-stripping store of a capability}}
  } else
    memmove(dst, ptr, size);
  free(ptr);
  return dst;
}

void memcpy_impl_good(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  if ((len < sizeof(BLOCK_TYPE)) || ((long)src & (BLOCK_SIZE - 1)) || ((long)dst & (BLOCK_SIZE - 1)))
    while (len--)
      *dst++ = *src++; // no-warning
}

void memcpy_impl_bad(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  if (len < sizeof(BLOCK_TYPE)+1)
    while (len--)
      *dst++ = *src++; // expected-warning{{Tag-stripping store of a capability}}
}

char voidptr_arg_load1(void *q) {
  char *s = (char*)q; 
  return *s; 
}

void voidptr_arg_store1(void *q) {
  char *s = (char*)q; 
  *s = 42;
}

char fp_malloc() {
  void *q = malloc(100);
  char *s = (char*)q; // no warning
  return *s;
}

char fp_init_str(void) {
  char s[] = "String literal"; // no warning
  return s[3];
}

// =====     Part of capability representation used as argument in binary operator     =====

int hash_no_call(void *key0, size_t len) {
  char *k = key0;
  int h = 0;
  while (len--)
    h = (h << 5) + *k++;
  return h;
}

int hash(void *key0, size_t len) {
  char *k = key0;
  int h = 0;
  while (len--)
    h = (h << 5) + *k++;  // expected-warning{{Part of capability representation used as argument in binary operator}}
  return h; 
}

int ptr_hash(void) {
  int *p = &x;
  return hash(&p, sizeof(int*));
}

int memcmp_impl_no_call(const void* m1, const void *m2, size_t len) {
  const char *s1 = m1;
  const char *s2 = m2;

  while (len--)
    if (*s1 != *s2)  
      return *s1 - *s2;  
  return 0;
}

int memcmp_impl(const void* m1, const void *m2, size_t len) {
  const char *s1 = m1;
  const char *s2 = m2;

    while (len--)
      if (*s1 != *s2) // expected-warning{{Part of capability representation used as argument in binary operator}} 
          return *s1 - *s2; // expected-warning{{Part of capability representation used as argument in binary operator}}  
    return 0;
}

int ptr_cmp(int *x, int *y) {
    return memcmp_impl(&x, &y, sizeof(int*));
}

