/// Check that we do not incorrectly emit a "comparison of distinct pointer types" error
/// This was previously happening due to a bug in Sema::FindCompositePointerType.
// RUN: %cheri_cc1 -fsyntax-only -verify %s
// RUN: %cheri_purecap_cc1 -fsyntax-only -verify %s

// expected-no-diagnostics
int f(long *__capability c1, long *__capability c2) {
  if (c1 == c2)
    return 0;
  if (c1 < c2)
    return 1;
  if (c1 > c2)
    return 2;
  if (c1 <= c2)
    return 3;
  if (c1 >= c2)
    return 4;
  return 0;
}
