// RUN: %cheri_purecap_cc1 -Wno-incompatible-pointer-types -fsyntax-only %s -verify=default
// RUN: %cheri_purecap_cc1 -Wno-incompatible-pointer-types -Wall -fsyntax-only %s -verify=default
// RUN: %cheri_purecap_cc1 -Wno-incompatible-pointer-types -Wextra -fsyntax-only %s -verify=strict,default
// RUN: %cheri_purecap_cc1 -Weverything -Wno-incompatible-pointer-types -Wno-strict-prototypes -Wdeprecated-non-prototype -Wcheri-prototypes -fsyntax-only %s -verify=default,strict
// RUN: %cheri_purecap_cc1 -Wno-incompatible-pointer-types -Wcheri -Wcheri-prototypes-strict -fsyntax-only %s -verify=default,strict
// RUN: %cheri_purecap_cc1 -Wno-incompatible-pointer-types -Wno-cheri -Wno-deprecated-non-prototype -fsyntax-only %s -verify=cheri-off
// cheri-off-no-diagnostics

extern void variadic(int, ...); //default-note{{'variadic' declared here}}
extern void noproto();
extern void proto(int); //strict-note{{'proto' declared here}}

static void *(*ptr)() = variadic; // default-error{{converting variadic function type}}
// default-note@-1{{read uninitialized values from the argument registers}}
// no warnings by default:
static void *(*ptr1)() = noproto;
static void *(*ptr2)() = proto; // strict-warning{{converting function type 'void (int)' to function pointer without prototype}}
// strict-note@-1{{read uninitialized values from the argument registers}}

// TODO: C++/C20 should also be handled (there is no no-proto case but the others should still warn)

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

extern int noarg_fn(void);
extern int noproto_fn();                   // default-note {{'noproto_fn' declared here}} // strict-note 3 {{'noproto_fn' declared here}}
extern int intarg_fn(int);                 // default-note {{'intarg_fn' declared here}} // strict-note 2 {{'intarg_fn' declared here}}
extern int ptrarg_fn(const char *);        // default-note {{'ptrarg_fn' declared here}} // strict-note 2 {{'ptrarg_fn' declared here}}
extern int variadic_fn(const char *, ...); // default-note 5 {{'variadic_fn' declared here}}

static void assign_functions(struct Foo *f) {
  f->noproto = noproto_fn;  // fine, these are the same  (and will be caught be -Wstrict-prototypes)
  f->noproto = variadic_fn; // default-error{{converting variadic function type 'int (const char *, ...)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (*)()') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->noproto = noarg_fn;  // noarg is fine since there is no calling convention confusion
  f->noproto = intarg_fn; // strict-warning{{converting function type 'int (int)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (*)()') may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  f->noproto = ptrarg_fn; // strict-warning{{converting function type 'int (const char *)' to function pointer without prototype 'NoProtoFnTy' (aka 'int (*)()') may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}

  f->variadic = variadic_fn;
  f->variadic = noarg_fn;
  f->variadic = intarg_fn; // default-error{{converting function type 'int (int)' to variadic function pointer 'VariadicFnTy' (aka 'int (*)(const char *, ...)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->variadic = ptrarg_fn; // default-error{{converting function type 'int (const char *)' to variadic function pointer 'VariadicFnTy' (aka 'int (*)(const char *, ...)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->variadic = noproto_fn; // default-error{{converting function type without prototype 'int ()' to variadic function pointer 'VariadicFnTy' (aka 'int (*)(const char *, ...)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}

  f->noarg = variadic_fn; // default-error{{converting variadic function type 'int (const char *, ...)' to function pointer 'NoArgFnTy' (aka 'int (*)(void)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->noarg = noarg_fn;
  f->noarg = intarg_fn;
  f->noarg = ptrarg_fn;
  f->noarg = noproto_fn; // strict-warning{{converting function type without prototype 'int ()' to function pointer 'NoArgFnTy' (aka 'int (*)(void)') may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}

  f->intarg = variadic_fn; // default-error{{converting variadic function type 'int (const char *, ...)' to function pointer 'IntArgFnTy' (aka 'int (*)(int)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->intarg = noarg_fn;
  f->intarg = intarg_fn;
  f->intarg = ptrarg_fn;
  f->intarg = noproto_fn; // strict-warning{{converting function type without prototype 'int ()' to function pointer 'IntArgFnTy' (aka 'int (*)(int)') may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}

  f->ptrarg = variadic_fn; // default-error{{converting variadic function type 'int (const char *, ...)' to function pointer 'PtrArgFnTy' (aka 'int (*)(const char *)') may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  // incompatible function pointer types but that is handled by a different warning:
  f->ptrarg = noarg_fn;
  f->ptrarg = intarg_fn;
  f->ptrarg = ptrarg_fn;
  f->ptrarg = noproto_fn; // strict-warning{{converting function type without prototype 'int ()' to function pointer 'PtrArgFnTy' (aka 'int (*)(const char *)') may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
}

static void assign_fnptr_noproto(struct Foo *f,
                                 NoProtoFnTy NoProtoFnTy_arg,
                                 VariadicFnTy VariadicFnTy_arg,
                                 PtrArgFnTy PtrArgFnTy_arg,
                                 NoArgFnTy NoArgFnTy_arg,
                                 IntArgFnTy IntArgFnTy_arg) {
  f->noproto = NoProtoFnTy_arg;
  f->noproto = VariadicFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->noproto = NoArgFnTy_arg;
  f->noproto = IntArgFnTy_arg; // strict-warning{{may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  f->noproto = PtrArgFnTy_arg; // strict-warning{{may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}

  f->variadic = NoProtoFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->variadic = VariadicFnTy_arg;
  f->variadic = NoArgFnTy_arg;
  f->variadic = IntArgFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->variadic = PtrArgFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}

  f->noarg = NoProtoFnTy_arg; // strict-warning{{may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  f->noarg = VariadicFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->noarg = NoArgFnTy_arg;
  f->noarg = IntArgFnTy_arg;
  f->noarg = PtrArgFnTy_arg;

  f->intarg = NoProtoFnTy_arg; // strict-warning{{may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  f->intarg = VariadicFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->intarg = NoArgFnTy_arg;
  f->intarg = IntArgFnTy_arg;
  f->intarg = PtrArgFnTy_arg;

  f->ptrarg = NoProtoFnTy_arg; // strict-warning{{may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  f->ptrarg = VariadicFnTy_arg; // default-error{{may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
  f->ptrarg = NoArgFnTy_arg;
  f->ptrarg = IntArgFnTy_arg;
  f->ptrarg = PtrArgFnTy_arg;
}

extern void noproto_callback(int arg, void (*callback)());

static void test_callback(int arg) {
  // TODO: it would be nice if we could say where the function we are calling was declared
  noproto_callback(arg, noarg_fn);
  noproto_callback(arg, intarg_fn); // strict-warning{{converting function type 'int (int)' to function pointer without prototype 'void (*)()' may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  noproto_callback(arg, ptrarg_fn); // strict-warning{{converting function type 'int (const char *)' to function pointer without prototype 'void (*)()' may cause incorrect parameters to be passed at run time}}
  // strict-note@-1{{read uninitialized values from the argument registers}}
  noproto_callback(arg, variadic_fn); // default-error{{converting variadic function type 'int (const char *, ...)' to function pointer without prototype 'void (*)()' may cause incorrect parameters to be passed at run time}}
  // default-note@-1{{read uninitialized values from the argument registers}}
}

static void *kr(void *arg);
static void *kr(arg) void *arg; // default-warning {{a function definition without a prototype is deprecated in all versions of C and is not supported in C2x}}
{
  return arg;
}

static void *(*ptr3)(void *) = kr;         // this should not warn!
static void *(*ptr3_addrof)(void *) = &kr; // this should not warn!

static void *kr2(); // default-warning {{a function declaration without a prototype is deprecated in all versions of C and is treated as a zero-parameter prototype in C2x, conflicting with a subsequent definition}}
static void *kr2(arg) void *arg;  // default-warning {{a function definition without a prototype is deprecated}}
{
  return arg;
}

static void *(*ptr4)(void *) = kr2;         // this should not warn!
static void *(*ptr4_addrof)(void *) = &kr2; // this should not warn!

extern void *kr3();              //strict-note 2 {{'kr3' declared here}}
static void *(*ptr5)(int) = kr3; // strict-warning {{converting function type without prototype 'void *()' to function pointer 'void *(*)(int)' may cause incorrect parameters to be passed at run time}}
// strict-note@-1{{read uninitialized values from the argument registers}}
static void *(*ptr5_addrof)(int) = &kr3; // strict-warning {{converting function type without prototype 'void *(*)()' to function pointer 'void *(*)(int)' may cause incorrect parameters to be passed at run time}}
                                         // strict-note@-1{{read uninitialized values from the argument registers}}
