// RUN: %clang -cc1 -triple cheri-unknown-bsd -cheri-linker -target-abi purecap -O2 -S -o - %s | FileCheck %s
// REQUIRES: clang
// ModuleID = 'ocsp_cl.i'
// GVN was seen to unconditionally get the size of the underlying type for a
// pointer when it was unsized.
// This needs to be as C source rather than LLVM IR, since the assertion that
// was hit with this test case is reached when generating the IR itself.
typedef struct { void *qualifiers; } POLICYINFO;
struct SOMETHING_ELSE *fn2();
POLICYINFO *a;
POLICYINFO *b;
typedef struct stack_st stack_st;
void sk_push(stack_st *);
// CHECK-LABEL: fn1:
void fn1() {
  // Call fn2
  // CHECK: ld	$1, %call16(fn2)($gp)
  // CHECK: cgetpccsetoffset	$c12, $1
  // CHECK: cjalr	$c12, $c17
  // CHECK: nop
  // Load address of a
  // CHECK-NEXT: ld $1, %got_disp(.size.a)($gp)
  // CHECK-NEXT: ld	$2, %got_disp(a)($gp)
  // CHECK-NEXT: ld $1, 0($1)
  // CHECK-NEXT: cfromptr	$c1, $c0, $2
  // CHECK-NEXT: csetbounds  [[A_CAP:\$c[0-9]+]], $c1, $1
  // Store in a
  // CHECK-NEXT: csc	$c3, $zero, 0([[A_CAP]])
  a = (POLICYINFO *__capability)fn2();
  // Load address of b
  // CHECK-NEXT: ld $1, %got_disp(.size.b)($gp)
  // CHECK-NEXT: ld	$2, %got_disp(b)($gp)
  // CHECK-NEXT: ld $1, 0($1)
  // CHECK-NEXT: cfromptr	$c1, $c0, $2
  // CHECK-NEXT: csetbounds  [[B_CAP:\$c[0-9]+]], $c1, $1
  // Store in b
  // CHECK-NEXT: csc	$c3, $zero, 0([[B_CAP]])
  b = a;
  // Load qualifiers
  // CHECK-NEXT: clc	[[B:\$c[0-9]+]], $zero, 0($c3)
  // Create NULL capability
  // CHECK-NEXT: cfromptr  [[NULLPTR:\$c[0-9]+]], $c0, $zero
  // Check if qualifiers is NULL
  // CHECK-NEXT: ceq	$1, [[B]], [[NULLPTR]]
  // CHECK: bnez	$1, .LBB0_2
  if (b->qualifiers)
    // Store above NULL capability in qualifiers
    // CHECK: csc	[[NULLPTR]], $zero, 0($c3)
    b->qualifiers = 0;
  // CHECK-LABEL: .LBB0_2:
  // Call sk_push
  // CHECK: ld	$1, %call16(sk_push)($gp)
  // CHECK: cgetpccsetoffset	$c12, $1
  // CHECK: cjalr	$c12, $c17
  sk_push((stack_st *)b->qualifiers);
}
