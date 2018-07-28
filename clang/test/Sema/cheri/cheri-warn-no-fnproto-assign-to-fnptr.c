// RUN: %cheri_purecap_cc1 -Weverything -Wno-incompatible-pointer-types -Wno-strict-prototypes -Werror=mips-cheri-prototypes -fsyntax-only %s -verify -fdiagnostics-show-option

// TODO: C++ should also be handled (there is no no-proto case but the others should still warn)

// Assigning a function with non-variadic calling convention to a variadic function
// pointer should give a warning
typedef int (*NoProtoFnTy)();
typedef int (*VariadicFnTy)(const char *, ...);
typedef int (*NoArgFnTy)(void);
typedef int (*IntArgFnTy)(int);
typedef int (*PtrArgFnTy)(const char *);

struct Foo {
  NoProtoFnTy noproto;
  VariadicFnTy variadic;
  NoArgFnTy noarg;
  IntArgFnTy intarg;
  PtrArgFnTy ptrarg;
};

extern int noproto_fn();  // expected-note 4 {{'noproto_fn' declared here}}
extern int noarg_fn(void);
extern int intarg_fn(int);  // expected-note 3 {{'intarg_fn' declared here}}
extern int ptrarg_fn(const char *);  // expected-note 3 {{'ptrarg_fn' declared here}}
extern int variadic_fn(const char *, ...);  // expected-note 5 {{'variadic_fn' declared here}}

static void assign_functions(struct Foo *f) {
  f->noproto = noproto_fn; // fine, these are the same  (and will be caught be -Wstrict-prototypes)
  f->noproto = variadic_fn; // expected-error{{converting variadic function type 'int (const char * __capability, ...)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (* __capability)()') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noproto = noarg_fn; // noarg is fine since there is no calling convention confusion
  f->noproto = intarg_fn; // expected-error{{converting function type 'int (int)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (* __capability)()') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noproto = ptrarg_fn; // expected-error{{converting function type 'int (const char * __capability)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (* __capability)()') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}

  f->variadic = noproto_fn; // expected-error{{converting function type without prototype 'int ()' to variadic function pointer 'VariadicFnTy' (aka 'int (* __capability)(const char * __capability, ...)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noarg = noproto_fn; // expected-error{{converting function type without prototype 'int ()' to function pointer 'NoArgFnTy' (aka 'int (* __capability)(void)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->intarg = noproto_fn; // expected-error{{converting function type without prototype 'int ()' to function pointer 'IntArgFnTy' (aka 'int (* __capability)(int)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->ptrarg = noproto_fn; // expected-error{{converting function type without prototype 'int ()' to function pointer 'PtrArgFnTy' (aka 'int (* __capability)(const char * __capability)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}


  f->variadic = variadic_fn;
  f->variadic = noarg_fn;
  f->variadic = intarg_fn; // expected-error{{converting function type 'int (int)' to variadic function pointer 'VariadicFnTy' (aka 'int (* __capability)(const char * __capability, ...)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->variadic = ptrarg_fn; // expected-error{{converting function type 'int (const char * __capability)' to variadic function pointer 'VariadicFnTy' (aka 'int (* __capability)(const char * __capability, ...)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}

  f->noarg = variadic_fn; // expected-error{{converting variadic function type 'int (const char * __capability, ...)' to function pointer 'NoArgFnTy' (aka 'int (* __capability)(void)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->noarg = noarg_fn;
  f->noarg = intarg_fn;
  f->noarg = ptrarg_fn;

  f->intarg = variadic_fn; // expected-error{{converting variadic function type 'int (const char * __capability, ...)' to function pointer 'IntArgFnTy' (aka 'int (* __capability)(int)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->intarg = noarg_fn;
  f->intarg = intarg_fn;
  f->intarg = ptrarg_fn;

  f->ptrarg = variadic_fn;  // expected-error{{converting variadic function type 'int (const char * __capability, ...)' to function pointer 'PtrArgFnTy' (aka 'int (* __capability)(const char * __capability)') may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->ptrarg = noarg_fn;
  f->ptrarg = intarg_fn;
  f->ptrarg = ptrarg_fn;
}

static void assign_fnptr_noproto(struct Foo *f,
                                 NoProtoFnTy NoProtoFnTy_arg,
                                 VariadicFnTy VariadicFnTy_arg,
                                 PtrArgFnTy PtrArgFnTy_arg,
                                 NoArgFnTy NoArgFnTy_arg,
                                 IntArgFnTy IntArgFnTy_arg) {
  f->noproto = NoProtoFnTy_arg;
  f->noproto = VariadicFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noproto = NoArgFnTy_arg;
  f->noproto = IntArgFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noproto = PtrArgFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}

  f->variadic = NoProtoFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->variadic = VariadicFnTy_arg;
  f->variadic = NoArgFnTy_arg;
  f->variadic = IntArgFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->variadic = PtrArgFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}

  f->noarg = NoProtoFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noarg = VariadicFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->noarg = NoArgFnTy_arg;
  f->noarg = IntArgFnTy_arg;
  f->noarg = PtrArgFnTy_arg;

  f->intarg = NoProtoFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->intarg = VariadicFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->intarg = NoArgFnTy_arg;
  f->intarg = IntArgFnTy_arg;
  f->intarg = PtrArgFnTy_arg;

  f->ptrarg = NoProtoFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->ptrarg = VariadicFnTy_arg; // expected-error{{may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  f->ptrarg = NoArgFnTy_arg;
  f->ptrarg = IntArgFnTy_arg;
  f->ptrarg = PtrArgFnTy_arg;
}
#if 0
// casts should not be able to silence this warning since it is almost always going to causes crashes at runtime
static void assign_functions_with_casts(struct Foo* f) {

}
#endif

extern void noproto_callback(int arg, void(*callback)());

static void test_callback(int arg) {
  // TODO: it would be nice if we could say where the function we are calling was declared
  noproto_callback(arg, noarg_fn);
  noproto_callback(arg, intarg_fn); // expected-error{{converting function type 'int (int)' to function pointer without prototype 'void (* __capability)()' may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  noproto_callback(arg, ptrarg_fn); // expected-error{{converting function type 'int (const char * __capability)' to function pointer without prototype 'void (* __capability)()' may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}
  noproto_callback(arg, variadic_fn); // expected-error{{converting variadic function type 'int (const char * __capability, ...)' to function pointer without prototype 'void (* __capability)()' may cause wrong parameters to be passed at run-time.}}
  // expected-note@-1{{will read garbage values from the argument registers}}

}
