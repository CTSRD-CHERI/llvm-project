// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-llvm -o /dev/null %s -O3 \
// RUN:    -mllvm -collect-csetbounds-stats=csv -mllvm -collect-csetbounds-output=/dev/stderr \
// RUN:  -debug-info-kind=standalone -gno-column-info 2>&1 | FileCheck %s

typedef __SIZE_TYPE__ size_t;

#define __result_use_check __attribute__((__warn_unused_result__))
#define __malloc_like __attribute__((__malloc__))
#define __alloc_size(x) __attribute__((__alloc_size__(x)))
#define __alloc_size2(n, x) __attribute__((__alloc_size__(n, x)))

void *calloc(size_t, size_t) __malloc_like __result_use_check __alloc_size2(1, 2);
void *malloc(size_t) __malloc_like __result_use_check __alloc_size(1);
void *realloc(void *, size_t) __result_use_check __alloc_size(2);

void *my_alloc(int kind, int size) __malloc_like __result_use_check __alloc_size(2);
void *my_alloc_array(int kind, int first, int second) __malloc_like __result_use_check __alloc_size2(2, 3);

void *my_alloc_align64(int kind, int size) __malloc_like __result_use_check __alloc_size(2) __attribute__((assume_aligned(64)));

void *no_alloc_attr(int size) __malloc_like __result_use_check;

// CHECK-LABEL: alignment_bits,size,kind,source_loc,compiler_pass,details
void *test_malloc(int n) {
  return malloc(n);
  // CHECK: 0,<unknown>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to malloc"
}

void *test_malloc2(void) {
  return malloc(16);
  // CHECK: 0,16,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to malloc"
}

void *test_calloc(int n) {
  return calloc(8, n);
  // CHECK: 0,<unknown multiple of 8>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to calloc"
}

void *test_calloc2(void) {
  return calloc(16, 8);
  // CHECK-NEXT: 0,128,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to calloc"
}

void *test_calloc3(int n) {
  return calloc(n, 9);
  // CHECK-NEXT: 0,<unknown multiple of 9>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to calloc"
}

void *test_realloc(void *ptr, int n) {
  return realloc(ptr, n);
  // CHECK-NEXT: 0,<unknown>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to realloc"
}

void *test_realloc2(void *ptr) {
  return realloc(ptr, 55);
  // CHECK-NEXT: 0,55,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to realloc"
}

void *test_my_alloc(int n) {
  return my_alloc(1, n);
  // CHECK-NEXT: 0,<unknown>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc"
}
void *test_my_alloc2(void) {
  return my_alloc(1, 32);
  // CHECK-NEXT: 0,32,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc"
}

void *test_my_alloc_mult(int n) {
  return my_alloc_array(2, 17, n);
  // CHECK-NEXT: 0,<unknown multiple of 17>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc_array"
}
void *test_my_alloc_mult2(void) {
  return my_alloc_array(2, 16, 8);
  // CHECK-NEXT: 0,128,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc_array"
}

void *no_alloc_attr(int n) {
  return no_alloc_attr(n);
}

void *no_alloc_attr2(void) {
  return no_alloc_attr(2);
}

void *test_my_alloc_align64(int n) {
  return my_alloc_align64(1, n);
  // CHECK-NEXT: 6,<unknown>,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc_align64"
}
void *test_my_alloc_align64_2(void) {
  return my_alloc_align64(1, 32);
  // CHECK-NEXT: 6,32,h,"{{.+}}csetbounds-stats-malloc-attr.c:[[@LINE-1]]","function with alloc_size","call to my_alloc_align64"
}

// CHECK-EMPTY:
