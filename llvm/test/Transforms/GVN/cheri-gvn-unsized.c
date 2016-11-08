// RUN: clang -cc1 -triple cheri-unknown-bsd -cheri-linker -target-abi sandbox -O2 -S -o - %s | FileCheck %s
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
  // Load address of a
  // CHECK: ld	$2, %got_disp(a)($gp)
  // CHECK: cfromptr	$c1, $c0, $2
  // Store in a
  // CHECK: csc	$c3, $zero, 0($c1)
  a = (POLICYINFO *__capability)fn2();
  // Load address of b
  // CHECK: ld	$2, %got_disp(b)($gp)
  // CHECK: cfromptr	$c1, $c0, $2
  // Store in b
  // CHECK: csc	$c3, $zero, 0($c2)
  b = a;
  // Load qualifiers
  // CHECK: clc	$c1, $zero, 0($c3)
  // Create NULL capability
  // CHECK: cfromptr	$c4, $c0, $zero
  // Check if qualifiers is NULL
  // CHECK: ceq	$1, $c1, $c4
  // CHECK: bnez	$1, $BB0_2
  if (b->qualifiers)
    // Store above NULL capability in qualifiers
    // CHECK: csc	$c4, $zero, 0($c3)
    b->qualifiers = 0;
  // CHECK-LABEL: $BB0_2:
  // Call sk_push
  // CHECK: ld	$1, %call16(sk_push)($gp)
  // CHECK: cgetpccsetoffset	$c12, $1
  // CHECK: cjalr	$c12, $c17
  sk_push((stack_st *)b->qualifiers);
}
