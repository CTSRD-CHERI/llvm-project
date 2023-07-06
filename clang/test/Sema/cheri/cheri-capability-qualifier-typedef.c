// RUN: %cheri_cc1 %s -fsyntax-only -verify

#define CHECK_SAME(a, b) _Static_assert(__builtin_types_compatible_p(a *, b *), "")
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define CHECK_TYPE(ty, expected)                                 \
  typedef ty CONCAT(type_, __LINE__);                            \
  CHECK_SAME(CONCAT(type_, __LINE__), expected);                 \
  static ty CONCAT(var_, __LINE__);                              \
  CHECK_SAME(typeof(CONCAT(var_, __LINE__)), expected);          \
  static CONCAT(type_, __LINE__) CONCAT(typedef_var_, __LINE__); \
  CHECK_SAME(typeof(CONCAT(typedef_var_, __LINE__)), expected)

struct s;
typedef struct s s;
typedef s *sptr;
typedef s *__capability scap;
CHECK_TYPE(sptr, struct s *);
CHECK_TYPE(scap, struct s *__capability);

CHECK_TYPE(s *__capability, struct s *__capability);
CHECK_TYPE(__capability s *, struct s *__capability); // expected-warning 2 {{use of __capability before the pointer type is deprecated}}
CHECK_TYPE(s __capability *, struct s *__capability); // expected-warning 2 {{use of __capability before the pointer type is deprecated}}
CHECK_TYPE(sptr __capability, struct s *__capability);
// TODO: should __capability sptr also diagnose legacy usage?
CHECK_TYPE(__capability sptr, struct s *__capability);

// TODO: Should this warn about __capability being ignored?
CHECK_TYPE(__capability scap, struct s *__capability);
CHECK_TYPE(scap __capability, struct s *__capability);

/// Also check two levels of pointers
CHECK_TYPE(s *__capability *, struct s *__capability *);
CHECK_TYPE(scap *, struct s *__capability *);
CHECK_TYPE(__capability sptr *, struct s *__capability *);
CHECK_TYPE(sptr __capability *, struct s *__capability *);
CHECK_TYPE(__capability sptr *__capability, struct s *__capability *__capability);
CHECK_TYPE(sptr __capability *__capability, struct s *__capability *__capability);
// FIXME: this one is a bit odd, maybe should warn about ignored capability qualifier?
CHECK_TYPE(__capability scap *, struct s *__capability *);
CHECK_TYPE(scap __capability *, struct s *__capability *);

CHECK_TYPE(__capability s **, struct s *__capability *); // expected-error 2{{use of __capability is ambiguous}}
// expected-error@-1 3{{static_assert failed due to requirement '__builtin_types_compatible_p(struct s ***, struct s * __capability **)'}}
CHECK_TYPE(s __capability **, struct s *__capability *); // expected-error 2{{use of __capability is ambiguous}}
// expected-error@-1 3{{static_assert failed due to requirement '__builtin_types_compatible_p(struct s ***, struct s * __capability **)'}}
CHECK_TYPE(__capability s *__capability *, struct s *__capability *__capability); // expected-error 2 {{use of __capability is ambiguous}}
// expected-error@-1 3{{static_assert failed due to requirement '__builtin_types_compatible_p(struct s * __capability **, struct s * __capability * __capability *)'}}
