/// Check that the attribute can be applied at any position
// RUN: %cheri_purecap_cc1 %s -fsyntax-only -Wall -verify -ast-dump -fcolor-diagnostics
/// Check that it also works in hybrid mode
// RUN: %cheri_cc1 %s -fsyntax-only -Wall -verify=expected,hybrid


typedef __uintcap_t uintptr_t;

typedef __attribute__((cheri_no_provenance)) __uintcap_t no_provenance_uintptr;
typedef __uintcap_t __attribute__((cheri_no_provenance)) no_provenance_uintptr2;
typedef __uintcap_t no_provenance_uintptr3 __attribute__((cheri_no_provenance));

// Generate a MacroQualifiedType
#define __no_provenance __attribute__((cheri_no_provenance))
typedef __no_provenance __uintcap_t no_provenance_uintptr4;
typedef __uintcap_t __no_provenance no_provenance_uintptr5;
typedef __uintcap_t  no_provenance_uintptr6 __no_provenance;

// Ignored on non-(u)intcap types:
typedef __attribute__((cheri_no_provenance)) long badattr;  // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}
typedef long __attribute__((cheri_no_provenance)) badattr2;  // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}
typedef long badattr3 __attribute__((cheri_no_provenance));  // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}

__no_provenance int x = 0; // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}
int __no_provenance x2 = 0; // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}
__no_provenance uintptr_t y = 0;
uintptr_t __no_provenance y2 = 0;
extern void foo() __no_provenance; // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}

struct s {
  __no_provenance uintptr_t u1;
  uintptr_t __no_provenance u2;
  uintptr_t u3 __no_provenance;
  __no_provenance uintptr_t __no_provenance u4 __no_provenance; // expected-warning 2 {{attribute 'cheri_no_provenance' is already applied}}
  uintptr_t prov;
};

void test_field(uintptr_t prov, struct s* s) {
  (void)(prov + s->prov); // expected-warning{{it is not clear which should be used as the source of provenance}}
  (void)(prov + s->u1);
  (void)(prov + s->u2);
  (void)(prov + s->u3);
  (void)(prov + s->u4);
}

long test(uintptr_t prov1, uintptr_t prov2) {
  (void)(prov1 + prov2); // expected-warning{{it is not clear which should be used as the source of provenance}}
  (void)(prov1 + (no_provenance_uintptr)prov2);
  (void)(prov1 + (no_provenance_uintptr2)prov2);
  (void)(prov1 + (no_provenance_uintptr3)prov2);
  (void)(prov1 + (no_provenance_uintptr4)prov2);
  (void)(prov1 + (no_provenance_uintptr5)prov2);
  (void)(prov1 + (no_provenance_uintptr6)prov2);
}
long test_bad(uintptr_t arg) {
  return (long)(__no_provenance long)arg; // expected-error{{'cheri_no_provenance' attribute only applies to capability types}}
}
