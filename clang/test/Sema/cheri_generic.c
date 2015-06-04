// RUN: %clang_cc1 "-target-abi" "sandbox" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// expected-no-diagnostics
_Static_assert(_Generic(((float _Complex)1.0i), float _Complex: 1, default: 0),
               "_Complex_I must be of type float _Complex");
#define x(d) _Generic((d), float: 1, double: 2)

int y(float z)
{
  return x(z);
}

