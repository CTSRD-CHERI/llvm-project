// RUN: %clang_cc1 -fsyntax-only -verify -Wfirst-field-punning %s

typedef struct {
  void *ptr;
  int parentId;
  char shared_buf[256];
} shared_t;

typedef struct {
  shared_t s; // expected-note {{pointer originated here as '&'a_t'.'s'' (first field)}}
  int id;
} a_t;

typedef struct {
  shared_t s; // expected-note {{pointer originated here as '&'b_t'.'s'' (first field)}}
  int bId;
} b_t;

typedef struct {
  int tag;
  shared_t s; // not first field
  int id;
} not_first_t;

void positive_triggers_a(a_t *a) {
  shared_t *p = &a->s;
  a_t *ap = (a_t *)p; // expected-warning {{from a pointer to its first subobject}}
  ap->id = 1;
}

void positive_triggers_b(b_t *b) {
  shared_t *p = &b->s;
  b_t *bp = (b_t *)p; // expected-warning {{from a pointer to its first subobject}}
  bp->bId = 1;
}

void negative_same_type_no_warning(a_t *a) {
  shared_t *p = &a->s;
  shared_t *q = (shared_t *)p;
  (void)q;
}

void negative_not_first_field_no_warning(not_first_t *n) {
  shared_t *p = &n->s;
  not_first_t *np = (not_first_t *)p; // should NOT warn since we're looking for "first field only"
  (void)np;
}
