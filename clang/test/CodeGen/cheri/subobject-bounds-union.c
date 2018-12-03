// Check that we only set bounds on union member expressions in very-aggressive mode
// FIXME: or should it be aggressive vs safe behave instead (i.e. set bounds in aggressive mode)?
// RUN: %cheri_purecap_cc1 -cheri-bounds=aggressive -O2 -std=c11 -emit-llvm %s -o %t.ll -Wcheri-subobject-bounds -Rcheri-subobject-bounds -verify=aggressive
// RUN: %cheri_purecap_cc1 -cheri-bounds=very-aggressive -O2 -std=c11 -emit-llvm %s -o %t.ll -Wcheri-subobject-bounds -Rcheri-subobject-bounds -verify=very-aggressive

struct sockaddr {
  char data[16];
};

struct sockaddr_in {
  __INT16_TYPE__ sin_family;
  __UINT16_TYPE__ sin_port;
  __UINT32_TYPE__ sin_addr;
  char sin_zero[8];
};

struct sockaddr_in6 {
    __UINT16_TYPE__ sin6_family;
    __UINT16_TYPE__ sin6_port;
    __UINT32_TYPE__ sin6_flowinfo;
    char sin6_addr[16];
    __UINT32_TYPE__ sin6_scope_id;
};

// Example from CheriBSD:
union sockunion {
  struct sockaddr sa;
  struct sockaddr_in sin;
  struct sockaddr_in6    sin6;
};

_Static_assert(sizeof(union sockunion) == 28, "");

void call(void* arg);

void test(union sockunion* un) {
  // For unions only set bounds on individual members for very-aggressive mode and up:
  call(&un->sa);
  // aggressive-remark@-1{{not setting bounds for 'struct sockaddr' (container is union)}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'struct sockaddr' to 16 bytes}}
  call(&un->sin);
  // aggressive-remark@-1{{not setting bounds for 'struct sockaddr_in' (container is union)}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'struct sockaddr_in' to 16 bytes}}
  call(&un->sin6);
  // aggressive-remark@-1{{not setting bounds for 'struct sockaddr_in6' (container is union)}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'struct sockaddr_in6' to 28 bytes}}
}

union WithNestedStruct {
  struct {
    int a;
    int b;
  } nested;
  char buffer[64];
};

void test2(union WithNestedStruct* un) {
  // Accessing a member of a nested struct should set bounds
  call(&un->nested.a);
  // aggressive-remark@-1{{setting sub-object bounds for pointer to 'int' to 4 bytes}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'int' to 4 bytes}}
  call(&un->nested.b);
  // aggressive-remark@-1{{setting sub-object bounds for pointer to 'int' to 4 bytes}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'int' to 4 bytes}}
  // But using the immediate children of the union should not
  call(&un->nested);
  // aggressive-remark-re@-1{{not setting bounds for 'struct (anonymous struct at {{.+}}subobject-bounds-union.c:50:3)' (container is union)}}
  // very-aggressive-remark-re@-2{{setting sub-object bounds for pointer to 'struct (anonymous struct at {{.+}}subobject-bounds-union.c:50:3)' to 8 bytes}}
  call(&un->buffer);
  // aggressive-remark@-1{{not setting bounds for 'char [64]' (container is union)}}
  // very-aggressive-remark@-2{{setting sub-object bounds for pointer to 'char [64]' to 64 bytes}}
}
