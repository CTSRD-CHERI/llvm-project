// RUN: %cheri_purecap_cc1 -std=gnu99 -cheri-bounds=conservative %s -verify=nobounds
// nobounds-no-diagnostics
// RUN: %cheri_purecap_cc1 -std=gnu99 -cheri-bounds=subobject-safe %s -verify=expected

#if !__has_builtin(__builtin_marked_no_subobject_bounds)
#error __builtin_marked_no_subobject_bounds not supported?
#endif

#define offsetof(type, member) __builtin_offsetof(type, member)
#define container_of(ptr, type, member) ({				\
	const __typeof(((type *)0)->member) *__x = (ptr);		\
	((type *)((const char*)__x - offsetof(type, member))); })

#ifdef __CHERI_SUBOBJECT_BOUNDS__
#define check_safe_type_for_containerof(type)                \
  _Static_assert(__builtin_marked_no_subobject_bounds(type), \
                 "this type is unsafe for use in containerof() with sub-object bounds. Please mark the member/type with __no_subobject_bounds")
#else
// No checks without sub-object bounds
#define check_safe_type_for_containerof(type)
#endif

#define container_of_safe(ptr, type, member) ({				\
	const __typeof(((type *)0)->member) *__x = (ptr);		\
	check_safe_type_for_containerof(__typeof(((type *)0)->member));        \
	((type *)((const char*)__x - offsetof(type, member))); })

struct list_entry {
  struct list_entry *next;
};

_Static_assert(!__builtin_marked_no_subobject_bounds(char), "");
_Static_assert(__builtin_marked_no_subobject_bounds(__attribute__((cheri_no_subobject_bounds)) char), "");
// Also check typedefs
typedef struct list_entry list_entry_bounds;
typedef __attribute__((cheri_no_subobject_bounds)) struct list_entry list_entry_nobounds1;
typedef struct list_entry list_entry_nobounds2 __attribute__((cheri_no_subobject_bounds));
typedef struct list_entry __attribute__((cheri_no_subobject_bounds)) list_entry_nobounds3;

_Static_assert(!__builtin_marked_no_subobject_bounds(list_entry_bounds), "");
_Static_assert(__builtin_marked_no_subobject_bounds(list_entry_nobounds1), "");
_Static_assert(__builtin_marked_no_subobject_bounds(list_entry_nobounds2), "");
_Static_assert(__builtin_marked_no_subobject_bounds(list_entry_nobounds3), "");

// Check that we can warn if we use something unsafe
struct mytype_bad {
  int value;
  struct list_entry list; // should be annotated with no subobject bounds
};

struct mytype_bad *next_bad_nodiag(struct mytype_bad *ptr, struct mytype_bad *new_entry) {
  struct list_entry *next_entry = ptr->list.next;
  return container_of(next_entry, struct mytype_bad, list); // nothing
}

struct mytype_bad *next_bad_diag(struct mytype_bad *ptr, struct mytype_bad *new_entry) {
  struct list_entry *next_entry = ptr->list.next;
  // This should generate an error because list is not annotated as no_subobject_bounds
  return container_of_safe(next_entry, struct mytype_bad, list);
  // expected-error@-1{{static_assert failed due to requirement '__builtin_marked_no_subobject_bounds(struct list_entry)' "this type is unsafe for use in containerof() with sub-object bounds. Please mark the member/type with __no_subobject_bounds"}}
}

struct mytype_good {
  int value;
  __attribute__((cheri_no_subobject_bounds)) struct list_entry list;
};

struct mytype_good *next_good(struct mytype_good *ptr, struct mytype_good *new_entry) {
  struct list_entry *next_entry = ptr->list.next;
  // Should not emit a diagnostic
  return container_of_safe(next_entry, struct mytype_good, list);
}

struct __attribute__((cheri_no_subobject_bounds)) list_entry_good {
  struct list_entry_good *next;
};
// Note: can't just look at the type, also need to look at the type declaration:
_Static_assert(__builtin_marked_no_subobject_bounds(struct list_entry_good), "");
struct mytype_good2 {
  int value;
  struct list_entry_good list;
};
struct mytype_good2 *next_good2(struct mytype_good2 *ptr, struct mytype_good2 *new_entry) {
  struct list_entry_good *next_entry = ptr->list.next;
  // Should not emit a diagnostic
  return container_of_safe(next_entry, struct mytype_good2, list);
}
