// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=expected,hybrid -ast-dump -o - %s | FileCheck %s -check-prefix AST
// Note: we currently get R_MIPS_CHERI_CAPABILITY relocations even for the casts via long
// RUNNOT: %cheri_cc1 -Wno-error=cheri-capability-misuse -emit-obj -o - %s | llvm-objdump -t -r -
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=expected,hybrid -emit-llvm -o - %s | FileCheck %s --check-prefix=HYBRID
// RUN: %cheri_purecap_cc1 -Wno-error=cheri-capability-misuse -verify=expected,purecap -emit-llvm -o - %s | FileCheck %s --check-prefix=PURECAP

extern void extern_fn(void);
extern char extern_data;

char *__capability global_cap_const = (char *__capability)(long)1234;
// expected-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// hybrid-note@-2{{insert cast to __intcap_t to silence this warning}}
// purecap-note@-3{{insert cast to intptr_t to silence this warning}}
// HYBRID:  @global_cap_const = global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16
// PURECAP: @global_cap_const = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16

// AST-LABEL: VarDecl {{.+}} global_cap_const 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <IntegralCast>
// AST-NEXT:  IntegerLiteral {{.+}} 'int' 1234

char *global_ptr_const = (char *)(long)1234;
// purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// purecap-note@-2{{insert cast to intptr_t to silence this warning}}
// HYBRID-NEXT: @global_ptr_const = global i8* inttoptr (i64 1234 to i8*), align 8
// PURECAP-NEXT: @global_ptr_const = addrspace(200) global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16
// AST-LABEL: VarDecl {{.+}} global_ptr_const 'char *' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char *' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <IntegralCast>
// AST-NEXT:  IntegerLiteral {{.+}} 'int' 1234

// HYBRID-NEXT: @extern_data = external global i8, align 1
// PURECAP-NEXT: @extern_data = external addrspace(200) global i8, align 1
char *global_ptr_data = &extern_data;
// HYBRID-NEXT: @global_ptr_data = global i8* @extern_data, align 8
// PURECAP-NEXT: @global_ptr_data = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_ptr_data 'char *' cinit
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_implicit = &extern_data; // implicit conversion to capability
// HYBRID-NEXT: @global_cap_data_implicit = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_implicit = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_implicit 'char * __capability' cinit
// AST-NEXT:  ImplicitCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_default = (char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_default = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_default = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_default 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_tocap = (__cheri_tocap char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_tocap = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_tocap = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_tocap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_addr_uintcap = (char *__capability)(__cheri_addr __uintcap_t)&extern_data;
// HYBRID-NEXT: @global_cap_data_addr_uintcap = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_uintcap = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_addr_uintcap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} '__uintcap_t':'__uintcap_t' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_addr_long = (char *__capability)(__cheri_addr long)&extern_data;
// FIXME: this warning is incorrect for the IR: purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_addr_long = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_long = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_addr_long 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_via_long = (char *__capability)(long)&extern_data;
// FIXME: this warning is incorrect for the IR: purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_via_long = global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_via_long = addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_via_long 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'

void (*global_fnptr)(void) = &extern_fn;
// HYBRID-NEXT: @global_fnptr = global void ()* @extern_fn, align 8
// PURECAP-NEXT: @global_fnptr = addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fnptr 'void (*)(void)' cinit
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_default)(void) = (void (*__capability)(void)) & extern_fn;
// HYBRID-NEXT: @global_fncap_default = global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_default = addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_default 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_tocap)(void) = (__cheri_tocap void (*__capability)(void)) & extern_fn;
// HYBRID-NEXT: @global_fncap_tocap = global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_tocap = addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_tocap 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_addr)(void) = (void (*__capability)(void))(__cheri_addr __uintcap_t)&extern_fn;
// HYBRID-NEXT: @global_fncap_addr = global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_addr = addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_addr 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} '__uintcap_t':'__uintcap_t' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'

// HYBRID-EMPTY:
// PURECAP-EMPTY:
