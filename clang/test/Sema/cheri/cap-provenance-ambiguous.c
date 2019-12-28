/// Check that we can infer the correct provenance source (-Wcheri-provenance)
/// Warn for all bitwise operators, as well as addition and multiplication
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=|' '-DARITH_EQ_OP=|='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=|' '-DARITH_EQ_OP=|='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=&' '-DARITH_EQ_OP=&='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=&' '-DARITH_EQ_OP=&='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=^' '-DARITH_EQ_OP=^='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=^' '-DARITH_EQ_OP=^='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=+' '-DARITH_EQ_OP=+='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=+' '-DARITH_EQ_OP=+='
/// XXX: multiplication does not make very much sense with two capabilities, maybe don't warn and move it into the inefficient/causing unrepresentable results category?
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=*' '-DARITH_EQ_OP=*='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify -fsyntax-only '-DARITH_OP=*' '-DARITH_EQ_OP=*='
/// We should not warn about ambigous provenance for (non-commutative) operations where it is inherited from the LHS
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative -fsyntax-only '-DARITH_OP=-' '-DARITH_EQ_OP=-='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative -fsyntax-only '-DARITH_OP=-' '-DARITH_EQ_OP=-='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=%' '-DARITH_EQ_OP=%='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=%' '-DARITH_EQ_OP=%='
/// XXX: for shifts we should probably warn that they cause unrepresentable intermediate value?
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=<<' '-DARITH_EQ_OP=<<='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=<<' '-DARITH_EQ_OP=<<='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=>>' '-DARITH_EQ_OP=>>='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=>>' '-DARITH_EQ_OP=>>='
// RUN: %cheri_purecap_cc1 %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=/' '-DARITH_EQ_OP=/='
// RUN: %cheri_purecap_cc1 -xc++ %s -Wall -Wno-cheri-bitwise-operations -verify=non-commutative  -fsyntax-only '-DARITH_OP=/' '-DARITH_EQ_OP=/='
// TODO: check the AST dump for the provenance attributes?

// non-commutative-no-diagnostics

typedef __uintcap_t uintptr_t;
#define __no_provenance __attribute__((cheri_no_provenance))
typedef __no_provenance __uintcap_t no_provenance_uintptr_t;
typedef unsigned int uint;
typedef unsigned long ulong;

#define FLAG_INT 1u
#define FLAG_INTPTR ((uintptr_t)3u)
#define FLAG_NOPROV_INTPTR ((no_provenance_uintptr_t)7u)
enum E {
  ENUM_CONSTANT = 15
};
#ifdef __cplusplus
enum class EClass : uintptr_t {
  Value = 1
};
enum class EClassNoProv : no_provenance_uintptr_t {
  Value = 1
};
#endif

#ifndef ARITH_OP
#error ARITH_OP MISSING
#endif
#ifndef ARITH_EQ_OP
#error ARITH_EQ_OP MISSING
#endif

void check(uintptr_t arg);

void test_op_flag_constant(void *arg) {
  // Check int,intptr_t,noprov_intptr_t constants both LHS and RHS
  // None of these cases should warn since we can always infer that one of them
  // is not a valid capability
  check((uintptr_t)arg ARITH_OP FLAG_INT);
  check((uintptr_t)arg ARITH_OP FLAG_INTPTR);
  check((uintptr_t)arg ARITH_OP FLAG_NOPROV_INTPTR);
  check((uintptr_t)arg ARITH_OP ENUM_CONSTANT);
  check(FLAG_INT ARITH_OP(uintptr_t) arg);
  check(FLAG_INTPTR ARITH_OP(uintptr_t) arg);
  check(FLAG_NOPROV_INTPTR ARITH_OP(uintptr_t) arg);
  check(ENUM_CONSTANT ARITH_OP(uintptr_t) arg);
#ifdef __cplusplus
  // C++11 enum class previously caused false positives.
  // Check that none of these cases warn:
  check((uintptr_t)arg ARITH_OP (uintptr_t)EClass::Value);
  check((uintptr_t)arg ARITH_OP (uintptr_t)EClassNoProv::Value);
  check((uintptr_t)arg ARITH_OP (uintptr_t)(EClass)FLAG_INTPTR);
  check((uintptr_t)arg ARITH_OP (uintptr_t)(EClassNoProv)FLAG_INTPTR);
  check((uintptr_t)EClass::Value ARITH_OP(uintptr_t) arg);
  check((uintptr_t)(EClass)FLAG_INTPTR ARITH_OP(uintptr_t) arg);
  check((uintptr_t)EClassNoProv::Value ARITH_OP(uintptr_t) arg);
  check((uintptr_t)(EClassNoProv)FLAG_INTPTR ARITH_OP(uintptr_t) arg);
#endif
  // But not if there is an intermediate assignment
  // TODO: could adjust the type here when doing the assignment operation to track provenance
  no_provenance_uintptr_t flag_noprov = FLAG_INT;
  check(flag_noprov ARITH_OP(uintptr_t) arg);
  check((uintptr_t)arg ARITH_OP flag_noprov);

  // Casting from a no-provenance type to a provenance type should fix the warning
  check((uintptr_t)flag_noprov ARITH_OP(uintptr_t) arg);
  check((uintptr_t)arg ARITH_OP(uintptr_t) flag_noprov);

  uintptr_t flag_prov = FLAG_INT;
  // These cases currently warn since we don't walk the CFG, but that is probably reasonable
  check(flag_prov ARITH_OP(uintptr_t) arg); // expected-warning{{binary expression on capability types 'uintptr_t' (aka '__uintcap_t') and 'uintptr_t'; it is not clear which should be used as the source of provenance;}}
  check((uintptr_t)arg ARITH_OP flag_prov); // expected-warning{{binary expression on capability types 'uintptr_t' (aka '__uintcap_t') and 'uintptr_t'; it is not clear which should be used as the source of provenance;}}

  // but casting to the no-provenance type should work
  check((no_provenance_uintptr_t)flag_prov ARITH_OP(uintptr_t) arg);
  check((__no_provenance uintptr_t)flag_prov ARITH_OP(uintptr_t) arg);
  check((uintptr_t)arg ARITH_OP(no_provenance_uintptr_t) flag_prov);
  check((uintptr_t)arg ARITH_OP(__no_provenance uintptr_t) flag_prov);
}
void test_ambiguous(uintptr_t a, uintptr_t b) {
  check(a ARITH_OP b); // expected-warning{{binary expression on capability types 'uintptr_t' (aka '__uintcap_t') and 'uintptr_t'; it is not clear which should be used as the source of provenance}}
  // Casting one of the operands should silence the warning
  check(a ARITH_OP(__no_provenance uintptr_t) b);                            // no warning
  check((__no_provenance uintptr_t)a ARITH_OP b);                            // no warning
  check((__no_provenance uintptr_t)a ARITH_OP(__no_provenance uintptr_t) b); // no warning
  // Also when casting to an integer type (that is then promoted again)
  check(a ARITH_OP(ulong) b);        // no warning
  check((ulong)a ARITH_OP b);        // no warning
  check((ulong)a ARITH_OP(ulong) b); // no warning for implicit promotion after operation
}

void cast_noprov_via_ptr(uintptr_t arg, no_provenance_uintptr_t noprov, uintptr_t prov) {
  check(arg ARITH_OP noprov);                    // no warning
  check(noprov ARITH_OP(uintptr_t)(void *) arg); // no warning

  // casting to pointer and back retains ambiguity
  check((uintptr_t)(void *)prov ARITH_OP arg); // expected-warning{{binary expression on capability types 'uintptr_t' (aka '__uintcap_t') and 'uintptr_t'; it is not clear which should be used as the source of provenance}}
  check(arg ARITH_OP(uintptr_t)(void *) prov); // expected-warning{{binary expression on capability types 'uintptr_t' (aka '__uintcap_t') and 'uintptr_t'; it is not clear which should be used as the source of provenance}}
  // And casting to a noprov type clears it:
  check((no_provenance_uintptr_t)(void *)prov ARITH_OP arg);
  check(arg ARITH_OP(no_provenance_uintptr_t)(void *) prov);

  // casting via pointer types should retain the no-provenance attribute
  check((uintptr_t)(void *)noprov ARITH_OP arg);
  check((uintptr_t)(void *)1 ARITH_OP arg);
  check(arg ARITH_OP(uintptr_t)(void *) noprov);
  check(arg ARITH_OP(uintptr_t)(void *) 1);
}

/// Check warnings for compound assignment operators.
/// Here the provenance is clear, but it may be inefficient to use uintptr_t
/// TODO: Implement the inefficient warning
void assign_op(long l, long l2,
               no_provenance_uintptr_t noprov, no_provenance_uintptr_t noprov2,
               uintptr_t prov, uintptr_t prov2) {
  // No diagnostics expected here (yet)
  prov ARITH_EQ_OP noprov2;
  prov ARITH_EQ_OP prov2;
  prov ARITH_EQ_OP l2;

  noprov ARITH_EQ_OP noprov2;
  noprov ARITH_EQ_OP prov2;
  noprov ARITH_EQ_OP l2;

  l ARITH_EQ_OP l2;
  l ARITH_EQ_OP prov2;
  l ARITH_EQ_OP noprov2;
}

#ifdef __cplusplus
/// While enum class values should not hold values not in the enumeration, let's
/// be conservative here and assume enum classes with underlying type uintptr_t
/// can hold valid pointers
void enum_class(EClass EC, EClassNoProv ECNP, EClass EC2, EClassNoProv ECNP2, uintptr_t U, uintptr_t U2) {
  check((uintptr_t)EC ARITH_OP (uintptr_t)EC2); // expected-warning{{it is not clear which should be used as the source of provenance}}
  check((uintptr_t)EC ARITH_OP (uintptr_t)ECNP2);
  check((uintptr_t)EC ARITH_OP U2); // expected-warning{{it is not clear which should be used as the source of provenance}}

  check((uintptr_t)ECNP ARITH_OP (uintptr_t)EC2);
  check((uintptr_t)ECNP ARITH_OP (uintptr_t)ECNP2);
  check((uintptr_t)ECNP ARITH_OP U2);

  check(U ARITH_OP (uintptr_t)EC2); // expected-warning{{it is not clear which should be used as the source of provenance}}
  check(U ARITH_OP (uintptr_t)ECNP2);
  check(U ARITH_OP U2); // expected-warning{{it is not clear which should be used as the source of provenance}}
}
#endif
