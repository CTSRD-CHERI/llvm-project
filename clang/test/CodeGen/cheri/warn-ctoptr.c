// Check that we emit sensible code for constants casted through intcap_t to a void* in hybrid mode
// See the extremely confusing behaviour in https://github.com/CTSRD-CHERI/cheribsd/commit/0ad6f6cceb44636c6c08f816953600ba53779df5 (SIG_DFL == SIG_IGN)
// Simplified test case for the error we had in hybrid mode comparing values to SIG_IGN (works fine in purecap)

// Check all the warnings:
// TODO: Check the IR instead of assembly
// RUN: %cheri_cc1 -Wcheri-pointer-conversion -DCHECK_BAD_CODE %s -emit-llvm -o /dev/null -verify -cheri-uintcap=offset
// RUN: %cheri_purecap_cc1 -Wcheri-pointer-conversion -DCHECK_BAD_CODE -S %s -o - -O0 -cheri-uintcap=offset -verify=purecap
// RUN: %cheri_cc1 -Wcheri-pointer-conversion -S %s -o - -O1 -verify | FileCheck %s -implicit-check-not ctoptr  --check-prefixes CHECK,HYBRID

// And finally try compiling this in purecap mode (and verify doesn't create any ctoptr):
// RUN: %cheri_purecap_cc1 -Wcheri-pointer-conversion -S %s -o - -O1 -verify=purecap-codegen | FileCheck %s -implicit-check-not ctoptr --check-prefixes CHECK
// purecap-codegen-no-diagnostics

// CHECK: .file "warn-ctoptr.c"

typedef __UINTPTR_TYPE__ uintptr_t;

#define SIG_DFL ((void *)(unsigned __intcap)0)
#define SIG_IGN ((void *)(unsigned __intcap)1)
// The cast to unsigned __intcap will cause a ctoptr to be used on the cincoffset NULL, 1
// It then uses ctoptr on that untagged value which will result in 0 being returned form ctoptr
// ctoptr is almost never what is intended in hybrid mode (at least for constants), diagnose it

// This will behave differently since the global SIG_IGN will be a simple .8byte 1
// which means we are comparing ctoptr cap to 1
static void *GLOBAL_SIG_IGN = SIG_IGN;

int test(void *__capability cap) {
  // This causes two ctoptrs to be generated
  return (uintptr_t)cap == (uintptr_t)SIG_IGN; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr use __builtin_cheri_cap_to_pointer() or a __cheri_fromcap cast to silence this warning; to get the virtual address use __builtin_cheri_address_get() or a __cheri_addr cast; to get the capability offset use __builtin_cheri_offset_get() or a __cheri_offset cast}}
  // expected-warning@-2{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-3{{if you really intended to use CToPtr use __builtin_cheri_cap_to_pointer() or a __cheri_fromcap cast to silence this warning; to get the virtual address use __builtin_cheri_address_get() or a __cheri_addr cast; to get the capability offset use __builtin_cheri_offset_get() or a __cheri_offset cast}}
  // CHECK-LABEL: test:
  // HYBRID: ctoptr {{.+}}, $ddc
  // HYBRID: ctoptr {{.+}}, $ddc
  // CHECK .end test
}

uintptr_t test2a(void *__capability cap) {
  return (uintptr_t)cap; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr use __builtin_cheri_cap_to_pointer() or a __cheri_fromcap cast to silence this warning; to get the virtual address use __builtin_cheri_address_get() or a __cheri_addr cast; to get the capability offset use __builtin_cheri_offset_get() or a __cheri_offset cast}}
  // CHECK-LABEL: test2a:
  // HYBRID: ctoptr {{.+}}, $ddc
  // CHECK .end test2a
}

uintptr_t test2b(void *__capability cap) {
  // This one doesn't include a ctoptr since it loads the globals
  return (uintptr_t)GLOBAL_SIG_IGN;
}

int test3(void *ptr) {
  // Don't warn here, we used an explicit __cheri_fromcap
  return ptr == ((__cheri_fromcap void *)(unsigned __intcap)1);
  // CHECK-LABEL: test3:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end test4
}

int test3a(void *ptr) {
  // Don't warn here, we used an explicit __cheri_fromcap
  return ptr == ((__cheri_fromcap void *)(__intcap)1);
  // CHECK-LABEL: test3a:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end test3a
}

int test4(void *ptr) {
#if defined(__CHERI_PURE_CAPABILITY__) && !defined(CHECK_BAD_CODE)
  return 0;
#else
  // Don't warn here, we used an explicit __builtin_cheri_ctoptr
  return ptr == __builtin_cheri_cap_to_pointer(__builtin_cheri_global_data_get(), (void *__capability)(unsigned __intcap)1);
  // purecap-error@-1{{builtin is not supported on this target}}
#endif
  // CHECK-LABEL: test4:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end test4
}

// FIXME: We should guess the right instruction (possibly opt-in with -data-dependent-provenacne)
void *cast_constant_uintcap_to_intptr(void) {
  // trivial to constant fold (should return untagged 1 and warn)
  return (void *)(unsigned __intcap)1; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  // CHECK-LABEL: cast_constant_uintcap_to_intptr:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end cast_constant_uintcap_to_intptr
}

void *cast_constant_uintcap_to_intptr2(void) {
  // Not trivial to constant fold (should return untagged 1 and warn)
  unsigned __intcap cap = 1;
  return (void *)cap; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  // CHECK-LABEL: cast_constant_uintcap_to_intptr2:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end cast_constant_uintcap_to_intptr2
}

void *cast_uintcap_to_intptr(unsigned __intcap cap) {
  // Impossible to constant fold (should return ctoptr and warn)
  return (void *)cap; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  // CHECK-LABEL: cast_uintcap_to_intptr:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end cast_uintcap_to_intptr
}

int cheribsd_test(void) {
  // Compiler generated CToPtr instructions cause very confusing errors such as SIG_IGN == SIG_DFL
  // See https://github.com/CTSRD-CHERI/cheribsd/commit/0ad6f6cceb44636c6c08f816953600ba53779df5
  // (It also broke PT_CONTINUE and other cases of magic integer values casted to pointers)
  // Since CToPtr returns 0 if the argument is untagged all magic unsigned __intcap constants are turned into zero
  // It becomes even more confusing because for globals we get the untagged int value but inside functions we don't
  // And for extra fun: If the global variable is static the compiler would remove the variable load at -O1 and higher
  // and turn it into the CToPtr call
  return SIG_IGN == SIG_DFL; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  // CHECK-LABEL: cheribsd_test:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end cheribsd_test
}

#define SIG_DFL1 ((void *)(unsigned __intcap)0)
#define SIG_IGN1 ((void *)(unsigned __intcap)1)
#define SIG_ERR1 ((void *)(unsigned __intcap)-1)

// Upstream freebsd definitions (we had to add the unsigned __intcap to silence the warning):
#if 0
#define SIG_DFL ((__sighandler_t *)0)
#define SIG_IGN ((__sighandler_t *)1)
#define SIG_ERR ((__sighandler_t *)-1)
/* #define	SIG_CATCH	((__sighandler_t *)2) See signalvar.h */
#define SIG_HOLD ((__sighandler_t *)3)
typedef	void __sighandler_t(int);
#endif

extern void do_stuff(void *);

int test_sig_macros() {
  void *a1 = SIG_DFL1; // this one is fine since it is a null pointer conversion:
  do_stuff(a1);
  void *a2 = SIG_IGN1; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  do_stuff(a2);
  void *a3 = SIG_ERR1; // expected-warning{{the following conversion will result in a CToPtr operation;}}
  // expected-note@-1{{if you really intended to use CToPtr}}
  do_stuff(a3);

  return 0;
  // CHECK-LABEL: test_sig_macros:
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // HYBRID: ctoptr ${{[0-9]+}}, $c{{[0-9]+}}, $ddc
  // CHECK .end test_sig_macros
}

#ifdef CHECK_BAD_CODE
void *cast_capptr_to_intptr_implicit(void *__capability cap) {
  return cap; // expected-error{{converting capability type 'void * __capability' to non-capability type 'void *' without an explicit cast}}
}

void *cast_capptr_to_intptr_explicit(void *__capability cap) {
  return (void *)cap; // expected-error{{cast from capability type 'void * __capability' to non-capability type 'void *' is most likely an error}}
}

void *cast_uintcap_to_intptr_implicit(unsigned __intcap cap) {
  // Should also warn about ctoptr
  return cap; // expected-warning{{incompatible integer to pointer conversion returning 'unsigned __intcap' from a function with result type 'void *'}}
  // purecap-warning@-1{{incompatible integer to pointer conversion returning 'unsigned __intcap' from a function with result type 'void *'}}
}

void *__capability cast_uintcap_to_cap_ptr_implicit(unsigned __intcap cap) {
  // Should also warn about ctoptr
  return cap; // expected-warning{{incompatible integer to pointer conversion returning 'unsigned __intcap' from a function with result type 'void * __capability'}}
  // purecap-warning@-1{{incompatible integer to pointer conversion returning 'unsigned __intcap' from a function with result type 'void *'}}
}

void *cast_uintcap_to_intptr_explicit(unsigned __intcap cap) {
  return (void *)cap; // this should warn!
}
#endif

// Check that this does not warn since it only uses cfromddc (which could also be a bug but a different one):
#define PTREXPAND_CP(src, dst, fld)                        \
  do {                                                     \
    (dst).fld = (void *__capability)(__intcap)(src).fld; \
  } while (0)

struct kinfo_proc {
  struct vmspace *ki_vmspace; /* pointer to kernel vmspace struct */
  char other_stuff[1024];
};
struct kinfo_proc_c {
  void *__capability ki_vmspace; /* struct vmspace */
  char other_stuff[1024];
};

void *
cheriabi_kinfo_proc_out(const struct kinfo_proc *ki, struct kinfo_proc_c *ki_c) {
  PTREXPAND_CP(*ki, *ki_c, ki_vmspace);
  return ki_c;
  // CHECK-LABEL: cheriabi_kinfo_proc_out
  // CHECK-NOT: cfromddc
  // CHECK-NOT: cfromptr
  // HYBRID:     cincoffset	$c{{[0-9]+}}, $cnull, ${{[0-9]+}}
  // CHECK-NOT: cfromddc
  // CHECK-NOT: cfromptr
  // CHECK: .end cheriabi_kinfo_proc_out
}

void *__capability same_thing_expanded(const struct kinfo_proc *ki, struct kinfo_proc_c *ki_c) {
  (*ki_c).ki_vmspace = (void *__capability)(__intcap)(*ki).ki_vmspace;
  return ki_c->ki_vmspace;
  // CHECK-LABEL: same_thing_expanded:
  // CHECK-NOT: cfromddc
  // CHECK-NOT: cfromptr
  // HYBRID:     cincoffset	$c{{[0-9]+}}, $cnull, ${{[0-9]+}}
  // CHECK-NOT: cfromddc
  // CHECK-NOT: cfromptr
  // CHECK: .end same_thing_expanded
}
