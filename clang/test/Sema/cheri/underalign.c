// RUN: %cheri_cc1 -fsyntax-only -verify %s -target-abi purecap

__attribute__((aligned(2)))
typedef struct {
        void    *data;
        long   size;
} DBT; // expected-warning-re{{alignment (2) of 'DBT' is less than the required capability alignment ({{8|16|32}})}}

DBT x;

__attribute__((aligned(4)))
void    *data; // expected-warning-re{{alignment (4) of 'void * __capability' is less than the required capability alignment ({{8|16|32}})}}
