// Check that __attribute__((__aligned__)) aligns to sizeof(void* __capability) even for Cheri256
// RUN: %cheri128_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 %s -verify -emit-llvm -o - | FileCheck '-D$CAP_SIZE=16' '-D$GLOBALS_AS=' %s
// RUN: %cheri256_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 %s -verify -emit-llvm -o - | FileCheck '-D$CAP_SIZE=32' '-D$GLOBALS_AS=' %s
// RUN: %cheri128_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 %s -verify -emit-llvm -o - | FileCheck '-D$CAP_SIZE=16' '-D$GLOBALS_AS=addrspace(200) ' %s
// RUN: %cheri256_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -std=c11 %s -verify -emit-llvm -o - | FileCheck '-D$CAP_SIZE=32' '-D$GLOBALS_AS=addrspace(200) ' %s

struct no_caps {
  int i;
} __attribute__((__aligned__));
struct no_caps a;
// CHECK: @a = common [[$GLOBALS_AS]]global %struct.no_caps zeroinitializer, align [[$CAP_SIZE]]

struct with_cap {
  int *__capability i;
} __attribute__((__aligned__));
struct with_cap b;
// CHECK: @b = common [[$GLOBALS_AS]]global %struct.with_cap zeroinitializer, align [[$CAP_SIZE]]

struct with_cap_no_attr {
  int *__capability i;
};
struct with_cap_no_attr c;
// CHECK: @c = common [[$GLOBALS_AS]]global %struct.with_cap_no_attr zeroinitializer, align [[$CAP_SIZE]]

typedef struct {
  int i;
} __attribute__((__aligned__(2))) no_caps_td; // can only increase alignment
no_caps_td a_td;
typedef struct {
  int *__capability i;
} __attribute__((__aligned__(2))) with_cap_td;
with_cap_td b_td;
// CHECK: @a_td = common [[$GLOBALS_AS]]global %struct.no_caps_td zeroinitializer, align 4
// CHECK: @b_td = common [[$GLOBALS_AS]]global %struct.with_cap_td zeroinitializer, align [[$CAP_SIZE]]


// This sets alignment
typedef __attribute__((__aligned__(2))) struct { int i; } no_caps_td2;
no_caps_td2 a_td2;
typedef __attribute__((__aligned__(2))) struct { int *__capability i; } with_cap_td2; // expected-warning{{alignment (2) of 'with_cap_td2' is less than the required capability alignment}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
with_cap_td2 b_td2;
// CHECK: @a_td2 = common [[$GLOBALS_AS]]global %struct.no_caps_td2 zeroinitializer, align 2
// CHECK: @b_td2 = common [[$GLOBALS_AS]]global %struct.with_cap_td2 zeroinitializer, align 2

// annotation on a variable also sets alignemnt
__attribute__((__aligned__(2))) struct { int i; } no_caps_global_align2;
__attribute__((__aligned__(2))) struct { int *__capability i; } with_cap_global_align2; // expected-warning-re{{alignment (2) of 'struct (anonymous struct at {{.+}}' is less than the required capability alignment}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
// CHECK: @no_caps_global_align2 = common [[$GLOBALS_AS]]global %struct.anon zeroinitializer, align 2
// CHECK: @with_cap_global_align2 = common [[$GLOBALS_AS]]global %struct.anon.0 zeroinitializer, align 2

// Without an argument attribute aligned should be max(CAP_SIZE, 16)
__attribute__((__aligned__)) struct { int i; } no_caps_global_default_align;
__attribute__((__aligned__)) struct { int *__capability i; } with_cap_global_default_align;
// CHECK: @no_caps_global_default_align = common [[$GLOBALS_AS]]global %struct.anon.1 zeroinitializer, align [[$CAP_SIZE]]
// CHECK: @with_cap_global_default_align = common [[$GLOBALS_AS]]global %struct.anon.2 zeroinitializer, align [[$CAP_SIZE]]

// same for __BIGGEST_ALIGNENT__
_Static_assert(__BIGGEST_ALIGNMENT__ == sizeof(void *__capability), "__BIGGEST_ALIGNMENT__ wrong?");
__attribute__((__aligned__(__BIGGEST_ALIGNMENT__))) struct { int i; } no_caps_global_biggest_align;
__attribute__((__aligned__(__BIGGEST_ALIGNMENT__))) struct { int *__capability i; } with_cap_global_biggest_align;
// CHECK: @no_caps_global_biggest_align = common [[$GLOBALS_AS]]global %struct.anon.3 zeroinitializer, align [[$CAP_SIZE]]
// CHECK: @with_cap_global_biggest_align = common [[$GLOBALS_AS]]global %struct.anon.4 zeroinitializer, align [[$CAP_SIZE]]
