// RUNNOT: %cheri128_purecap_cc1 -cheri-bounds=subobject-safe -O2 -std=c++17 -emit-llvm %s -o - \
// RUNNOT:   -Wno-array-bounds -Rcheri-subobject-bounds -ast-dump -fcolor-diagnostics
// RUN: %cheri128_purecap_cc1 -cheri-bounds=subobject-safe -O2 -std=c++17 -emit-llvm %s -o /dev/null -Rcheri-subobject-bounds -verify

// A reduced version of the original test case: pointer_traits<char*>::pointer_to(std::string::__data[0])

// Check that we don't use the type of the reference to bound addrof(reference).
// Since it is already a reference it should be correctly bounded
// TODO: maybe in a super-aggressive mode we could add the bounds

char *pointer_from_ref(char &ref) {
  // we should just assume that the reference is already bounded
  return &ref; // expected-remark{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

char *pointer_from_value(char ref) {
  // Should still set bounds for value types:
  return &ref; // expected-remark{{setting bounds for pointer to 'char' to 1 bytes}}
  // expected-warning@-1{{address of stack memory associated with parameter 'ref' returned}}
}

char& get_reference();
char* pointer_from_ref_call() {
  return &get_reference(); // expected-remark{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

char &ref_from_ptr(char *ptr) {
  return *ptr;  // expected-remark{{setting sub-object bounds for reference to 'char' to 1 bytes}}
}

char &ref_from_ref(char &ref) {
  // No need to add bounds when simply passing on a reference
  return ref; // expected-remark{{not setting bounds for reference to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

char &ref_from_rval_ref(char &&ref) {
  return ref; // expected-remark{{not setting bounds for reference to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

char array_ref_subscript(char (&ref)[5], int n) {
  // Should still set bounds on array subscripts
  return ref[n];  // expected-remark{{setting bounds for array subscript on 'char[5]' to 5 bytes}}
}

// A less reduced version of the original test case: pointer_traits<char*>::pointer_to(std::string::__data[0])

template <class _Tp>
constexpr inline _Tp *addressof(_Tp &__x) noexcept {
  // This previously added bounds to __x and caused std::string to break
  return __builtin_addressof(__x);
  // expected-remark@-1{{not setting bounds for pointer to 'char' (source is a C++ reference and therefore should already have sub-object bounds)}}
}

template <class _Ptr>
struct pointer_traits {};

template <class _Tp>
struct pointer_traits<_Tp *> {
  typedef _Tp *pointer;
  typedef _Tp element_type;

public:
  constexpr static pointer pointer_to(element_type &__r) noexcept {
    // This previously added bounds to __r and caused std::string to break
    return addressof(__r);
    // expected-remark@-1{{not setting bounds for reference to 'pointer_traits<char *>::element_type' (aka 'char') (source is a C++ reference and therefore should already have sub-object bounds)}}
  }
};

void *test() {
  __attribute__((cheri_no_subobject_bounds)) char buffer[10];
  return pointer_traits<char *>::pointer_to(buffer[0]);
  // FIXME: we should probably also take the opt-out annotation into account for array subscripts
  // expected-remark@-2{{setting bounds for array subscript on 'char[10] __attribute__((cheri_no_subobject_bounds))' to 10 bytes}}
  // expected-remark@-3{{not setting bounds for reference to 'char __attribute__((cheri_no_subobject_bounds))' (array type has opt-out attribute)}}
}
