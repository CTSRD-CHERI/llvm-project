// RUN: %cheri_cc1 -emit-obj -disable-free -main-file-name hash.c -mrelocation-model pic -pic-level 2 -mthread-model posix -mdisable-fp-elim -masm-verbose -mconstructor-aliases -target-cpu cheri -target-feature +soft-float -target-abi n64 -msoft-float -mfloat-abi soft -O2 -vectorize-loops -vectorize-slp -cheri-linker -x c %s -o - | FileCheck %s

// XFAIL: *

typedef struct a { struct a * __capability b; } * c;
void d(void) {
  c __capability *e;
  e = &(*e)->b;
}
