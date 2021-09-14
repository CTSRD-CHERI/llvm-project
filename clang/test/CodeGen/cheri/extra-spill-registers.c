// UNSUPPORTED: true
// RUNs: %clang -S -mcpu=cheri128 -mabi=purecap -target mips64-unknown-freebsd %s -o - -O2 \
// RUNs:    -mllvm -stop-before=mips-delay-slot-filler -mllvm -debug-only="regalloc,scheduler" -mllvm -stats
// RUN: %clang -S -mcpu=cheri128 -mabi=n64 -target mips64-unknown-freebsd %s -o - -O2 \
// RUN:    -mllvm -stop-before=mips-delay-slot-filler -mllvm -debug-only="regalloc,cheduler" -mllvm -stats
// RUN: false
// RUNs: %clang -S -mcpu=mips4 -target mips64-unknown-freebsd %s -o %t.mips4 -O2 -mllvm -stop-before=expand-isel-pseudos
// RUNs: %clang -S -mcpu=beri -target mips64-unknown-freebsd %s -o %t.beri -O2
// RUNs: diff -u %t.mips4  %t.beri
// RUNs: cat %t.beri
// RUN: %clang -S -mcpu=cheri128 -mabi=purecap -target mips64-unknown-freebsd %s -o -  -O2 -mllvm -print-regusage
// -mllvm -debug-only=regalloc

void *get_ptr(void);
long get_long(void);
void force_callee_save_spills(void);

void use_ptrs(void *, void *, void *, void *, void *, void *, void *, void *);
void use_longs(long, long, long, long, long, long, long, long);

void test(void) {
  void *p1 = get_ptr();
  void *p2 = get_ptr();
  void *p3 = get_ptr();
  void *p4 = get_ptr();
  void *p5 = get_ptr();
  void *p6 = get_ptr();
  void *p7 = get_ptr();
  void *p8 = get_ptr();

  long l1 = get_long();
  long l2 = get_long();
  long l3 = get_long();
  long l4 = get_long();
  long l5 = get_long();
  long l6 = get_long();
  long l7 = get_long();
  long l8 = get_long();

  force_callee_save_spills();

  use_ptrs(p1, p2, p3, p4, p5, p6, p7, p8);
  use_longs(l1, l2, l3, l4, l5, l6, l7, l8);
}
