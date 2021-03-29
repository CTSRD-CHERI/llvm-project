/// Check that we can differentiate NULL-derived capabilities (e.g. via __cheri_addr casts)
/// and valid global capabilities in the IR. The assembly/object output is tested in LLVM.
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=expected,hybrid -ast-dump -o - %s | FileCheck %s -check-prefix AST
// RUN: %cheri_purecap_cc1 -Wno-error=cheri-capability-misuse -verify=expected,purecap -emit-llvm -o - %s | FileCheck %s --check-prefix=PURECAP
// RUN: %cheri_cc1 -Wno-error=cheri-capability-misuse -verify=expected,hybrid -emit-llvm -o - %s | FileCheck %s --check-prefix=HYBRID
// hybrid-no-diagnostics

extern void extern_fn(void);
extern char extern_data;

char *__capability global_cap_const = (char *__capability)(long)1234;
// HYBRID:  @global_cap_const = dso_local global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16
// PURECAP: @global_cap_const = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1234), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_const 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <IntegralCast>
// AST-NEXT:  IntegerLiteral {{.+}} 'int' 1234
char *__capability global_cap_const_tocap = (__cheri_tocap char *__capability)(char *)1234;
// HYBRID-NEXT: @global_cap_const_tocap = dso_local global i8 addrspace(200)* inttoptr (i64 1234 to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_const_tocap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1234), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_const_tocap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  CStyleCastExpr {{.+}} 'char *' <IntegralToPointer>
// AST-NEXT:  IntegerLiteral {{.+}} 'int' 1234
char *global_ptr_const = (char *)(long)1234;
// HYBRID-NEXT: @global_ptr_const = dso_local global i8* inttoptr (i64 1234 to i8*), align 8
// PURECAP-NEXT: @global_ptr_const = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 1234), align 16
// AST-LABEL: VarDecl {{.+}} global_ptr_const 'char *' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char *' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <IntegralCast>
// AST-NEXT:  IntegerLiteral {{.+}} 'int' 1234

// HYBRID-NEXT: @extern_data = external global i8, align 1
// PURECAP-NEXT: @extern_data = external addrspace(200) global i8, align 1
char *global_ptr_data = &extern_data;
// HYBRID-NEXT: @global_ptr_data = dso_local global i8* @extern_data, align 8
// PURECAP-NEXT: @global_ptr_data = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_ptr_data 'char *' cinit
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_implicit = &extern_data; // implicit conversion to capability
// HYBRID-NEXT: @global_cap_data_implicit = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_implicit = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_implicit 'char * __capability' cinit
// AST-NEXT:  ImplicitCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_default = (char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_default = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_default = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_default 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_tocap = (__cheri_tocap char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_tocap = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_tocap = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_tocap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_addr_uintcap = (char *__capability)(__cheri_addr unsigned __intcap)&extern_data;
// FIXME: The following variable should be null-derived in hybrid mode, but that needs some __cheri_addr AST changes
// FIXME-HYBRID-NEXT: @global_cap_data_addr_uintcap = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* @extern_data to i64)), align 16
// HYBRID-NEXT: @global_cap_data_addr_uintcap = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_uintcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_addr_uintcap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'unsigned __intcap' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_addr_uintcap_src_is_cap = (char *__capability)(__cheri_addr unsigned __intcap)(char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_addr_uintcap_src_is_cap = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* @extern_data to i64)), align 16
// PURECAP-NEXT: @global_cap_data_addr_uintcap_src_is_cap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_addr_uintcap_src_is_cap 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'unsigned __intcap' <CHERICapabilityToAddress>
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
/// The following two should result in an addrspacecast in hybrid mode:
char *__capability global_cap_data_via_uintcap_explicit = (char *__capability)(unsigned __intcap)(char *__capability) & extern_data;
// HYBRID-NEXT: @global_cap_data_via_uintcap_explicit = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_via_uintcap_explicit = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_via_uintcap_explicit 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'unsigned __intcap' <PointerToIntegral>
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_via_uintcap_default = (char *__capability)(unsigned __intcap)&extern_data;
// HYBRID-NEXT: @global_cap_data_via_uintcap_default = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_via_uintcap_default = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_via_uintcap_default 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'unsigned __intcap' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_addr_long = (char *__capability)(__cheri_addr long)&extern_data;
// purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_addr_long = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_addr_long 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'
char *__capability global_cap_data_via_long = (char *__capability)(long)&extern_data;
// purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_via_long = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_via_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
// AST-LABEL: VarDecl {{.+}} global_cap_data_via_long 'char * __capability' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'char * __capability __attribute__((cheri_no_provenance))':'char * __capability' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'long' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'char *' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'char' lvalue Var {{.+}} 'extern_data' 'char'

void (*global_fnptr)(void) = &extern_fn;
// HYBRID-NEXT: @global_fnptr = dso_local global void ()* @extern_fn, align 8
// PURECAP-NEXT: @global_fnptr = dso_local addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fnptr 'void (*)(void)' cinit
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_default)(void) = (void (*__capability)(void)) & extern_fn;
// HYBRID-NEXT: @global_fncap_default = dso_local global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_default = dso_local addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_default 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_tocap)(void) = (__cheri_tocap void (*__capability)(void)) & extern_fn;
// HYBRID-NEXT: @global_fncap_tocap = dso_local global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_tocap = dso_local addrspace(200) global void () addrspace(200)* @extern_fn, align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_tocap 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <PointerToCHERICapability>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'
void (*__capability global_fncap_addr)(void) = (void (*__capability)(void))(__cheri_addr unsigned __intcap)&extern_fn;
// HYBRID-NEXT: @global_fncap_addr = dso_local global void () addrspace(200)* addrspacecast (void ()* @extern_fn to void () addrspace(200)*), align 16
// PURECAP-NEXT: @global_fncap_addr = dso_local addrspace(200) global void () addrspace(200)* bitcast (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (void () addrspace(200)* @extern_fn to i64)) to void () addrspace(200)*), align 16
// AST-LABEL: VarDecl {{.+}} global_fncap_addr 'void (* __capability)(void)' cinit
// AST-NEXT:  CStyleCastExpr {{.+}} 'void (* __capability)(void)' <IntegralToPointer>
// AST-NEXT:  CStyleCastExpr {{.+}} 'unsigned __intcap' <PointerToIntegral>
// AST-NEXT:  UnaryOperator {{.+}} 'void (*)(void)' prefix '&' cannot overflow
// AST-NEXT:  DeclRefExpr {{.+}} 'void (void)' Function {{.+}} 'extern_fn' 'void (void)'

/// More code-generation checks here, but don't bother with the AST dump:

/// This should result in a tagged value
__intcap global_intcap_data_default = (__intcap)&extern_data;
// HYBRID-NEXT: @global_intcap_data_default = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_default = dso_local addrspace(200) global i8 addrspace(200)* @extern_data, align 16
/// Same for one-past-end and OOB values:
__intcap global_intcap_data_one_past_end_default = (__intcap)(&extern_data + 1);
// HYBRID-NEXT: @global_intcap_data_one_past_end_default = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_one_past_end_default = dso_local addrspace(200) global i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1), align 16
__intcap global_intcap_data_plus_two_default = (__intcap)(&extern_data + 2);
// HYBRID-NEXT: @global_intcap_data_plus_two_default = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* @extern_data, i64 2) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_plus_two_default = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* @extern_data, i64 2), align 16

/// __cheri_addr casts should ensure that it's untagged
__intcap global_intcap_data_addr_cast_long = (__cheri_addr long)&extern_data;
// HYBRID-NEXT: @global_intcap_data_addr_cast_long = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* @extern_data to i64)), align 16
// PURECAP-NEXT: @global_intcap_data_addr_cast_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
__intcap global_intcap_data_addr_cast_intcap = (__cheri_addr __intcap)&extern_data;
// FIXME: this should be null-derived in hybrid mode
// HYBRID-NEXT: @global_intcap_data_addr_cast_intcap = dso_local global i8 addrspace(200)* addrspacecast (i8* @extern_data to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_addr_cast_intcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* @extern_data to i64)), align 16
__intcap global_intcap_data_one_past_end_addr_cast_long = (__cheri_addr long)(&extern_data + 1);
// HYBRID-NEXT: @global_intcap_data_one_past_end_addr_cast_long = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i64)), align 16
// PURECAP-NEXT: @global_intcap_data_one_past_end_addr_cast_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1) to i64)), align 16
__intcap global_intcap_data_one_past_end_addr_cast_intcap = (__cheri_addr __intcap)(&extern_data + 1);
// FIXME: this should be null-derived in hybrid mode
// HYBRID-NEXT: @global_intcap_data_one_past_end_addr_cast_intcap = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_one_past_end_addr_cast_intcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1) to i64)), align 16
__intcap global_intcap_data_plus_two_addr_cast_long = (__cheri_addr long)(&extern_data + 2);
// HYBRID-NEXT: @global_intcap_data_plus_two_addr_cast_long = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr (i8, i8* @extern_data, i64 2) to i64)), align 16
// PURECAP-NEXT: @global_intcap_data_plus_two_addr_cast_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* @extern_data, i64 2) to i64)), align 16
__intcap global_intcap_data_plus_two_addr_cast_intcap = (__cheri_addr __intcap)(&extern_data + 2);
// FIXME: this should be null-derived in hybrid mode
// HYBRID-NEXT: @global_intcap_data_plus_two_addr_cast_intcap = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* @extern_data, i64 2) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_data_plus_two_addr_cast_intcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* @extern_data, i64 2) to i64)), align 16

/// Similar tests with address of function:
__intcap global_intcap_fn = (__intcap)(&extern_fn);
// HYBRID-NEXT: @global_intcap_fn = dso_local global i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @extern_fn to i8*) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn = dso_local addrspace(200) global i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), align 16
__intcap global_intcap_fn_cast_addr = (__cheri_addr __intcap)((char *)&extern_fn);
// FIXME: should be null-derived in hybrid mode
// HYBRID-NEXT: @global_intcap_fn_cast_addr = dso_local global i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @extern_fn to i8*) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn_cast_addr = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (void () addrspace(200)* @extern_fn to i64)), align 16
/// This one is tagged since we are just casting via capability types:
__intcap global_intcap_fn_cast_tocap = (__intcap)(__cheri_tocap char *__capability)((char *)&extern_fn);
// HYBRID-NEXT: @global_intcap_fn_cast_tocap = dso_local global i8 addrspace(200)* addrspacecast (i8* bitcast (void ()* @extern_fn to i8*) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn_cast_tocap = dso_local addrspace(200) global i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), align 16

/// FIXME: should these generate a compiler error? I don't think we want pointers to the middle of functions to exist...
__intcap global_intcap_fn_plus_two_intcap = (__intcap)((char *)&extern_fn + 2);
// HYBRID-NEXT: @global_intcap_fn_plus_two_intcap = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn_plus_two_intcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), i64 2), align 16
/// Just the address should be fine though?
__intcap global_intcap_fn_plus_two_intcap_addr_intcap = (__cheri_addr __intcap)((char *)&extern_fn + 2);
// FIXME: should be null-derived in hybrid
// HYBRID-NEXT: @global_intcap_fn_plus_two_intcap_addr_intcap = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn_plus_two_intcap_addr_intcap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), i64 2) to i64)), align 16
__intcap global_intcap_fn_plus_two_intcap_addr_long = (__cheri_addr long)((char *)&extern_fn + 2);
// HYBRID-NEXT: @global_intcap_fn_plus_two_intcap_addr_long = dso_local global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i64)), align 16
// PURECAP-NEXT: @global_intcap_fn_plus_two_intcap_addr_long = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), i64 2) to i64)), align 16
/// This one should probably not be allowed:
__intcap global_intcap_fn_plus_two_intcap_tocap = (__intcap)(__cheri_tocap char *__capability)((char *)&extern_fn + 2);
// HYBRID-NEXT: @global_intcap_fn_plus_two_intcap_tocap = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr (i8, i8* bitcast (void ()* @extern_fn to i8*), i64 2) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_intcap_fn_plus_two_intcap_tocap = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* bitcast (void () addrspace(200)* @extern_fn to i8 addrspace(200)*), i64 2), align 16

/// These should warn since they are NULL-derived pointers in purecap, but be valid capabilities in hybrid:
char *__capability global_cap_data_addr_cast_long_one_past_end = (char *__capability)(__cheri_addr long)(&extern_data + 1);
// purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_addr_cast_long_one_past_end = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_cast_long_one_past_end = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1) to i64)), align 16
char *__capability global_cap_data_addr_cast_intcap_one_past_end = (char *__capability)(__cheri_addr __intcap)(&extern_data + 1);
// HYBRID-NEXT: @global_cap_data_addr_cast_intcap_one_past_end = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_addr_cast_intcap_one_past_end = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1) to i64)), align 16
char *__capability global_cap_data_tocap_one_past_end = (__cheri_tocap char *__capability)(char *)(__cheri_addr long)(&extern_data + 1);
// purecap-warning@-1{{cast from provenance-free integer type to pointer type will give pointer that can not be dereferenced}}
// HYBRID-NEXT: @global_cap_data_tocap_one_past_end = dso_local global i8 addrspace(200)* addrspacecast (i8* getelementptr inbounds (i8, i8* @extern_data, i64 1) to i8 addrspace(200)*), align 16
// PURECAP-NEXT: @global_cap_data_tocap_one_past_end = dso_local addrspace(200) global i8 addrspace(200)* getelementptr (i8, i8 addrspace(200)* null, i64 ptrtoint (i8 addrspace(200)* getelementptr inbounds (i8, i8 addrspace(200)* @extern_data, i64 1) to i64)), align 16

// HYBRID-EMPTY:
// PURECAP-EMPTY:
