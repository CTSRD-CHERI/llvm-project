// RUN: %cheri256_cc1 "-target-abi" "purecap" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// RUN: %cheri128_cc1 "-target-abi" "purecap" -fsyntax-only -triple cheri-unknown-freebsd %s -verify
// expected-no-diagnostics
_Pragma("pointer_interpretation push")
_Pragma("pointer_interpretation integer")
struct foo {
	void *a;  // Should be interpreted as a long
	long d;
	long e;
};
struct foo si;
void *a;  // Should be interpreted as a long
_Pragma("pointer_interpretation pop")
struct bar {
	void *a;
	long d;
	long e;
};
void *b;
_Static_assert(sizeof(a) == 8, "Pointer size incorrect");
_Static_assert(sizeof(b) == _MIPS_SZCAP / 8, "Pointer size incorrect");
_Static_assert(sizeof(si) == 24, "Pointer size incorrect");
_Static_assert(sizeof(struct bar) == 2 * _MIPS_SZCAP / 8, "Pointer size incorrect");

// RUN: %cheri256_cc1 "-target-abi" "purecap" -fsyntax-only -triple cheri-unknown-freebsd %s -ast-dump | FileCheck %s
// RUN: %cheri128_cc1 "-target-abi" "purecap" -fsyntax-only -triple cheri-unknown-freebsd %s -ast-dump | FileCheck %s
// CHECK:      |-RecordDecl {{.*}} <{{.*}}/cheri-pointer-sizeof.c:6:1, line:10:1> line:6:8 struct foo definition
// CHECK-NEXT: | |-FieldDecl {{.*}} <line:7:2, col:8> col:8 a 'long'
// CHECK-NEXT: | |-FieldDecl {{.*}} <line:8:2, col:7> col:7 d 'long'
// CHECK-NEXT: | `-FieldDecl {{.*}} <line:9:2, col:7> col:7 e 'long'
// CHECK-NEXT: |-VarDecl {{.*}} <line:11:1, col:12> col:12 referenced si 'struct foo':'struct foo'
// CHECK-NEXT: |-VarDecl {{.*}} <line:12:1, col:7> col:7 referenced a 'long'
// CHECK-NEXT: |-RecordDecl {{.*}} <line:14:1, line:18:1> line:14:8 struct bar definition
// CHECK-NEXT: | |-FieldDecl {{.*}} <line:15:2, col:8> col:8 a 'void * __capability'
// CHECK-NEXT: | |-FieldDecl {{.*}} <line:16:2, col:7> col:7 d 'long'
// CHECK-NEXT: | `-FieldDecl {{.*}} <line:17:2, col:7> col:7 e 'long'
// CHECK-NEXT: |-VarDecl {{.*}} <line:19:1, col:7> col:7 referenced b 'void * __capability'
