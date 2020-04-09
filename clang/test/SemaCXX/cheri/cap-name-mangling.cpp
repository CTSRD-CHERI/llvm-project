// RUN: %cheri_cc1 -target-abi n64 -std=c++11 -o - -emit-llvm %s | FileCheck %s --check-prefixes CHECK,HYBRID
// RUN: %cheri_purecap_cc1 -std=c++11 -o - -emit-llvm %s | FileCheck %s  --check-prefixes CHECK,PURECAP

// See Sema/cheri/issue160.c
// In C++, functions that differ only by __capability are allowed in hybrid mode
// because __capability-qualified pointers and references are mangled differently.
// Also check that purecap does not include __capability in mangled names since it's the default.

struct a;

// Note: in purecap we don't include __capability it in the mangling since it's the default pointer representation
void ptr_qual(struct a * __capability c) {}
// HYBRID: define void @_Z8ptr_qualU12__capabilityP1a(
// PURECAP: define void @_Z8ptr_qualP1a(
void ref_qual(struct a & __capability) {}
// HYBRID: define void @_Z8ref_qualU12__capabilityR1a(
// PURECAP: define void @_Z8ref_qualR1a(
void rref_qual(struct a && __capability) {}
// HYBRID: define void @_Z9rref_qualU12__capabilityO1a(
// PURECAP: define void @_Z9rref_qualO1a(

// No additional mangling for default representation:
void ptr_default(struct a *) {} // CHECK: define void @_Z11ptr_defaultP1a(
void ref_default(struct a &) {} // CHECK: define void @_Z11ref_defaultR1a(
void rref_default(struct a &&) {} // CHECK: define void @_Z12rref_defaultO1a(


// In hybrid mode the __capability qualifier should be mangled using U12__capability
#pragma pointer_interpretation integer
// This is now mangled as a long in purecap and pointer in hybrid:
void ptr_pii(struct a *c) {}
// PURECAP: define void @_Z7ptr_piil(
// HYBRID: define void @_Z7ptr_piiP1a(

// Note: #pragma pointer_interpretation doesn't apply to references
void ref_pii(struct a &) {}  // CHECK: define void @_Z7ref_piiR1a(
void rref_pii(struct a &&) {}  // CHECK: define void @_Z8rref_piiO1a(

#pragma pointer_interpretation capability
// This is now mangled as a pointer in purecap and __capability qualified pointer in hybrid:
void ptr_pic(struct a *c) {}
// PURECAP: define void @_Z7ptr_picP1a(
// HYBRID: define void @_Z7ptr_picU12__capabilityP1a(
// Note: #pragma pointer_interpretation doesn't apply to references
void ref_pic(struct a &) {} // CHECK: define void @_Z7ref_picR1a(
void rref_pic(struct a &&) {}  // CHECK: define void @_Z8rref_picO1a(


#pragma pointer_interpretation default // reset to default

// Check that __uintcap_t is mangled as u11__uintcap_t and intcap_t as u10__intcap_t

void uptr(__UINTPTR_TYPE__ u) {}
// HYBRID: define void @_Z4uptrm(
// PURECAP: define void @_Z4uptru11__uintcap_t(
void sptr(__INTPTR_TYPE__ s) {}
// HYBRID: define void @_Z4sptrl(
// PURECAP: define void @_Z4sptru10__intcap_t(
