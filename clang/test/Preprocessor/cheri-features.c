// RUN: %cheri_clang -fsyntax-only -target cheri-unknown-freebsd11 %s \
// RUN:   2>&1 | FileCheck --check-prefix=CHECK-CHERI %s

// RUN: %clang -fsyntax-only -target mips64-unknown-freebsd11 %s \
// RUN:   2>&1 | FileCheck --check-prefix=CHECK-MIPS %s


#if __has_feature(cheri)
#pragma message("__has_feature(cheri)")
// CHECK-CHERI: warning: __has_feature(cheri)
void* __capability w = 0;
#else
#pragma message("no cheri")
// CHECK-MIPS: warning: no cheri
#endif

#if __has_feature(capabilities)
#pragma message("__has_feature(capabilities)")
// CHECK-CHERI: warning: __has_feature(capabilities)
void* __capability x = 0;
#else
#pragma message("no capabilities")
// CHECK-MIPS: warning: no capabilities
#endif

#if defined(__CHERI__)
#pragma message("__CHERI__ defined")
// CHECK-CHERI: warning: __CHERI__ defined
void* __capability y = 0;
#else
#pragma message("__CHERI__ not defined")
// CHECK-MIPS: warning: __CHERI__ not defined
#endif

int main() {
	return 0;
}
