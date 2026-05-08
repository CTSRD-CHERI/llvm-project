// RUN: %cheri_cc1 -fsyntax-only -verify %s

/// Verify that the PointerInterpretationKind is preserved for non-canonical
/// pointer types in barCommonNonSugarTypeNode. Previousy it would be lost and
/// hit:
///
///   clang::QualType clang::ASTContext::barCommonSugaredType(clang::QualType, clang::QualType, bool): Assertion `Unqualified ? hasSameUnqualifiedType(R, X) : hasSameType(R, X)' failed.
///
/// (or just proceed with the non-capability type if assertions were disabled).

struct foo {
  void *(*bar)(void __attribute__((btf_type_tag("tag"))) * __capability);
};

void *bar_ptr(void __attribute__((btf_type_tag("tag"))) *);
void *bar_cap(void __attribute__((btf_type_tag("tag"))) * __capability);

struct foo baz[] = {
  { 0 ? bar_ptr : bar_ptr }, // expected-error {{incompatible function pointer types initializing 'void *(*)(void  __attribute__((btf_type_tag("tag")))* __capability)' (aka 'void *(*)(void * __capability)') with an expression of type 'void *(*)(void  __attribute__((btf_type_tag("tag")))*)' (aka 'void *(*)(void *)')}}
  { 0 ? bar_ptr : bar_cap }, // expected-warning {{pointer type mismatch ('void *(*)(void  __attribute__((btf_type_tag("tag")))*)' (aka 'void *(*)(void *)') and 'void *(*)(void  __attribute__((btf_type_tag("tag")))* __capability)' (aka 'void *(*)(void * __capability)'))}}
  { 0 ? bar_cap : bar_ptr }, // expected-warning {{pointer type mismatch ('void *(*)(void  __attribute__((btf_type_tag("tag")))* __capability)' (aka 'void *(*)(void * __capability)') and 'void *(*)(void  __attribute__((btf_type_tag("tag")))*)' (aka 'void *(*)(void *)'))}}
  { 0 ? bar_cap : bar_cap },
};
