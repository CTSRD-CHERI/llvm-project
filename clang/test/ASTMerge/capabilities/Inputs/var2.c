int *notcap;
int * __capability cap;
char * charptr;
char * __capability charcap;
char * __capability should_not_match; // not a capability in var1.c
float * wrong_type; // is a int in var1.c
double bar;
#ifndef __cplusplus
void* __capability fn_decl_cap_mismatch(int* __capability ptr) { return 0; }
#else
extern int& __capability ref_cap_mismatch;

// overload, uses int in another file
void* __capability fn_decl_type_mismatch(float& lvalue, float&& rvalue, float* ptr) { return nullptr; }
// overload
void* __capability fn_decl_cap_mismatch(int& __capability lvalue, int&& __capability rvalue, int* __capability ptr) { return nullptr; }
// same function
void* __capability fn_decl_cap_same(int& __capability lvalue, int&& __capability rvalue, int* __capability ptr) { return nullptr; }
#endif

