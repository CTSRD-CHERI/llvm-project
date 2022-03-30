// RUN: %cheri_purecap_cc1 -fsyntax-only %s -std=c++11 -verify -ast-dump | FileCheck %s


struct foo_cap { // expected-note {{Add __attribute__((aligned(16))) to ensure sufficient alignment}}
	void *a; // expected-warning {{capability field at offset 0 in packed structure will trap if structure is used in an array}}
	long d;
	int e;
} __attribute__((packed,annotate("underaligned_capability")));  // make it 48 instead of 64
_Pragma("pointer_interpretation push")
_Pragma("pointer_interpretation integer")
// Exactly the same as above
struct foo_int {
	void *a;
	long d;
	int e;
};
_Static_assert(sizeof(void*) == 8, "Pointer size incorrect");
_Static_assert(sizeof(foo_int) == 24, "Pointer size incorrect");
_Static_assert(sizeof(foo_cap) == 28, "Pointer size incorrect");
// Check that we don't accidentally break sizeof() for references:
// When applied to a reference or a reference type, the result is the size of the referenced type.
_Static_assert(sizeof(int&) == 4, "Reference size incorrect");
_Static_assert(sizeof(int&&) == 4, "Reference size incorrect");
_Pragma("pointer_interpretation pop")
_Static_assert(sizeof(void*) == 16, "Pointer size incorrect");
_Static_assert(sizeof(foo_int) == 24, "Pointer size incorrect");
_Static_assert(sizeof(foo_cap) == 28, "Pointer size incorrect");
_Static_assert(sizeof(int&) == 4, "Reference size incorrect");
_Static_assert(sizeof(int&&) == 4, "Reference size incorrect");

// CHECK:     |-CXXRecordDecl {{.*}} <{{.*}}/cheri-pointer-interpretation.cpp:4:1, line:8:1> line:4:8 referenced struct foo_cap definition
// CHECK:     | |-PackedAttr {{.*}} <line:8:18>
// CHECK-NEXT:| |-AnnotateAttr {{.*}} <col:25, col:59> "underaligned_capability"
// CHECK-NEXT:| |-CXXRecordDecl {{.*}} <line:4:1, col:8> col:8 implicit struct foo_cap
// CHECK-NEXT:| |-FieldDecl {{.*}} <line:5:2, col:8> col:8 a 'void *'
// CHECK-NEXT:| |-FieldDecl {{.*}} <line:6:2, col:7> col:7 d 'long'
// CHECK-NEXT:| `-FieldDecl {{.*}} <line:7:2, col:6> col:6 e 'int'
// CHECK:     |-CXXRecordDecl {{.*}} <line:12:1, line:16:1> line:12:8 referenced struct foo_int definition
// CHECK:     | |-CXXRecordDecl {{.*}} <col:1, col:8> col:8 implicit struct foo_int
// CHECK-NEXT:| |-FieldDecl {{.*}} <line:13:2, col:8> col:8 a 'long'
// CHECK-NEXT:| |-FieldDecl {{.*}} <line:14:2, col:7> col:7 d 'long'
// CHECK-NEXT:| `-FieldDecl {{.*}} <line:15:2, col:6> col:6 e 'int'


