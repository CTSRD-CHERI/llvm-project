// RUN: %cheri_cc1 -S -no-struct-path-tbaa -target-abi n64 -O2 -o -  %s
// REQUIRES: asserts
// RUN: not --crash %cheri_cc1 -emit-llvm -new-struct-path-tbaa -target-abi n64 -O2 -o -  %s
// RUN: not --crash %cheri_cc1 -S -new-struct-path-tbaa -target-abi n64 -O2 -o -  %s
long a;
extern int c(void);

void b() {
  if (*(unsigned *)a)
    c();
}
