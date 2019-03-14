int *notcap;
int * __capability cap;
char * charptr;
char * __capability charcap;
char * should_not_match; // is a capability in var2.c
int * wrong_type; // is a float in var2.c
double foo;
#ifndef __cplusplus
void* fn_decl_cap_mismatch(int* ptr) { return 0; }
#else
extern int& ref_cap_mismatch;

// overload, uses float in other file
void* __capability fn_decl_type_mismatch(int& lvalue, int&& rvalue, int* ptr) { return nullptr; }
// overload, uses capabilities in other file
void* __capability fn_decl_cap_mismatch(int& lvalue, int&& rvalue, int* ptr) { return nullptr; }
// same function
void* __capability fn_decl_cap_same(int& __capability lvalue, int&& __capability rvalue, int* __capability ptr) { return (void* __capability)(__intcap_t)1; }
#endif
