// Check that __attribute__((__aligned__)) aligns to at least sizeof(void* __capability) even for Cheri256
// RUN: %riscv32_cheri_cc1 -std=c11 -verify -emit-llvm -o - %s \
// RUN:  | FileCheck '-D#CAP_SIZE=8' '-D#BIG_ALIGN=16' '-D$GLOBALS_AS=' %s
// RUN: %cheri_cc1 -std=c11 -verify -emit-llvm -o - %s \
// RUN:  | FileCheck '-D#CAP_SIZE=16' '-D#BIG_ALIGN=16' '-D$GLOBALS_AS=' %s
// RUN: %riscv32_cheri_purecap_cc1 -std=c11 -verify -emit-llvm -o - %s \
// RUN:  | FileCheck '-D#CAP_SIZE=8' '-D#BIG_ALIGN=16' '-D$GLOBALS_AS=addrspace(200) ' %s
// RUN: %cheri_purecap_cc1 -std=c11 -verify -emit-llvm -o - %s \
// RUN:  | FileCheck '-D#CAP_SIZE=16' '-D#BIG_ALIGN=16' '-D$GLOBALS_AS=addrspace(200) ' %s

struct with_cap_no_attr {
  int *__capability i;
};
struct with_cap_no_attr c;
// CHECK: @c = dso_local [[$GLOBALS_AS]]global %struct.with_cap_no_attr zeroinitializer, align [[#CAP_SIZE]]

typedef struct {
  int i;
} __attribute__((__aligned__(2))) no_caps_td; // can only increase alignment
no_caps_td a_td;
typedef struct {
  int *__capability i;
} __attribute__((__aligned__(2))) with_cap_td;
with_cap_td b_td;
// CHECK: @a_td = dso_local [[$GLOBALS_AS]]global %struct.no_caps_td zeroinitializer, align 4
// CHECK: @b_td = dso_local [[$GLOBALS_AS]]global %struct.with_cap_td zeroinitializer, align [[#CAP_SIZE]]

// This sets alignment
typedef __attribute__((__aligned__(2))) struct { int i; } no_caps_td2;
no_caps_td2 a_td2;
typedef __attribute__((__aligned__(2))) struct { int *__capability i; } with_cap_td2; // expected-warning{{alignment (2) of 'with_cap_td2' is less than the required capability alignment}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
with_cap_td2 b_td2;
// CHECK: @a_td2 = dso_local [[$GLOBALS_AS]]global %struct.no_caps_td2 zeroinitializer, align 2
// CHECK: @b_td2 = dso_local [[$GLOBALS_AS]]global %struct.with_cap_td2 zeroinitializer, align 2

// annotation on a variable also sets alignemnt
__attribute__((__aligned__(2))) struct { int i; } no_caps_global_align2;
__attribute__((__aligned__(2))) struct { int *__capability i; } with_cap_global_align2; // expected-warning-re{{alignment (2) of 'struct (anonymous struct at {{.+}}' is less than the required capability alignment}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
// CHECK: @no_caps_global_align2 = dso_local [[$GLOBALS_AS]]global %struct.anon zeroinitializer, align 2
// CHECK: @with_cap_global_align2 = dso_local [[$GLOBALS_AS]]global %struct.anon.0 zeroinitializer, align 2

// But not if the location of the annotation is after the struct decl
struct { int i; } __attribute__((__aligned__(2))) no_caps_global_align3;
struct { int *__capability i; } __attribute__((__aligned__(2))) with_cap_global_align3;
// CHECK: @no_caps_global_align3 = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align 4
// CHECK: @with_cap_global_align3 = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align [[#CAP_SIZE]]

// Without an argument attribute aligned should be max(CAP_SIZE, 16)
__attribute__((__aligned__)) struct { int i; } no_caps_global_default_align;
__attribute__((__aligned__)) struct { int *__capability i; } with_cap_global_default_align;
// CHECK: @no_caps_global_default_align = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align [[#BIG_ALIGN]]
// CHECK: @with_cap_global_default_align = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align [[#BIG_ALIGN]]

struct no_caps {
  int i;
} __attribute__((__aligned__));
struct no_caps a;
// CHECK: @a = dso_local [[$GLOBALS_AS]]global %struct.no_caps zeroinitializer, align [[#BIG_ALIGN]]

struct with_cap {
  int *__capability i;
} __attribute__((__aligned__));
struct with_cap b;
// CHECK: @b = dso_local [[$GLOBALS_AS]]global %struct.with_cap zeroinitializer, align [[#BIG_ALIGN]]

// same for __BIGGEST_ALIGNENT__
_Static_assert(__BIGGEST_ALIGNMENT__ == sizeof(void *__capability) < 16 ? 16 : sizeof(void *__capability), "__BIGGEST_ALIGNMENT__ wrong?");
__attribute__((__aligned__(__BIGGEST_ALIGNMENT__))) struct { int i; } no_caps_global_biggest_align;
__attribute__((__aligned__(__BIGGEST_ALIGNMENT__))) struct { int *__capability i; } with_cap_global_biggest_align;
// CHECK: @no_caps_global_biggest_align = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align [[#BIG_ALIGN]]
// CHECK: @with_cap_global_biggest_align = dso_local [[$GLOBALS_AS]]global %struct.anon.{{[0-9]+}} zeroinitializer, align [[#BIG_ALIGN]]
