// RUN: %cheri_cc1 -target-abi n64 -std=gnu99 -verify %s
// expected-no-diagnostics

// This code used to crash because TypedefNameDecl::CHERICapTypeForDecl was uninitialized
typedef void* voidptr;
typedef __capability voidptr voidcap;
typedef voidptr __capability voidcap2;
