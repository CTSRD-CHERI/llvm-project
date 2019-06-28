// RUN: %cheri_purecap_cc1 -std=c11 -emit-llvm -verify %s -o - | %cheri_FileCheck %s

// Check that the underalignment check doesn't crash when used on an incomplete type (when
// parsing the attribute on a fieldDecl)

struct {
  _Alignas(8) long a;
} a;
// CHECK: @a = common addrspace(200) global %struct.anon zeroinitializer, align 8
struct {
  long a __attribute__((__aligned__(2)));
} b1;
struct {
  long a __attribute__((packed, __aligned__(2)));
} b2;

struct {
  __intcap_t a __attribute__((packed, __aligned__(2))); // expected-warning{{alignment (2) of '__intcap_t' is less than the required capability alignment}}
  // expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
} c1;

// no warning without packed
struct {
  __intcap_t a __attribute__((__aligned__(2)));
} c2;

// without packed it can only increase alignment, but with packed it also decreases it:
// CHECK: @b1 = common addrspace(200) global %struct.anon.0 zeroinitializer, align 8
// CHECK: @b2 = common addrspace(200) global %struct.anon.1 zeroinitializer, align 2
// CHECK: @c1 = common addrspace(200) global %struct.anon.2 zeroinitializer, align 2
// CHECK: @c2 = common addrspace(200) global %struct.anon.3 zeroinitializer, align [[#CAP_SIZE]]
