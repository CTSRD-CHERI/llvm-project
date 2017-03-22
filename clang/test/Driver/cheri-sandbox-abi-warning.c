// RUN: %clang -target cheri-unknown-freebsd -mabi=sandbox -fsyntax-only %s 2>&1 | FileCheck %s
// CHECK: warning: CHERI ABI 'sandbox' is deprecated; use 'purecap' instead
