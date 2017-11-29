// RUN: %cheri_clang -mabi=sandbox -fsyntax-only %s 2>&1 | FileCheck --check-prefix=WARN %s
// RUN: %cheri_clang -### -mabi=sandbox %s 2>&1 | FileCheck --check-prefix=CHERILIBS %s
// WARN: warning: CHERI ABI 'sandbox' is deprecated; use 'purecap' instead
// CHERILIBS: libcheri
