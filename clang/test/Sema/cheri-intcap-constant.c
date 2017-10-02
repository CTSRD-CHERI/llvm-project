// RUN: %cheri_cc1 -fsyntax-only -triple cheri-unknown-freebsd %s -std=c11 -verify
// RUN: %cheri_purecap_cc1 -fsyntax-only -triple cheri-unknown-freebsd %s -std=c11 -verify
// expected-no-diagnostics


int i;

static const void* foo = &i;
static const void* foo1 = (void*)&i;
// Casting via uintptr_t should not be an error as this is a common pattern (at least in C) to remove const/volatile
static const void* foo3 = (void*)(__intcap_t)&i;
static const void* foo4 = (void*)(__intcap_t)(const void*)&i;

#ifndef __CHERI_PURE_CAPABILITY__
static const void* foo2 = (void*)(long)&i;
#endif
