// RUN: %cheri_cc1 -std=c11 -fsyntax-only -verify %s -target-abi purecap
// RUN: %cheri_cc1 -std=c11 -fsyntax-only %s -w -target-abi purecap -emit-llvm -DSKIP_ERRORS -o - | %cheri_FileCheck %s -check-prefix IR

// On a variable declaration __attribute__((aligned(4))) sets the alignment to 4
__attribute__((aligned(4))) void *data; // expected-warning-re{{alignment (4) of 'void * __capability' is less than the required capability alignment ({{8|16|32}})}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
// IR: @data = common addrspace(200) global i8 addrspace(200)* null, align 4

// If __attribute__((aligned(2))) is applied to typedefs it sets the alignment to 4
__attribute__((aligned(2)))
typedef struct {
        void    *data;
        long   size;
} DBT; // expected-warning-re{{alignment (2) of 'DBT' is less than the required capability alignment ({{8|16|32}})}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}

DBT dbt;
// IR: @dbt = common addrspace(200) global %struct.DBT zeroinitializer, align 2

// However, when used on a record declaration it actually just sets the minimum alignment. If the type
// has a larger natural alignment this will generate
struct DBT2 {
        void    *data;
        long   size;
} __attribute__((aligned(2)));  // this should not warn!

struct DBT2 dbt2;
// IR: @dbt2 = common addrspace(200) global %struct.DBT2 zeroinitializer, align [[$CAP_SIZE]]

// We should also be warning if it has the packed attribute:
struct DBT3 {  // expected-warning-re{{alignment (4) of 'struct DBT3' is less than the required capability alignment ({{8|16|32}})}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
        void    *data;
 } __attribute__((packed, aligned(4)));

struct DBT3 dbt3;
// IR: @dbt3 = common addrspace(200) global %struct.DBT3 zeroinitializer, align 4

// Furthermore we should also be able to silence the warning:
// Currently this is done  using the annotate("underaligned_capability") attribute
struct DBT4 {
        void    *data;
} __attribute__((packed, aligned(4), annotate("underaligned_capability")));
struct DBT4 dbt4;
// Check that only the specified string value is valid:
// IR: @dbt4 = common addrspace(200) global %struct.DBT4 zeroinitializer, align 4
struct DBT5 { // expected-warning-re{{alignment (4) of 'struct DBT5' is less than the required capability alignment ({{8|16|32}})}}
// expected-note@-1{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
        void    *data;
} __attribute__((packed, aligned(4), annotate("underaligned_capability_typo")));
struct DBT5 dbt5;
// IR: @dbt5 = common addrspace(200) global %struct.DBT5 zeroinitializer, align 4


// __BIGGEST_ALIGNMENT__ should be fine even for CHERI256
struct DBT6 {
        void    *data;
 } __attribute__((packed, aligned(__BIGGEST_ALIGNMENT__)));
_Static_assert(__BIGGEST_ALIGNMENT__ == sizeof(void* __capability), "__BIGGEST_ALIGNMENT__ wrong?");
struct DBT6 dbt6;
// IR: @dbt6 = common addrspace(200) global %struct.DBT6 zeroinitializer, align [[$CAP_SIZE]]

#ifndef SKIP_ERRORS
// _Alignas gives sensible errors:
_Alignas(4) void *data2; // expected-error-re{{requested alignment is less than minimum alignment of {{8|16|32}} for type 'void * __capability'}}
// expected-warning-re@-1{{alignment (4) of 'void * __capability' is less than the required capability alignment ({{8|16|32}})}}
// expected-note@-2{{If you are certain that this is correct you can silence the warning by adding __attribute__((annotate("underaligned_capability")))}}
#endif
