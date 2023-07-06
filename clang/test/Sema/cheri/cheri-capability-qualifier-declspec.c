// RUN: %cheri_cc1 %s -fsyntax-only -verify
// expected-no-diagnostics

void *__capability test_typeof(void *p) {
  __typeof__(p) __capability c = (__cheri_tocap __typeof__(p) __capability)p;
  _Static_assert(__builtin_types_compatible_p(typeof(c), void *__capability), "");
  __capability __typeof__(p) c2 = (__cheri_tocap __capability __typeof__(p))p;
  _Static_assert(__builtin_types_compatible_p(typeof(c2), void *__capability), "");
  __capability __typeof__(p) *__capability c3 = (__cheri_tocap __capability __typeof__(p) *__capability)p;
  _Static_assert(__builtin_types_compatible_p(typeof(c3), void *__capability *__capability), "");
  return c;
}

static int *global_intptr = 0;
static __capability typeof(global_intptr) global_intcap = 0;
_Static_assert(__builtin_types_compatible_p(typeof(global_intptr), int *), "");
_Static_assert(__builtin_types_compatible_p(typeof(global_intcap), int *__capability), "");

/// Crash reproducer for https://github.com/CTSRD-CHERI/llvm-project/issues/710
// RUN: not --crash %cheri_cc1 %s -fsyntax-only -verify -DCRASH
// REQUIRES: asserts
#ifdef CRASH
struct s {
  int i;
};
static struct s *global_sptr = 0;
static __capability typeof(global_sptr) global_scap = 0;
_Static_assert(__builtin_types_compatible_p(typeof(global_sptr), struct s *), "");
_Static_assert(__builtin_types_compatible_p(typeof(global_scap), struct s *__capability), "");
#endif
