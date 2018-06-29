// RUN: %cheri_cc1 -std=c11 -fsyntax-only -verify %s -target-abi purecap
// RUN: %cheri_cc1 -std=c11 -fsyntax-only %s -w -target-abi purecap -emit-llvm -DSKIP_ERRORS -o - | %cheri_FileCheck %s -check-prefix IR

// On a variable declaration __attribute__((aligned(4))) sets the alignment to 4
__attribute__((aligned(4))) void *data; // expected-warning-re{{alignment (4) of 'void * __capability' is less than the required capability alignment ({{8|16|32}})}}
// IR: @data = common addrspace(200) global i8 addrspace(200)* null, align 4

// If __attribute__((aligned(2))) is applied to typedefs it sets the alignment to 4
__attribute__((aligned(2)))
typedef struct {
        void    *data;
        long   size;
} DBT; // expected-warning-re{{alignment (2) of 'DBT' is less than the required capability alignment ({{8|16|32}})}}

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


#ifndef SKIP_ERRORS
// _Alignas gives sensible errors:
_Alignas(4) void *data2; // expected-error-re{{requested alignment is less than minimum alignment of {{8|16|32}} for type 'void * __capability'}}
// expected-warning-re@-1{{alignment (4) of 'void * __capability' is less than the required capability alignment ({{8|16|32}})}}
#endif
