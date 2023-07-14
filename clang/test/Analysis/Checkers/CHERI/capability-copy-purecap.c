// RUN: %cheri_purecap_cc1 -analyze -analyzer-checker=core,alpha.cheri.CapabilityCopyChecker -verify %s

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

// =====   Pointer to capability passed as void*   ====

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

void memcpy_impl_good(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  if ((len < sizeof(BLOCK_TYPE)))
    while (len--)
      *dst++ = *src++; // no-warn
}

void memcpy_impl_unaligned(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  if ((len < sizeof(BLOCK_TYPE)) || ((long)src & (BLOCK_SIZE - 1)) || ((long)dst & (BLOCK_SIZE - 1)))
    while (len--)
      *dst++ = *src++; // expected-warning{{Tag-stripping store of a capability}}
}

void memcpy_impl_bad(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  if (len < sizeof(BLOCK_TYPE)+1)
    while (len--)
      *dst++ = *src++; // expected-warning{{Tag-stripping store of a capability}}
}

struct S {
  int x;
  int fill[100];
  int *p;
};

void struct_field(void *p) {
  struct S *ps = p;
  ps->p = malloc(10*sizeof(int));
  int x = ps->x;
  *ps->p = x; // no warning
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

void copyAsLong(void* src0, void *dst0, size_t len) {
  if (len == 16) {
    long *src = src0;
    long *dst = dst0;
    *dst++ = *src++;
    *dst = *src; // expected-warning{{Tag-stripping store of a capability}}
  }
}

// =====   Pointer to capability passed as char*   ====

void char_ptr(char* src, char *dst, size_t len) {
  while (--len)
    *dst++ = *src++; // expected-warning{{Tag-stripping store of a capability}}
}

#define EOL 10
void c_string(char* src1, char* src2, char* src3, char *src4, char *dst) {
  int i = 0;
  while (src1[i])
    *dst++ = src1[i++]; // no warning

  src2++;
  while (*src2 != '.')
    *dst++ = *src2++; // no warning

  char c;
  while ((c = *src3++))
    *dst++ = c; // no warning

  while (EOL != (*dst++ = *src4++)); // no warning
}


extern size_t strlen(const char *s);
void strcpy_impl(char* src, char *dst) {
  size_t len = strlen(src);
  for (int i=0; i < len; i++)
    *dst++ = *src++; // no warning
}

extern const unsigned short int **__ctype_b_loc (void);
#define isalpha(c) \
  ((*__ctype_b_loc ())[(int) ((c))] & (unsigned short int) (1 << 10))
void ctype(char* src, char *dst, int len) {
  if (isalpha(src[0]))
    while (--len)
      *dst++ = *src++; // no warning
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

