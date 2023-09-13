#include "sail.h"
#include "rts.h"
#include "elf.h"

// enum write_kind
enum zwrite_kind { zWrite_plain, zWrite_conditional, zWrite_release, zWrite_exclusive, zWrite_exclusive_release, zWrite_RISCV_release, zWrite_RISCV_strong_release, zWrite_RISCV_conditional, zWrite_RISCV_conditional_release, zWrite_RISCV_conditional_strong_release, zWrite_X86_locked };

static bool eq_zwrite_kind(enum zwrite_kind op1, enum zwrite_kind op2) { return op1 == op2; }

static enum zwrite_kind UNDEFINED(zwrite_kind)(unit u) { return zWrite_plain; }

// enum trans_kind
enum ztrans_kind { zTransaction_start, zTransaction_commit, zTransaction_abort };

static bool eq_ztrans_kind(enum ztrans_kind op1, enum ztrans_kind op2) { return op1 == op2; }

static enum ztrans_kind UNDEFINED(ztrans_kind)(unit u) { return zTransaction_start; }

// enum signal
enum zsignal { zLOW, zHIGH };

static bool eq_zsignal(enum zsignal op1, enum zsignal op2) { return op1 == op2; }

static enum zsignal UNDEFINED(zsignal)(unit u) { return zLOW; }

// enum read_kind
enum zread_kind { zRead_plain, zRead_reserve, zRead_acquire, zRead_exclusive, zRead_exclusive_acquire, zRead_stream, zRead_ifetch, zRead_RISCV_acquire, zRead_RISCV_strong_acquire, zRead_RISCV_reserved, zRead_RISCV_reserved_acquire, zRead_RISCV_reserved_strong_acquire, zRead_X86_locked };

static bool eq_zread_kind(enum zread_kind op1, enum zread_kind op2) { return op1 == op2; }

static enum zread_kind UNDEFINED(zread_kind)(unit u) { return zRead_plain; }

// struct tuple_(%bv1, %bv1, %bv5, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4);
}


// struct tuple_(%bv1, %bv12, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3);
}


// struct tuple_(%bv1, %bv5, %bv1, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4);
}


// struct tuple_(%bv1, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2);
}


// struct tuple_(%bv1, %bv5, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3);
}


// struct tuple_(%bv1, %bv7, %bv5, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4);
}


// struct tuple_(%bv1, %bv9, %bv2, %bv5, %bv5)
struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
};

static void COPY(ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
}

static bool EQUAL(ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4);
}


// struct tuple_(%bv17, %bv5)
struct ztuple_z8z5bv17zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

static void COPY(ztuple_z8z5bv17zCz0z5bv5z9)(struct ztuple_z8z5bv17zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv17zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5bv17zCz0z5bv5z9)(struct ztuple_z8z5bv17zCz0z5bv5z9 op1, struct ztuple_z8z5bv17zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1);
}


// struct tuple_(%bv2, %bv1, %bv9, %bv2, %bv5, %bv5)
struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
  uint64_t ztup5;
};

static void COPY(ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
  rop->ztup5 = op.ztup5;
}

static bool EQUAL(ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4) && EQUAL(fbits)(op1.ztup5, op2.ztup5);
}


// struct tuple_(%bv2, %bv5)
struct ztuple_z8z5bv2zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

static void COPY(ztuple_z8z5bv2zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv2zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5z9 op1, struct ztuple_z8z5bv2zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1);
}


// struct tuple_(%bv2, %bv5, %bv1, %bv1, %bv1, %bv5, %bv5)
struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
  uint64_t ztup5;
  uint64_t ztup6;
};

static void COPY(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
  rop->ztup5 = op.ztup5;
  rop->ztup6 = op.ztup6;
}

static bool EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4) && EQUAL(fbits)(op1.ztup5, op2.ztup5) && EQUAL(fbits)(op1.ztup6, op2.ztup6);
}


// struct tuple_(%bv2, %bv5, %bv5)
struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
};

static void COPY(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
}

static bool EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2);
}


// struct tuple_(%bv2, %bv9, %bv5, %bv5)
struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
};

static void COPY(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
}

static bool EQUAL(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3);
}


// struct tuple_(%bv5, %bv1, %bv1, %bv1, %bv1, %bv5, %bv5)
struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
  uint64_t ztup3;
  uint64_t ztup4;
  uint64_t ztup5;
  uint64_t ztup6;
};

static void COPY(ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
  rop->ztup3 = op.ztup3;
  rop->ztup4 = op.ztup4;
  rop->ztup5 = op.ztup5;
  rop->ztup6 = op.ztup6;
}

static bool EQUAL(ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2) && EQUAL(fbits)(op1.ztup3, op2.ztup3) && EQUAL(fbits)(op1.ztup4, op2.ztup4) && EQUAL(fbits)(op1.ztup5, op2.ztup5) && EQUAL(fbits)(op1.ztup6, op2.ztup6);
}


// struct tuple_(%bv5, %bv2, %bv5)
struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
  uint64_t ztup2;
};

static void COPY(ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
  rop->ztup2 = op.ztup2;
}

static bool EQUAL(ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op1, struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1) && EQUAL(fbits)(op1.ztup2, op2.ztup2);
}


// struct tuple_(%bv5, %bv5)
struct ztuple_z8z5bv5zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

static void COPY(ztuple_z8z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv5zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5bv5zCz0z5bv5z9)(struct ztuple_z8z5bv5zCz0z5bv5z9 op1, struct ztuple_z8z5bv5zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1);
}


// struct tuple_(%bv7, %bv5)
struct ztuple_z8z5bv7zCz0z5bv5z9 {
  uint64_t ztup0;
  uint64_t ztup1;
};

static void COPY(ztuple_z8z5bv7zCz0z5bv5z9)(struct ztuple_z8z5bv7zCz0z5bv5z9 *rop, const struct ztuple_z8z5bv7zCz0z5bv5z9 op) {
  rop->ztup0 = op.ztup0;
  rop->ztup1 = op.ztup1;
}

static bool EQUAL(ztuple_z8z5bv7zCz0z5bv5z9)(struct ztuple_z8z5bv7zCz0z5bv5z9 op1, struct ztuple_z8z5bv7zCz0z5bv5z9 op2) {
  return EQUAL(fbits)(op1.ztup0, op2.ztup0) && EQUAL(fbits)(op1.ztup1, op2.ztup1);
}

// union instr_ast
enum kind_zinstr_ast { Kind_zInstr_ALDAR_C_R_C, Kind_zInstr_ALDR_C_RRB_C, Kind_zInstr_ALDR_C_RUI_C, Kind_zInstr_ALDUR_C_RI_C, Kind_zInstr_BLRR_C_C, Kind_zInstr_BLRS_C_C, Kind_zInstr_BLRS_C_C_C, Kind_zInstr_BLR_CI_C, Kind_zInstr_BLR_C_C, Kind_zInstr_BRR_C_C, Kind_zInstr_BRS_C_C, Kind_zInstr_BRS_C_C_C, Kind_zInstr_BR_CI_C, Kind_zInstr_BR_C_C, Kind_zInstr_CASAL_C_R_C, Kind_zInstr_CASA_C_R_C, Kind_zInstr_CASL_C_R_C, Kind_zInstr_CAS_C_R_C, Kind_zInstr_LDAPR_C_R_C, Kind_zInstr_LDAR_C_R_C, Kind_zInstr_LDAXP_C_R_C, Kind_zInstr_LDAXR_C_R_C, Kind_zInstr_LDCT_R_R__, Kind_zInstr_LDNP_C_RIB_C, Kind_zInstr_LDPBLR_C_C_C, Kind_zInstr_LDPBR_C_C_C, Kind_zInstr_LDP_CC_RIAW_C, Kind_zInstr_LDP_C_RIBW_C, Kind_zInstr_LDP_C_RIB_C, Kind_zInstr_LDR_C_I_C, Kind_zInstr_LDR_C_RIAW_C, Kind_zInstr_LDR_C_RIBW_C, Kind_zInstr_LDR_C_RRB_C, Kind_zInstr_LDR_C_RUIB_C, Kind_zInstr_LDTR_C_RIB_C, Kind_zInstr_LDUR_C_RI_C, Kind_zInstr_LDXP_C_R_C, Kind_zInstr_LDXR_C_R_C, Kind_zInstr_Other, Kind_zInstr_RETR_C_C, Kind_zInstr_RETS_C_C, Kind_zInstr_RETS_C_C_C, Kind_zInstr_RET_C_C, Kind_zInstr_SWPAL_CC_R_C, Kind_zInstr_SWPA_CC_R_C, Kind_zInstr_SWPL_CC_R_C, Kind_zInstr_SWP_CC_R_C };

struct zinstr_ast {
  enum kind_zinstr_ast kind;
  union {
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_ALDAR_C_R_C; };
    struct { struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_ALDR_C_RRB_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 zInstr_ALDR_C_RUI_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 zInstr_ALDUR_C_RI_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BLRR_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BLRS_C_C; };
    struct { struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 zInstr_BLRS_C_C_C; };
    struct { struct ztuple_z8z5bv7zCz0z5bv5z9 zInstr_BLR_CI_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BLR_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BRR_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BRS_C_C; };
    struct { struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 zInstr_BRS_C_C_C; };
    struct { struct ztuple_z8z5bv7zCz0z5bv5z9 zInstr_BR_CI_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_BR_C_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_CASAL_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_CASA_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_CASL_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_CAS_C_R_C; };
    struct { struct ztuple_z8z5bv5zCz0z5bv5z9 zInstr_LDAPR_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_LDAR_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDAXP_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_LDAXR_C_R_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 zInstr_LDCT_R_R__; };
    struct { struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDNP_C_RIB_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 zInstr_LDPBLR_C_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 zInstr_LDPBR_C_C_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDP_CC_RIAW_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDP_C_RIBW_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDP_C_RIB_C; };
    struct { struct ztuple_z8z5bv17zCz0z5bv5z9 zInstr_LDR_C_I_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 zInstr_LDR_C_RIAW_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 zInstr_LDR_C_RIBW_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_LDR_C_RRB_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 zInstr_LDR_C_RUIB_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 zInstr_LDTR_C_RIB_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 zInstr_LDUR_C_RI_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_LDXP_C_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 zInstr_LDXR_C_R_C; };
    struct { uint64_t zInstr_Other; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_RETR_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_RETS_C_C; };
    struct { struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 zInstr_RETS_C_C_C; };
    struct { struct ztuple_z8z5bv2zCz0z5bv5z9 zInstr_RET_C_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_SWPAL_CC_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_SWPA_CC_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_SWPL_CC_R_C; };
    struct { struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 zInstr_SWP_CC_R_C; };
  };
};

static void CREATE(zinstr_ast)(struct zinstr_ast *op)
{
  op->kind = Kind_zInstr_ALDAR_C_R_C;

}

static void RECREATE(zinstr_ast)(struct zinstr_ast *op) {}

static void KILL(zinstr_ast)(struct zinstr_ast *op)
{
  if (op->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (op->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  };
}

static void COPY(zinstr_ast)(struct zinstr_ast *rop, struct zinstr_ast op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  };
  rop->kind = op.kind;
  if (op.kind == Kind_zInstr_ALDAR_C_R_C) {
    rop->zInstr_ALDAR_C_R_C = op.zInstr_ALDAR_C_R_C;
  } else if (op.kind == Kind_zInstr_ALDR_C_RRB_C) {
    rop->zInstr_ALDR_C_RRB_C = op.zInstr_ALDR_C_RRB_C;
  } else if (op.kind == Kind_zInstr_ALDR_C_RUI_C) {
    rop->zInstr_ALDR_C_RUI_C = op.zInstr_ALDR_C_RUI_C;
  } else if (op.kind == Kind_zInstr_ALDUR_C_RI_C) {
    rop->zInstr_ALDUR_C_RI_C = op.zInstr_ALDUR_C_RI_C;
  } else if (op.kind == Kind_zInstr_BLRR_C_C) {
    rop->zInstr_BLRR_C_C = op.zInstr_BLRR_C_C;
  } else if (op.kind == Kind_zInstr_BLRS_C_C) {
    rop->zInstr_BLRS_C_C = op.zInstr_BLRS_C_C;
  } else if (op.kind == Kind_zInstr_BLRS_C_C_C) {
    rop->zInstr_BLRS_C_C_C = op.zInstr_BLRS_C_C_C;
  } else if (op.kind == Kind_zInstr_BLR_CI_C) {
    rop->zInstr_BLR_CI_C = op.zInstr_BLR_CI_C;
  } else if (op.kind == Kind_zInstr_BLR_C_C) {
    rop->zInstr_BLR_C_C = op.zInstr_BLR_C_C;
  } else if (op.kind == Kind_zInstr_BRR_C_C) {
    rop->zInstr_BRR_C_C = op.zInstr_BRR_C_C;
  } else if (op.kind == Kind_zInstr_BRS_C_C) {
    rop->zInstr_BRS_C_C = op.zInstr_BRS_C_C;
  } else if (op.kind == Kind_zInstr_BRS_C_C_C) {
    rop->zInstr_BRS_C_C_C = op.zInstr_BRS_C_C_C;
  } else if (op.kind == Kind_zInstr_BR_CI_C) {
    rop->zInstr_BR_CI_C = op.zInstr_BR_CI_C;
  } else if (op.kind == Kind_zInstr_BR_C_C) {
    rop->zInstr_BR_C_C = op.zInstr_BR_C_C;
  } else if (op.kind == Kind_zInstr_CASAL_C_R_C) {
    rop->zInstr_CASAL_C_R_C = op.zInstr_CASAL_C_R_C;
  } else if (op.kind == Kind_zInstr_CASA_C_R_C) {
    rop->zInstr_CASA_C_R_C = op.zInstr_CASA_C_R_C;
  } else if (op.kind == Kind_zInstr_CASL_C_R_C) {
    rop->zInstr_CASL_C_R_C = op.zInstr_CASL_C_R_C;
  } else if (op.kind == Kind_zInstr_CAS_C_R_C) {
    rop->zInstr_CAS_C_R_C = op.zInstr_CAS_C_R_C;
  } else if (op.kind == Kind_zInstr_LDAPR_C_R_C) {
    rop->zInstr_LDAPR_C_R_C = op.zInstr_LDAPR_C_R_C;
  } else if (op.kind == Kind_zInstr_LDAR_C_R_C) {
    rop->zInstr_LDAR_C_R_C = op.zInstr_LDAR_C_R_C;
  } else if (op.kind == Kind_zInstr_LDAXP_C_R_C) {
    rop->zInstr_LDAXP_C_R_C = op.zInstr_LDAXP_C_R_C;
  } else if (op.kind == Kind_zInstr_LDAXR_C_R_C) {
    rop->zInstr_LDAXR_C_R_C = op.zInstr_LDAXR_C_R_C;
  } else if (op.kind == Kind_zInstr_LDCT_R_R__) {
    rop->zInstr_LDCT_R_R__ = op.zInstr_LDCT_R_R__;
  } else if (op.kind == Kind_zInstr_LDNP_C_RIB_C) {
    rop->zInstr_LDNP_C_RIB_C = op.zInstr_LDNP_C_RIB_C;
  } else if (op.kind == Kind_zInstr_LDPBLR_C_C_C) {
    rop->zInstr_LDPBLR_C_C_C = op.zInstr_LDPBLR_C_C_C;
  } else if (op.kind == Kind_zInstr_LDPBR_C_C_C) {
    rop->zInstr_LDPBR_C_C_C = op.zInstr_LDPBR_C_C_C;
  } else if (op.kind == Kind_zInstr_LDP_CC_RIAW_C) {
    rop->zInstr_LDP_CC_RIAW_C = op.zInstr_LDP_CC_RIAW_C;
  } else if (op.kind == Kind_zInstr_LDP_C_RIBW_C) {
    rop->zInstr_LDP_C_RIBW_C = op.zInstr_LDP_C_RIBW_C;
  } else if (op.kind == Kind_zInstr_LDP_C_RIB_C) {
    rop->zInstr_LDP_C_RIB_C = op.zInstr_LDP_C_RIB_C;
  } else if (op.kind == Kind_zInstr_LDR_C_I_C) {
    rop->zInstr_LDR_C_I_C = op.zInstr_LDR_C_I_C;
  } else if (op.kind == Kind_zInstr_LDR_C_RIAW_C) {
    rop->zInstr_LDR_C_RIAW_C = op.zInstr_LDR_C_RIAW_C;
  } else if (op.kind == Kind_zInstr_LDR_C_RIBW_C) {
    rop->zInstr_LDR_C_RIBW_C = op.zInstr_LDR_C_RIBW_C;
  } else if (op.kind == Kind_zInstr_LDR_C_RRB_C) {
    rop->zInstr_LDR_C_RRB_C = op.zInstr_LDR_C_RRB_C;
  } else if (op.kind == Kind_zInstr_LDR_C_RUIB_C) {
    rop->zInstr_LDR_C_RUIB_C = op.zInstr_LDR_C_RUIB_C;
  } else if (op.kind == Kind_zInstr_LDTR_C_RIB_C) {
    rop->zInstr_LDTR_C_RIB_C = op.zInstr_LDTR_C_RIB_C;
  } else if (op.kind == Kind_zInstr_LDUR_C_RI_C) {
    rop->zInstr_LDUR_C_RI_C = op.zInstr_LDUR_C_RI_C;
  } else if (op.kind == Kind_zInstr_LDXP_C_R_C) {
    rop->zInstr_LDXP_C_R_C = op.zInstr_LDXP_C_R_C;
  } else if (op.kind == Kind_zInstr_LDXR_C_R_C) {
    rop->zInstr_LDXR_C_R_C = op.zInstr_LDXR_C_R_C;
  } else if (op.kind == Kind_zInstr_Other) {
    rop->zInstr_Other = op.zInstr_Other;
  } else if (op.kind == Kind_zInstr_RETR_C_C) {
    rop->zInstr_RETR_C_C = op.zInstr_RETR_C_C;
  } else if (op.kind == Kind_zInstr_RETS_C_C) {
    rop->zInstr_RETS_C_C = op.zInstr_RETS_C_C;
  } else if (op.kind == Kind_zInstr_RETS_C_C_C) {
    rop->zInstr_RETS_C_C_C = op.zInstr_RETS_C_C_C;
  } else if (op.kind == Kind_zInstr_RET_C_C) {
    rop->zInstr_RET_C_C = op.zInstr_RET_C_C;
  } else if (op.kind == Kind_zInstr_SWPAL_CC_R_C) {
    rop->zInstr_SWPAL_CC_R_C = op.zInstr_SWPAL_CC_R_C;
  } else if (op.kind == Kind_zInstr_SWPA_CC_R_C) {
    rop->zInstr_SWPA_CC_R_C = op.zInstr_SWPA_CC_R_C;
  } else if (op.kind == Kind_zInstr_SWPL_CC_R_C) {
    rop->zInstr_SWPL_CC_R_C = op.zInstr_SWPL_CC_R_C;
  } else if (op.kind == Kind_zInstr_SWP_CC_R_C){
    rop->zInstr_SWP_CC_R_C = op.zInstr_SWP_CC_R_C;
  }
}

static bool EQUAL(zinstr_ast)(struct zinstr_ast op1, struct zinstr_ast op2) {
  if (op1.kind == Kind_zInstr_ALDAR_C_R_C && op2.kind == Kind_zInstr_ALDAR_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_ALDAR_C_R_C, op2.zInstr_ALDAR_C_R_C);
  } else if (op1.kind == Kind_zInstr_ALDR_C_RRB_C && op2.kind == Kind_zInstr_ALDR_C_RRB_C) {

  return EQUAL(ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_ALDR_C_RRB_C, op2.zInstr_ALDR_C_RRB_C);
  } else if (op1.kind == Kind_zInstr_ALDR_C_RUI_C && op2.kind == Kind_zInstr_ALDR_C_RUI_C) {

  return EQUAL(ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_ALDR_C_RUI_C, op2.zInstr_ALDR_C_RUI_C);
  } else if (op1.kind == Kind_zInstr_ALDUR_C_RI_C && op2.kind == Kind_zInstr_ALDUR_C_RI_C) {

  return EQUAL(ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_ALDUR_C_RI_C, op2.zInstr_ALDUR_C_RI_C);
  } else if (op1.kind == Kind_zInstr_BLRR_C_C && op2.kind == Kind_zInstr_BLRR_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BLRR_C_C, op2.zInstr_BLRR_C_C);
  } else if (op1.kind == Kind_zInstr_BLRS_C_C && op2.kind == Kind_zInstr_BLRS_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BLRS_C_C, op2.zInstr_BLRS_C_C);
  } else if (op1.kind == Kind_zInstr_BLRS_C_C_C && op2.kind == Kind_zInstr_BLRS_C_C_C) {
    return EQUAL(ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9)(op1.zInstr_BLRS_C_C_C, op2.zInstr_BLRS_C_C_C);
  } else if (op1.kind == Kind_zInstr_BLR_CI_C && op2.kind == Kind_zInstr_BLR_CI_C) {
    return EQUAL(ztuple_z8z5bv7zCz0z5bv5z9)(op1.zInstr_BLR_CI_C, op2.zInstr_BLR_CI_C);
  } else if (op1.kind == Kind_zInstr_BLR_C_C && op2.kind == Kind_zInstr_BLR_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BLR_C_C, op2.zInstr_BLR_C_C);
  } else if (op1.kind == Kind_zInstr_BRR_C_C && op2.kind == Kind_zInstr_BRR_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BRR_C_C, op2.zInstr_BRR_C_C);
  } else if (op1.kind == Kind_zInstr_BRS_C_C && op2.kind == Kind_zInstr_BRS_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BRS_C_C, op2.zInstr_BRS_C_C);
  } else if (op1.kind == Kind_zInstr_BRS_C_C_C && op2.kind == Kind_zInstr_BRS_C_C_C) {
    return EQUAL(ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9)(op1.zInstr_BRS_C_C_C, op2.zInstr_BRS_C_C_C);
  } else if (op1.kind == Kind_zInstr_BR_CI_C && op2.kind == Kind_zInstr_BR_CI_C) {
    return EQUAL(ztuple_z8z5bv7zCz0z5bv5z9)(op1.zInstr_BR_CI_C, op2.zInstr_BR_CI_C);
  } else if (op1.kind == Kind_zInstr_BR_C_C && op2.kind == Kind_zInstr_BR_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_BR_C_C, op2.zInstr_BR_C_C);
  } else if (op1.kind == Kind_zInstr_CASAL_C_R_C && op2.kind == Kind_zInstr_CASAL_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_CASAL_C_R_C, op2.zInstr_CASAL_C_R_C);
  } else if (op1.kind == Kind_zInstr_CASA_C_R_C && op2.kind == Kind_zInstr_CASA_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_CASA_C_R_C, op2.zInstr_CASA_C_R_C);
  } else if (op1.kind == Kind_zInstr_CASL_C_R_C && op2.kind == Kind_zInstr_CASL_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_CASL_C_R_C, op2.zInstr_CASL_C_R_C);
  } else if (op1.kind == Kind_zInstr_CAS_C_R_C && op2.kind == Kind_zInstr_CAS_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_CAS_C_R_C, op2.zInstr_CAS_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDAPR_C_R_C && op2.kind == Kind_zInstr_LDAPR_C_R_C) {
    return EQUAL(ztuple_z8z5bv5zCz0z5bv5z9)(op1.zInstr_LDAPR_C_R_C, op2.zInstr_LDAPR_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDAR_C_R_C && op2.kind == Kind_zInstr_LDAR_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDAR_C_R_C, op2.zInstr_LDAR_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDAXP_C_R_C && op2.kind == Kind_zInstr_LDAXP_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDAXP_C_R_C, op2.zInstr_LDAXP_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDAXR_C_R_C && op2.kind == Kind_zInstr_LDAXR_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDAXR_C_R_C, op2.zInstr_LDAXR_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDCT_R_R__ && op2.kind == Kind_zInstr_LDCT_R_R__) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDCT_R_R__, op2.zInstr_LDCT_R_R__);
  } else if (op1.kind == Kind_zInstr_LDNP_C_RIB_C && op2.kind == Kind_zInstr_LDNP_C_RIB_C) {

  return EQUAL(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDNP_C_RIB_C, op2.zInstr_LDNP_C_RIB_C);
  } else if (op1.kind == Kind_zInstr_LDPBLR_C_C_C && op2.kind == Kind_zInstr_LDPBLR_C_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDPBLR_C_C_C, op2.zInstr_LDPBLR_C_C_C);
  } else if (op1.kind == Kind_zInstr_LDPBR_C_C_C && op2.kind == Kind_zInstr_LDPBR_C_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDPBR_C_C_C, op2.zInstr_LDPBR_C_C_C);
  } else if (op1.kind == Kind_zInstr_LDP_CC_RIAW_C && op2.kind == Kind_zInstr_LDP_CC_RIAW_C) {

  return EQUAL(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDP_CC_RIAW_C, op2.zInstr_LDP_CC_RIAW_C);
  } else if (op1.kind == Kind_zInstr_LDP_C_RIBW_C && op2.kind == Kind_zInstr_LDP_C_RIBW_C) {

  return EQUAL(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDP_C_RIBW_C, op2.zInstr_LDP_C_RIBW_C);
  } else if (op1.kind == Kind_zInstr_LDP_C_RIB_C && op2.kind == Kind_zInstr_LDP_C_RIB_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDP_C_RIB_C, op2.zInstr_LDP_C_RIB_C);
  } else if (op1.kind == Kind_zInstr_LDR_C_I_C && op2.kind == Kind_zInstr_LDR_C_I_C) {
    return EQUAL(ztuple_z8z5bv17zCz0z5bv5z9)(op1.zInstr_LDR_C_I_C, op2.zInstr_LDR_C_I_C);
  } else if (op1.kind == Kind_zInstr_LDR_C_RIAW_C && op2.kind == Kind_zInstr_LDR_C_RIAW_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDR_C_RIAW_C, op2.zInstr_LDR_C_RIAW_C);
  } else if (op1.kind == Kind_zInstr_LDR_C_RIBW_C && op2.kind == Kind_zInstr_LDR_C_RIBW_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDR_C_RIBW_C, op2.zInstr_LDR_C_RIBW_C);
  } else if (op1.kind == Kind_zInstr_LDR_C_RRB_C && op2.kind == Kind_zInstr_LDR_C_RRB_C) {

  return EQUAL(ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDR_C_RRB_C, op2.zInstr_LDR_C_RRB_C);
  } else if (op1.kind == Kind_zInstr_LDR_C_RUIB_C && op2.kind == Kind_zInstr_LDR_C_RUIB_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDR_C_RUIB_C, op2.zInstr_LDR_C_RUIB_C);
  } else if (op1.kind == Kind_zInstr_LDTR_C_RIB_C && op2.kind == Kind_zInstr_LDTR_C_RIB_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDTR_C_RIB_C, op2.zInstr_LDTR_C_RIB_C);
  } else if (op1.kind == Kind_zInstr_LDUR_C_RI_C && op2.kind == Kind_zInstr_LDUR_C_RI_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDUR_C_RI_C, op2.zInstr_LDUR_C_RI_C);
  } else if (op1.kind == Kind_zInstr_LDXP_C_R_C && op2.kind == Kind_zInstr_LDXP_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDXP_C_R_C, op2.zInstr_LDXP_C_R_C);
  } else if (op1.kind == Kind_zInstr_LDXR_C_R_C && op2.kind == Kind_zInstr_LDXR_C_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_LDXR_C_R_C, op2.zInstr_LDXR_C_R_C);
  } else if (op1.kind == Kind_zInstr_Other && op2.kind == Kind_zInstr_Other) {
    return EQUAL(fbits)(op1.zInstr_Other, op2.zInstr_Other);
  } else if (op1.kind == Kind_zInstr_RETR_C_C && op2.kind == Kind_zInstr_RETR_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_RETR_C_C, op2.zInstr_RETR_C_C);
  } else if (op1.kind == Kind_zInstr_RETS_C_C && op2.kind == Kind_zInstr_RETS_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_RETS_C_C, op2.zInstr_RETS_C_C);
  } else if (op1.kind == Kind_zInstr_RETS_C_C_C && op2.kind == Kind_zInstr_RETS_C_C_C) {
    return EQUAL(ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9)(op1.zInstr_RETS_C_C_C, op2.zInstr_RETS_C_C_C);
  } else if (op1.kind == Kind_zInstr_RET_C_C && op2.kind == Kind_zInstr_RET_C_C) {
    return EQUAL(ztuple_z8z5bv2zCz0z5bv5z9)(op1.zInstr_RET_C_C, op2.zInstr_RET_C_C);
  } else if (op1.kind == Kind_zInstr_SWPAL_CC_R_C && op2.kind == Kind_zInstr_SWPAL_CC_R_C) {

  return EQUAL(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_SWPAL_CC_R_C, op2.zInstr_SWPAL_CC_R_C);
  } else if (op1.kind == Kind_zInstr_SWPA_CC_R_C && op2.kind == Kind_zInstr_SWPA_CC_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_SWPA_CC_R_C, op2.zInstr_SWPA_CC_R_C);
  } else if (op1.kind == Kind_zInstr_SWPL_CC_R_C && op2.kind == Kind_zInstr_SWPL_CC_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_SWPL_CC_R_C, op2.zInstr_SWPL_CC_R_C);
  } else if (op1.kind == Kind_zInstr_SWP_CC_R_C && op2.kind == Kind_zInstr_SWP_CC_R_C) {
    return EQUAL(ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9)(op1.zInstr_SWP_CC_R_C, op2.zInstr_SWP_CC_R_C);
  } else return false;
}

static void sailgen_Instr_ALDAR_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_ALDAR_C_R_C;
  rop->zInstr_ALDAR_C_R_C = op;
}

static void sailgen_Instr_ALDR_C_RRB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_ALDR_C_RRB_C;
  rop->zInstr_ALDR_C_RRB_C = op;
}

static void sailgen_Instr_ALDR_C_RUI_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_ALDR_C_RUI_C;
  rop->zInstr_ALDR_C_RUI_C = op;
}

static void sailgen_Instr_ALDUR_C_RI_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv1zCz0z5bv9zCz0z5bv2zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_ALDUR_C_RI_C;
  rop->zInstr_ALDUR_C_RI_C = op;
}

static void sailgen_Instr_BLRR_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BLRR_C_C;
  rop->zInstr_BLRR_C_C = op;
}

static void sailgen_Instr_BLRS_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BLRS_C_C;
  rop->zInstr_BLRS_C_C = op;
}

static void sailgen_Instr_BLRS_C_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BLRS_C_C_C;
  rop->zInstr_BLRS_C_C_C = op;
}

static void sailgen_Instr_BLR_CI_C(struct zinstr_ast *rop, struct ztuple_z8z5bv7zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BLR_CI_C;
  rop->zInstr_BLR_CI_C = op;
}

static void sailgen_Instr_BLR_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BLR_C_C;
  rop->zInstr_BLR_C_C = op;
}

static void sailgen_Instr_BRR_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BRR_C_C;
  rop->zInstr_BRR_C_C = op;
}

static void sailgen_Instr_BRS_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BRS_C_C;
  rop->zInstr_BRS_C_C = op;
}

static void sailgen_Instr_BRS_C_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BRS_C_C_C;
  rop->zInstr_BRS_C_C_C = op;
}

static void sailgen_Instr_BR_CI_C(struct zinstr_ast *rop, struct ztuple_z8z5bv7zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BR_CI_C;
  rop->zInstr_BR_CI_C = op;
}

static void sailgen_Instr_BR_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_BR_C_C;
  rop->zInstr_BR_C_C = op;
}

static void sailgen_Instr_CASAL_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_CASAL_C_R_C;
  rop->zInstr_CASAL_C_R_C = op;
}

static void sailgen_Instr_CASA_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_CASA_C_R_C;
  rop->zInstr_CASA_C_R_C = op;
}

static void sailgen_Instr_CASL_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_CASL_C_R_C;
  rop->zInstr_CASL_C_R_C = op;
}

static void sailgen_Instr_CAS_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_CAS_C_R_C;
  rop->zInstr_CAS_C_R_C = op;
}

static void sailgen_Instr_LDAPR_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDAPR_C_R_C;
  rop->zInstr_LDAPR_C_R_C = op;
}

static void sailgen_Instr_LDAR_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDAR_C_R_C;
  rop->zInstr_LDAR_C_R_C = op;
}

static void sailgen_Instr_LDAXP_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDAXP_C_R_C;
  rop->zInstr_LDAXP_C_R_C = op;
}

static void sailgen_Instr_LDAXR_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDAXR_C_R_C;
  rop->zInstr_LDAXR_C_R_C = op;
}

static void sailgen_Instr_LDCT_R_R__(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDCT_R_R__;
  rop->zInstr_LDCT_R_R__ = op;
}

static void sailgen_Instr_LDNP_C_RIB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDNP_C_RIB_C;
  rop->zInstr_LDNP_C_RIB_C = op;
}

static void sailgen_Instr_LDPBLR_C_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDPBLR_C_C_C;
  rop->zInstr_LDPBLR_C_C_C = op;
}

static void sailgen_Instr_LDPBR_C_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDPBR_C_C_C;
  rop->zInstr_LDPBR_C_C_C = op;
}

static void sailgen_Instr_LDP_CC_RIAW_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDP_CC_RIAW_C;
  rop->zInstr_LDP_CC_RIAW_C = op;
}

static void sailgen_Instr_LDP_C_RIBW_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDP_C_RIBW_C;
  rop->zInstr_LDP_C_RIBW_C = op;
}

static void sailgen_Instr_LDP_C_RIB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv7zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDP_C_RIB_C;
  rop->zInstr_LDP_C_RIB_C = op;
}

static void sailgen_Instr_LDR_C_I_C(struct zinstr_ast *rop, struct ztuple_z8z5bv17zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDR_C_I_C;
  rop->zInstr_LDR_C_I_C = op;
}

static void sailgen_Instr_LDR_C_RIAW_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDR_C_RIAW_C;
  rop->zInstr_LDR_C_RIAW_C = op;
}

static void sailgen_Instr_LDR_C_RIBW_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDR_C_RIBW_C;
  rop->zInstr_LDR_C_RIBW_C = op;
}

static void sailgen_Instr_LDR_C_RRB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5zCz0z5bv1zCz0z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDR_C_RRB_C;
  rop->zInstr_LDR_C_RRB_C = op;
}

static void sailgen_Instr_LDR_C_RUIB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv12zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDR_C_RUIB_C;
  rop->zInstr_LDR_C_RUIB_C = op;
}

static void sailgen_Instr_LDTR_C_RIB_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDTR_C_RIB_C;
  rop->zInstr_LDTR_C_RIB_C = op;
}

static void sailgen_Instr_LDUR_C_RI_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv9zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDUR_C_RI_C;
  rop->zInstr_LDUR_C_RI_C = op;
}

static void sailgen_Instr_LDXP_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDXP_C_R_C;
  rop->zInstr_LDXP_C_R_C = op;
}

static void sailgen_Instr_LDXR_C_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_LDXR_C_R_C;
  rop->zInstr_LDXR_C_R_C = op;
}

static void sailgen_Instr_Other(struct zinstr_ast *rop, uint64_t op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_Other;
  rop->zInstr_Other = op;
}

static void sailgen_Instr_RETR_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_RETR_C_C;
  rop->zInstr_RETR_C_C = op;
}

static void sailgen_Instr_RETS_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_RETS_C_C;
  rop->zInstr_RETS_C_C = op;
}

static void sailgen_Instr_RETS_C_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv5zCz0z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_RETS_C_C_C;
  rop->zInstr_RETS_C_C_C = op;
}

static void sailgen_Instr_RET_C_C(struct zinstr_ast *rop, struct ztuple_z8z5bv2zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_RET_C_C;
  rop->zInstr_RET_C_C = op;
}

static void sailgen_Instr_SWPAL_CC_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_SWPAL_CC_R_C;
  rop->zInstr_SWPAL_CC_R_C = op;
}

static void sailgen_Instr_SWPA_CC_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_SWPA_CC_R_C;
  rop->zInstr_SWPA_CC_R_C = op;
}

static void sailgen_Instr_SWPL_CC_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_SWPL_CC_R_C;
  rop->zInstr_SWPL_CC_R_C = op;
}

static void sailgen_Instr_SWP_CC_R_C(struct zinstr_ast *rop, struct ztuple_z8z5bv1zCz0z5bv1zCz0z5bv5zCz0z5bv5zCz0z5bv5z9 op)
{
  if (rop->kind == Kind_zInstr_ALDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDR_C_RUI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_ALDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BLR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BRS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_CI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_BR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASAL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASA_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CASL_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_CAS_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAPR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDAXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDCT_R_R__) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDNP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBLR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDPBR_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_CC_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDP_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_I_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIAW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RIBW_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RRB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDR_C_RUIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDTR_C_RIB_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDUR_C_RI_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXP_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_LDXR_C_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_Other) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETR_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RETS_C_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_RET_C_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPAL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPA_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWPL_CC_R_C) {
    /* do nothing */
  } else if (rop->kind == Kind_zInstr_SWP_CC_R_C){
    /* do nothing */
  }
  rop->kind = Kind_zInstr_SWP_CC_R_C;
  rop->zInstr_SWP_CC_R_C = op;
}

// union exception
enum kind_zexception { Kind_zError_ConstrainedUnpredictable, Kind_zError_ExceptionTaken, Kind_zError_ImplementationDefined, Kind_zError_ReservedEncoding, Kind_zError_SError, Kind_zError_See, Kind_zError_Undefined, Kind_zError_Unpredictable };

struct zexception {
  enum kind_zexception kind;
  union {
    struct { unit zError_ConstrainedUnpredictable; };
    struct { unit zError_ExceptionTaken; };
    struct { sail_string zError_ImplementationDefined; };
    struct { unit zError_ReservedEncoding; };
    struct { unit zError_SError; };
    struct { sail_string zError_See; };
    struct { unit zError_Undefined; };
    struct { unit zError_Unpredictable; };
  };
};

static void CREATE(zexception)(struct zexception *op)
{
  op->kind = Kind_zError_ConstrainedUnpredictable;

}

static void RECREATE(zexception)(struct zexception *op) {}

static void KILL(zexception)(struct zexception *op)
{
  if (op->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (op->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (op->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&op->zError_ImplementationDefined);
  } else if (op->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (op->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (op->kind == Kind_zError_See) {
    KILL(sail_string)(&op->zError_See);
  } else if (op->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (op->kind == Kind_zError_Unpredictable){
    /* do nothing */
  };
}

static void COPY(zexception)(struct zexception *rop, struct zexception op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  };
  rop->kind = op.kind;
  if (op.kind == Kind_zError_ConstrainedUnpredictable) {
    rop->zError_ConstrainedUnpredictable = op.zError_ConstrainedUnpredictable;
  } else if (op.kind == Kind_zError_ExceptionTaken) {
    rop->zError_ExceptionTaken = op.zError_ExceptionTaken;
  } else if (op.kind == Kind_zError_ImplementationDefined) {

  CREATE(sail_string)(&rop->zError_ImplementationDefined); COPY(sail_string)(&rop->zError_ImplementationDefined, op.zError_ImplementationDefined);
  } else if (op.kind == Kind_zError_ReservedEncoding) {
    rop->zError_ReservedEncoding = op.zError_ReservedEncoding;
  } else if (op.kind == Kind_zError_SError) {
    rop->zError_SError = op.zError_SError;
  } else if (op.kind == Kind_zError_See) {
    CREATE(sail_string)(&rop->zError_See); COPY(sail_string)(&rop->zError_See, op.zError_See);
  } else if (op.kind == Kind_zError_Undefined) {
    rop->zError_Undefined = op.zError_Undefined;
  } else if (op.kind == Kind_zError_Unpredictable){
    rop->zError_Unpredictable = op.zError_Unpredictable;
  }
}

static bool EQUAL(zexception)(struct zexception op1, struct zexception op2) {
  if (op1.kind == Kind_zError_ConstrainedUnpredictable && op2.kind == Kind_zError_ConstrainedUnpredictable) {
    return EQUAL(unit)(op1.zError_ConstrainedUnpredictable, op2.zError_ConstrainedUnpredictable);
  } else if (op1.kind == Kind_zError_ExceptionTaken && op2.kind == Kind_zError_ExceptionTaken) {
    return EQUAL(unit)(op1.zError_ExceptionTaken, op2.zError_ExceptionTaken);
  } else if (op1.kind == Kind_zError_ImplementationDefined && op2.kind == Kind_zError_ImplementationDefined) {
    return EQUAL(sail_string)(op1.zError_ImplementationDefined, op2.zError_ImplementationDefined);
  } else if (op1.kind == Kind_zError_ReservedEncoding && op2.kind == Kind_zError_ReservedEncoding) {
    return EQUAL(unit)(op1.zError_ReservedEncoding, op2.zError_ReservedEncoding);
  } else if (op1.kind == Kind_zError_SError && op2.kind == Kind_zError_SError) {
    return EQUAL(unit)(op1.zError_SError, op2.zError_SError);
  } else if (op1.kind == Kind_zError_See && op2.kind == Kind_zError_See) {
    return EQUAL(sail_string)(op1.zError_See, op2.zError_See);
  } else if (op1.kind == Kind_zError_Undefined && op2.kind == Kind_zError_Undefined) {
    return EQUAL(unit)(op1.zError_Undefined, op2.zError_Undefined);
  } else if (op1.kind == Kind_zError_Unpredictable && op2.kind == Kind_zError_Unpredictable) {
    return EQUAL(unit)(op1.zError_Unpredictable, op2.zError_Unpredictable);
  } else return false;
}

static void sailgen_Error_ConstrainedUnpredictable(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_ConstrainedUnpredictable;
  rop->zError_ConstrainedUnpredictable = op;
}

static void sailgen_Error_ExceptionTaken(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_ExceptionTaken;
  rop->zError_ExceptionTaken = op;
}

static void sailgen_Error_ImplementationDefined(struct zexception *rop, sail_string op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_ImplementationDefined;
  CREATE(sail_string)(&rop->zError_ImplementationDefined);
  COPY(sail_string)(&rop->zError_ImplementationDefined, op);

}

static void sailgen_Error_ReservedEncoding(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_ReservedEncoding;
  rop->zError_ReservedEncoding = op;
}

static void sailgen_Error_SError(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_SError;
  rop->zError_SError = op;
}

static void sailgen_Error_See(struct zexception *rop, sail_string op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_See;
  CREATE(sail_string)(&rop->zError_See);
  COPY(sail_string)(&rop->zError_See, op);

}

static void sailgen_Error_Undefined(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_Undefined;
  rop->zError_Undefined = op;
}

static void sailgen_Error_Unpredictable(struct zexception *rop, unit op)
{
  if (rop->kind == Kind_zError_ConstrainedUnpredictable) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ExceptionTaken) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_ImplementationDefined) {
    KILL(sail_string)(&rop->zError_ImplementationDefined);
  } else if (rop->kind == Kind_zError_ReservedEncoding) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_SError) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_See) {
    KILL(sail_string)(&rop->zError_See);
  } else if (rop->kind == Kind_zError_Undefined) {
    /* do nothing */
  } else if (rop->kind == Kind_zError_Unpredictable){
    /* do nothing */
  }
  rop->kind = Kind_zError_Unpredictable;
  rop->zError_Unpredictable = op;
}

struct zexception *current_exception = NULL;
bool have_exception = false;
sail_string *throw_location = NULL;

// enum cache_op_kind
enum zcache_op_kind { zCache_op_D_IVAC, zCache_op_D_ISW, zCache_op_D_CSW, zCache_op_D_CISW, zCache_op_D_ZVA, zCache_op_D_CVAC, zCache_op_D_CVAU, zCache_op_D_CIVAC, zCache_op_I_IALLUIS, zCache_op_I_IALLU, zCache_op_I_IVAU };

static bool eq_zcache_op_kind(enum zcache_op_kind op1, enum zcache_op_kind op2) { return op1 == op2; }

static enum zcache_op_kind UNDEFINED(zcache_op_kind)(unit u) { return zCache_op_D_IVAC; }

// enum a64_barrier_type
enum za64_barrier_type { zA64_barrier_all, zA64_barrier_LD, zA64_barrier_ST };

static bool eq_za64_barrier_type(enum za64_barrier_type op1, enum za64_barrier_type op2) { return op1 == op2; }

static enum za64_barrier_type UNDEFINED(za64_barrier_type)(unit u) { return zA64_barrier_all; }

// enum a64_barrier_domain
enum za64_barrier_domain { zA64_FullShare, zA64_InnerShare, zA64_OuterShare, zA64_NonShare };

static bool eq_za64_barrier_domain(enum za64_barrier_domain op1, enum za64_barrier_domain op2) { return op1 == op2; }

static enum za64_barrier_domain UNDEFINED(za64_barrier_domain)(unit u) { return zA64_FullShare; }

// enum __InstrEnc
enum z__InstrEnc { z__A64, z__A32, z__T16, z__T32 };

static bool eq_z__InstrEnc(enum z__InstrEnc op1, enum z__InstrEnc op2) { return op1 == op2; }

static enum z__InstrEnc UNDEFINED(z__InstrEnc)(unit u) { return z__A64; }

// enum VirtualAddressType
enum zVirtualAddressType { zVA_Bits64, zVA_Capability };

static bool eq_zVirtualAddressType(enum zVirtualAddressType op1, enum zVirtualAddressType op2) { return op1 == op2; }

static enum zVirtualAddressType UNDEFINED(zVirtualAddressType)(unit u) { return zVA_Bits64; }

// enum VBitOp
enum zVBitOp { zVBitOp_VBIF, zVBitOp_VBIT, zVBitOp_VBSL, zVBitOp_VEOR };

static bool eq_zVBitOp(enum zVBitOp op1, enum zVBitOp op2) { return op1 == op2; }

static enum zVBitOp UNDEFINED(zVBitOp)(unit u) { return zVBitOp_VBIF; }

// enum Unpredictable
enum zUnpredictable { zUnpredictable_WBOVERLAPLD, zUnpredictable_WBOVERLAPST, zUnpredictable_LDPOVERLAP, zUnpredictable_BASEOVERLAP, zUnpredictable_DATAOVERLAP, zUnpredictable_DEVPAGE2, zUnpredictable_INSTRDEVICE, zUnpredictable_RESCPACR, zUnpredictable_RESMAIR, zUnpredictable_RESTEXCB, zUnpredictable_RESPRRR, zUnpredictable_RESDACR, zUnpredictable_RESVTCRS, zUnpredictable_RESTnSZ, zUnpredictable_LARGEIPA, zUnpredictable_ESRCONDPASS, zUnpredictable_ILZEROIT, zUnpredictable_ILZEROT, zUnpredictable_BPVECTORCATCHPRI, zUnpredictable_VCMATCHHALF, zUnpredictable_VCMATCHDAPA, zUnpredictable_WPMASKANDBAS, zUnpredictable_WPBASCONTIGUOUS, zUnpredictable_RESWPMASK, zUnpredictable_WPMASKEDBITS, zUnpredictable_RESBPWPCTRL, zUnpredictable_BPNOTIMPL, zUnpredictable_RESBPTYPE, zUnpredictable_BPNOTCTXCMP, zUnpredictable_BPMATCHHALF, zUnpredictable_BPMISMATCHHALF, zUnpredictable_RESTARTALIGNPC, zUnpredictable_RESTARTZEROUPPERPC, zUnpredictable_ZEROUPPER, zUnpredictable_ERETZEROUPPERPC, zUnpredictable_A32FORCEALIGNPC, zUnpredictable_SMD, zUnpredictable_AFUPDATE, zUnpredictable_IESBinDebug, zUnpredictable_BADPMSFCR, zUnpredictable_LINKTRANSFEROVERLAPLD, zUnpredictable_LINKBASEOVERLAPLD, zUnpredictable_CLEARERRITEZERO };

static bool eq_zUnpredictable(enum zUnpredictable op1, enum zUnpredictable op2) { return op1 == op2; }

static enum zUnpredictable UNDEFINED(zUnpredictable)(unit u) { return zUnpredictable_WBOVERLAPLD; }

// enum SystemHintOp
enum zSystemHintOp { zSystemHintOp_NOP, zSystemHintOp_YIELD, zSystemHintOp_WFE, zSystemHintOp_WFI, zSystemHintOp_SEV, zSystemHintOp_SEVL, zSystemHintOp_ESB, zSystemHintOp_PSB, zSystemHintOp_CSDB };

static bool eq_zSystemHintOp(enum zSystemHintOp op1, enum zSystemHintOp op2) { return op1 == op2; }

static enum zSystemHintOp UNDEFINED(zSystemHintOp)(unit u) { return zSystemHintOp_NOP; }

// enum ShiftType
enum zShiftType { zShiftType_LSL, zShiftType_LSR, zShiftType_ASR, zShiftType_ROR };

static bool eq_zShiftType(enum zShiftType op1, enum zShiftType op2) { return op1 == op2; }

static enum zShiftType UNDEFINED(zShiftType)(unit u) { return zShiftType_LSL; }

// enum ReduceOp
enum zReduceOp { zReduceOp_FMINNUM, zReduceOp_FMAXNUM, zReduceOp_FMIN, zReduceOp_FMAX, zReduceOp_FADD, zReduceOp_ADD };

static bool eq_zReduceOp(enum zReduceOp op1, enum zReduceOp op2) { return op1 == op2; }

static enum zReduceOp UNDEFINED(zReduceOp)(unit u) { return zReduceOp_FMINNUM; }

// struct ProcState
struct zProcState {
  uint64_t zA;
  uint64_t zC;
  uint64_t zC64;
  uint64_t zD;
  uint64_t zDIT;
  uint64_t zE;
  uint64_t zEL;
  uint64_t zF;
  uint64_t zGE;
  uint64_t zI;
  uint64_t zIL;
  uint64_t zIT;
  uint64_t zJ;
  uint64_t zM;
  uint64_t zN;
  uint64_t zPAN;
  uint64_t zQ;
  uint64_t zSP;
  uint64_t zSS;
  uint64_t zSSBS;
  uint64_t zT;
  uint64_t zTCO;
  uint64_t zUAO;
  uint64_t zV;
  uint64_t zZ;
  uint64_t znRW;
};

static void COPY(zProcState)(struct zProcState *rop, const struct zProcState op) {
  rop->zA = op.zA;
  rop->zC = op.zC;
  rop->zC64 = op.zC64;
  rop->zD = op.zD;
  rop->zDIT = op.zDIT;
  rop->zE = op.zE;
  rop->zEL = op.zEL;
  rop->zF = op.zF;
  rop->zGE = op.zGE;
  rop->zI = op.zI;
  rop->zIL = op.zIL;
  rop->zIT = op.zIT;
  rop->zJ = op.zJ;
  rop->zM = op.zM;
  rop->zN = op.zN;
  rop->zPAN = op.zPAN;
  rop->zQ = op.zQ;
  rop->zSP = op.zSP;
  rop->zSS = op.zSS;
  rop->zSSBS = op.zSSBS;
  rop->zT = op.zT;
  rop->zTCO = op.zTCO;
  rop->zUAO = op.zUAO;
  rop->zV = op.zV;
  rop->zZ = op.zZ;
  rop->znRW = op.znRW;
}

static bool EQUAL(zProcState)(struct zProcState op1, struct zProcState op2) {
  return EQUAL(fbits)(op1.zA, op2.zA) && EQUAL(fbits)(op1.zC, op2.zC) && EQUAL(fbits)(op1.zC64, op2.zC64) && EQUAL(fbits)(op1.zD, op2.zD) && EQUAL(fbits)(op1.zDIT, op2.zDIT) && EQUAL(fbits)(op1.zE, op2.zE) && EQUAL(fbits)(op1.zEL, op2.zEL) && EQUAL(fbits)(op1.zF, op2.zF) && EQUAL(fbits)(op1.zGE, op2.zGE) && EQUAL(fbits)(op1.zI, op2.zI) && EQUAL(fbits)(op1.zIL, op2.zIL) && EQUAL(fbits)(op1.zIT, op2.zIT) && EQUAL(fbits)(op1.zJ, op2.zJ) && EQUAL(fbits)(op1.zM, op2.zM) && EQUAL(fbits)(op1.zN, op2.zN) && EQUAL(fbits)(op1.zPAN, op2.zPAN) && EQUAL(fbits)(op1.zQ, op2.zQ) && EQUAL(fbits)(op1.zSP, op2.zSP) && EQUAL(fbits)(op1.zSS, op2.zSS) && EQUAL(fbits)(op1.zSSBS, op2.zSSBS) && EQUAL(fbits)(op1.zT, op2.zT) && EQUAL(fbits)(op1.zTCO, op2.zTCO) && EQUAL(fbits)(op1.zUAO, op2.zUAO) && EQUAL(fbits)(op1.zV, op2.zV) && EQUAL(fbits)(op1.zZ, op2.zZ) && EQUAL(fbits)(op1.znRW, op2.znRW);
}

// enum PrefetchHint
enum zPrefetchHint { zPrefetch_READ, zPrefetch_WRITE, zPrefetch_EXEC };

static bool eq_zPrefetchHint(enum zPrefetchHint op1, enum zPrefetchHint op2) { return op1 == op2; }

static enum zPrefetchHint UNDEFINED(zPrefetchHint)(unit u) { return zPrefetch_READ; }

// enum PSTATEField
enum zPSTATEField { zPSTATEField_DAIFSet, zPSTATEField_DAIFClr, zPSTATEField_PAN, zPSTATEField_UAO, zPSTATEField_SSBS, zPSTATEField_SP };

static bool eq_zPSTATEField(enum zPSTATEField op1, enum zPSTATEField op2) { return op1 == op2; }

static enum zPSTATEField UNDEFINED(zPSTATEField)(unit u) { return zPSTATEField_DAIFSet; }

// enum MoveWideOp
enum zMoveWideOp { zMoveWideOp_N, zMoveWideOp_Z, zMoveWideOp_K };

static bool eq_zMoveWideOp(enum zMoveWideOp op1, enum zMoveWideOp op2) { return op1 == op2; }

static enum zMoveWideOp UNDEFINED(zMoveWideOp)(unit u) { return zMoveWideOp_N; }

// enum MemType
enum zMemType { zMemType_Normal, zMemType_Device };

static bool eq_zMemType(enum zMemType op1, enum zMemType op2) { return op1 == op2; }

static enum zMemType UNDEFINED(zMemType)(unit u) { return zMemType_Normal; }

// enum MemOp
enum zMemOp { zMemOp_LOAD, zMemOp_STORE, zMemOp_PREFETCH };

static bool eq_zMemOp(enum zMemOp op1, enum zMemOp op2) { return op1 == op2; }

static enum zMemOp UNDEFINED(zMemOp)(unit u) { return zMemOp_LOAD; }

// enum MemAtomicOp
enum zMemAtomicOp { zMemAtomicOp_ADD, zMemAtomicOp_BIC, zMemAtomicOp_EOR, zMemAtomicOp_ORR, zMemAtomicOp_SMAX, zMemAtomicOp_SMIN, zMemAtomicOp_UMAX, zMemAtomicOp_UMIN, zMemAtomicOp_SWP };

static bool eq_zMemAtomicOp(enum zMemAtomicOp op1, enum zMemAtomicOp op2) { return op1 == op2; }

static enum zMemAtomicOp UNDEFINED(zMemAtomicOp)(unit u) { return zMemAtomicOp_ADD; }

// enum MBReqTypes
enum zMBReqTypes { zMBReqTypes_Reads, zMBReqTypes_Writes, zMBReqTypes_All };

static bool eq_zMBReqTypes(enum zMBReqTypes op1, enum zMBReqTypes op2) { return op1 == op2; }

static enum zMBReqTypes UNDEFINED(zMBReqTypes)(unit u) { return zMBReqTypes_Reads; }

// enum MBReqDomain
enum zMBReqDomain { zMBReqDomain_Nonshareable, zMBReqDomain_InnerShareable, zMBReqDomain_OuterShareable, zMBReqDomain_FullSystem };

static bool eq_zMBReqDomain(enum zMBReqDomain op1, enum zMBReqDomain op2) { return op1 == op2; }

static enum zMBReqDomain UNDEFINED(zMBReqDomain)(unit u) { return zMBReqDomain_Nonshareable; }

// enum LogicalOp
enum zLogicalOp { zLogicalOp_AND, zLogicalOp_EOR, zLogicalOp_ORR };

static bool eq_zLogicalOp(enum zLogicalOp op1, enum zLogicalOp op2) { return op1 == op2; }

static enum zLogicalOp UNDEFINED(zLogicalOp)(unit u) { return zLogicalOp_AND; }

// enum ImmediateOp
enum zImmediateOp { zImmediateOp_MOVI, zImmediateOp_MVNI, zImmediateOp_ORR, zImmediateOp_BIC };

static bool eq_zImmediateOp(enum zImmediateOp op1, enum zImmediateOp op2) { return op1 == op2; }

static enum zImmediateOp UNDEFINED(zImmediateOp)(unit u) { return zImmediateOp_MOVI; }

// enum Fault
enum zFault { zFault_None, zFault_AccessFlag, zFault_Alignment, zFault_Background, zFault_Domain, zFault_Permission, zFault_Translation, zFault_AddressSizze, zFault_SyncExternal, zFault_SyncExternalOnWalk, zFault_SyncParity, zFault_SyncParityOnWalk, zFault_AsyncParity, zFault_AsyncExternal, zFault_Debug, zFault_TLBConflict, zFault_HWUpdateAccessFlag, zFault_CapTag, zFault_CapSeal, zFault_CapBounds, zFault_CapPerm, zFault_CapPagePerm, zFault_Lockdown, zFault_Exclusive, zFault_ICacheMaint };

static bool eq_zFault(enum zFault op1, enum zFault op2) { return op1 == op2; }

static enum zFault UNDEFINED(zFault)(unit u) { return zFault_None; }

// enum FPUnaryOp
enum zFPUnaryOp { zFPUnaryOp_ABS, zFPUnaryOp_MOV, zFPUnaryOp_NEG, zFPUnaryOp_SQRT };

static bool eq_zFPUnaryOp(enum zFPUnaryOp op1, enum zFPUnaryOp op2) { return op1 == op2; }

static enum zFPUnaryOp UNDEFINED(zFPUnaryOp)(unit u) { return zFPUnaryOp_ABS; }

// enum FPType
enum zFPType { zFPType_Nonzzero, zFPType_Zero, zFPType_Infinity, zFPType_QNaN, zFPType_SNaN };

static bool eq_zFPType(enum zFPType op1, enum zFPType op2) { return op1 == op2; }

static enum zFPType UNDEFINED(zFPType)(unit u) { return zFPType_Nonzzero; }

// enum FPRounding
enum zFPRounding { zFPRounding_TIEEVEN, zFPRounding_POSINF, zFPRounding_NEGINF, zFPRounding_ZERO, zFPRounding_TIEAWAY, zFPRounding_ODD };

static bool eq_zFPRounding(enum zFPRounding op1, enum zFPRounding op2) { return op1 == op2; }

static enum zFPRounding UNDEFINED(zFPRounding)(unit u) { return zFPRounding_TIEEVEN; }

// enum FPMaxMinOp
enum zFPMaxMinOp { zFPMaxMinOp_MAX, zFPMaxMinOp_MIN, zFPMaxMinOp_MAXNUM, zFPMaxMinOp_MINNUM };

static bool eq_zFPMaxMinOp(enum zFPMaxMinOp op1, enum zFPMaxMinOp op2) { return op1 == op2; }

static enum zFPMaxMinOp UNDEFINED(zFPMaxMinOp)(unit u) { return zFPMaxMinOp_MAX; }

// enum FPExc
enum zFPExc { zFPExc_InvalidOp, zFPExc_DivideByZero, zFPExc_Overflow, zFPExc_Underflow, zFPExc_Inexact, zFPExc_InputDenorm };

static bool eq_zFPExc(enum zFPExc op1, enum zFPExc op2) { return op1 == op2; }

static enum zFPExc UNDEFINED(zFPExc)(unit u) { return zFPExc_InvalidOp; }

// enum FPConvOp
enum zFPConvOp { zFPConvOp_CVT_FtoI, zFPConvOp_CVT_ItoF, zFPConvOp_MOV_FtoI, zFPConvOp_MOV_ItoF };

static bool eq_zFPConvOp(enum zFPConvOp op1, enum zFPConvOp op2) { return op1 == op2; }

static enum zFPConvOp UNDEFINED(zFPConvOp)(unit u) { return zFPConvOp_CVT_FtoI; }

// enum ExtendType
enum zExtendType { zExtendType_SXTB, zExtendType_SXTH, zExtendType_SXTW, zExtendType_SXTX, zExtendType_UXTB, zExtendType_UXTH, zExtendType_UXTW, zExtendType_UXTX };

static bool eq_zExtendType(enum zExtendType op1, enum zExtendType op2) { return op1 == op2; }

static enum zExtendType UNDEFINED(zExtendType)(unit u) { return zExtendType_SXTB; }

// enum Exception
enum zException { zException_Uncategorizzed, zException_WFxTrap, zException_CP15RTTrap, zException_CP15RRTTrap, zException_CP14RTTrap, zException_CP14DTTrap, zException_AdvSIMDFPAccessTrap, zException_FPIDTrap, zException_CP14RRTTrap, zException_IllegalState, zException_SupervisorCall, zException_HypervisorCall, zException_MonitorCall, zException_SystemRegisterTrap, zException_InstructionAbort, zException_PCAlignment, zException_DataAbort, zException_SPAlignment, zException_FPTrappedException, zException_SError, zException_Breakpoint, zException_SoftwareStep, zException_Watchpoint, zException_SoftwareBreakpoint, zException_VectorCatch, zException_IRQ, zException_CapabilitySysRegTrap, zException_CapabilityAccess, zException_FIQ };

static bool eq_zException(enum zException op1, enum zException op2) { return op1 == op2; }

static enum zException UNDEFINED(zException)(unit u) { return zException_Uncategorizzed; }

// enum DeviceType
enum zDeviceType { zDeviceType_GRE, zDeviceType_nGRE, zDeviceType_nGnRE, zDeviceType_nGnRnE };

static bool eq_zDeviceType(enum zDeviceType op1, enum zDeviceType op2) { return op1 == op2; }

static enum zDeviceType UNDEFINED(zDeviceType)(unit u) { return zDeviceType_GRE; }

// enum CrossTriggerIn
enum zCrossTriggerIn { zCrossTriggerIn_CrossHalt, zCrossTriggerIn_PMUOverflow, zCrossTriggerIn_RSVD2, zCrossTriggerIn_RSVD3, zCrossTriggerIn_TraceExtOut0, zCrossTriggerIn_TraceExtOut1, zCrossTriggerIn_TraceExtOut2, zCrossTriggerIn_TraceExtOut3 };

static bool eq_zCrossTriggerIn(enum zCrossTriggerIn op1, enum zCrossTriggerIn op2) { return op1 == op2; }

static enum zCrossTriggerIn UNDEFINED(zCrossTriggerIn)(unit u) { return zCrossTriggerIn_CrossHalt; }

// enum CountOp
enum zCountOp { zCountOp_CLZ, zCountOp_CLS, zCountOp_CNT };

static bool eq_zCountOp(enum zCountOp op1, enum zCountOp op2) { return op1 == op2; }

static enum zCountOp UNDEFINED(zCountOp)(unit u) { return zCountOp_CLZ; }

// enum Constraint
enum zConstraint { zConstraint_NONE, zConstraint_UNKNOWN, zConstraint_UNDEF, zConstraint_UNDEFEL0, zConstraint_NOP, zConstraint_TRUE, zConstraint_FALSE, zConstraint_DISABLED, zConstraint_UNCOND, zConstraint_COND, zConstraint_ADDITIONAL_DECODE, zConstraint_WBSUPPRESS, zConstraint_FAULT, zConstraint_FORCE, zConstraint_FORCENOSLCHECK };

static bool eq_zConstraint(enum zConstraint op1, enum zConstraint op2) { return op1 == op2; }

static enum zConstraint UNDEFINED(zConstraint)(unit u) { return zConstraint_NONE; }

// enum CompareOp
enum zCompareOp { zCompareOp_GT, zCompareOp_GE, zCompareOp_EQ, zCompareOp_LE, zCompareOp_LT };

static bool eq_zCompareOp(enum zCompareOp op1, enum zCompareOp op2) { return op1 == op2; }

static enum zCompareOp UNDEFINED(zCompareOp)(unit u) { return zCompareOp_GT; }

// enum BranchType
enum zBranchType { zBranchType_DIRCALL, zBranchType_INDCALL, zBranchType_ERET, zBranchType_DBGEXIT, zBranchType_RET, zBranchType_DIR, zBranchType_INDIR, zBranchType_EXCEPTION, zBranchType_RESET, zBranchType_UNKNOWN };

static bool eq_zBranchType(enum zBranchType op1, enum zBranchType op2) { return op1 == op2; }

static enum zBranchType UNDEFINED(zBranchType)(unit u) { return zBranchType_DIRCALL; }

// enum ArchVersion
enum zArchVersion { zARMv8p0, zARMv8p1, zARMv8p2 };

static bool eq_zArchVersion(enum zArchVersion op1, enum zArchVersion op2) { return op1 == op2; }

static enum zArchVersion UNDEFINED(zArchVersion)(unit u) { return zARMv8p0; }

// enum AccType
enum zAccType { zAccType_NORMAL, zAccType_VEC, zAccType_STREAM, zAccType_VECSTREAM, zAccType_ATOMIC, zAccType_ATOMICRW, zAccType_ORDERED, zAccType_ORDEREDRW, zAccType_ORDEREDATOMIC, zAccType_ORDEREDATOMICRW, zAccType_LIMITEDORDERED, zAccType_UNPRIV, zAccType_IFETCH, zAccType_PTW, zAccType_DC, zAccType_DC_UNPRIV, zAccType_IC, zAccType_DCZVA, zAccType_AT };

static bool eq_zAccType(enum zAccType op1, enum zAccType op2) { return op1 == op2; }

static enum zAccType UNDEFINED(zAccType)(unit u) { return zAccType_NORMAL; }





struct zz5vecz8z5bv64z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5bv64z9 zz5vecz8z5bv64z9;

static void CREATE(zz5vecz8z5bv64z9)(zz5vecz8z5bv64z9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5bv64z9)(zz5vecz8z5bv64z9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void RECREATE(zz5vecz8z5bv64z9)(zz5vecz8z5bv64z9 *rop) { KILL(zz5vecz8z5bv64z9)(rop); CREATE(zz5vecz8z5bv64z9)(rop); }

static void undefined_vector_zz5vecz8z5bv64z9(zz5vecz8z5bv64z9 *rop, sail_int len, uint64_t elem) {
  rop->len = sail_int_get_ui(len);
  rop->data = sail_malloc((rop->len) * sizeof(uint64_t));
  for (int i = 0; i < (rop->len); i++) {
    (rop->data)[i] = elem;
  }
}

static uint64_t vector_access_zz5vecz8z5bv64z9(zz5vecz8z5bv64z9 op, sail_int n) {
  int m = sail_int_get_ui(n);
  return op.data[m];
}

static void COPY(zz5vecz8z5bv64z9)(zz5vecz8z5bv64z9 *rop, zz5vecz8z5bv64z9 op) {
  KILL(zz5vecz8z5bv64z9)(rop);
  rop->len = op.len;
  rop->data = sail_malloc((rop->len) * sizeof(uint64_t));
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void vector_update_zz5vecz8z5bv64z9(zz5vecz8z5bv64z9 *rop, zz5vecz8z5bv64z9 op, sail_int n, uint64_t elem) {
  int m = sail_int_get_ui(n);
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv64z9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5bv64z9)(const zz5vecz8z5bv64z9 op1, const zz5vecz8z5bv64z9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(fbits)(op1.data[i], op2.data[i]);  }
  return result;
}

static void length_zz5vecz8z5bv64z9(sail_int *rop, zz5vecz8z5bv64z9 op) {
  mpz_set_ui(*rop, (unsigned long int)(op.len));
}

static void internal_vector_update_zz5vecz8z5bv64z9(zz5vecz8z5bv64z9 *rop, zz5vecz8z5bv64z9 op, const int64_t n, uint64_t elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5bv64z9(zz5vecz8z5bv64z9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_malloc(len * sizeof(uint64_t));
}



struct zz5vecz8z5bv32z9 {
  size_t len;
  uint64_t *data;
};
typedef struct zz5vecz8z5bv32z9 zz5vecz8z5bv32z9;

static void CREATE(zz5vecz8z5bv32z9)(zz5vecz8z5bv32z9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5bv32z9)(zz5vecz8z5bv32z9 *rop) {
  if (rop->data != NULL) sail_free(rop->data);
}

static void RECREATE(zz5vecz8z5bv32z9)(zz5vecz8z5bv32z9 *rop) { KILL(zz5vecz8z5bv32z9)(rop); CREATE(zz5vecz8z5bv32z9)(rop); }

static void undefined_vector_zz5vecz8z5bv32z9(zz5vecz8z5bv32z9 *rop, sail_int len, uint64_t elem) {
  rop->len = sail_int_get_ui(len);
  rop->data = sail_malloc((rop->len) * sizeof(uint64_t));
  for (int i = 0; i < (rop->len); i++) {
    (rop->data)[i] = elem;
  }
}

static uint64_t vector_access_zz5vecz8z5bv32z9(zz5vecz8z5bv32z9 op, sail_int n) {
  int m = sail_int_get_ui(n);
  return op.data[m];
}

static void COPY(zz5vecz8z5bv32z9)(zz5vecz8z5bv32z9 *rop, zz5vecz8z5bv32z9 op) {
  KILL(zz5vecz8z5bv32z9)(rop);
  rop->len = op.len;
  rop->data = sail_malloc((rop->len) * sizeof(uint64_t));
  for (int i = 0; i < op.len; i++) {
    (rop->data)[i] = op.data[i];
  }
}

static void vector_update_zz5vecz8z5bv32z9(zz5vecz8z5bv32z9 *rop, zz5vecz8z5bv32z9 op, sail_int n, uint64_t elem) {
  int m = sail_int_get_ui(n);
  if (rop->data == op.data) {
    rop->data[m] = elem;
  } else {
    COPY(zz5vecz8z5bv32z9)(rop, op);
    rop->data[m] = elem;
  }
}

static bool EQUAL(zz5vecz8z5bv32z9)(const zz5vecz8z5bv32z9 op1, const zz5vecz8z5bv32z9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(fbits)(op1.data[i], op2.data[i]);  }
  return result;
}

static void length_zz5vecz8z5bv32z9(sail_int *rop, zz5vecz8z5bv32z9 op) {
  mpz_set_ui(*rop, (unsigned long int)(op.len));
}

static void internal_vector_update_zz5vecz8z5bv32z9(zz5vecz8z5bv32z9 *rop, zz5vecz8z5bv32z9 op, const int64_t n, uint64_t elem) {
  rop->data[n] = elem;
}

static void internal_vector_init_zz5vecz8z5bv32z9(zz5vecz8z5bv32z9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_malloc(len * sizeof(uint64_t));
}



struct zz5vecz8z5bvz9 {
  size_t len;
  lbits *data;
};
typedef struct zz5vecz8z5bvz9 zz5vecz8z5bvz9;

static void CREATE(zz5vecz8z5bvz9)(zz5vecz8z5bvz9 *rop) {
  rop->len = 0;
  rop->data = NULL;
}

static void KILL(zz5vecz8z5bvz9)(zz5vecz8z5bvz9 *rop) {
  for (int i = 0; i < (rop->len); i++) {
    KILL(lbits)((rop->data) + i);
  }
  if (rop->data != NULL) sail_free(rop->data);
}

static void RECREATE(zz5vecz8z5bvz9)(zz5vecz8z5bvz9 *rop) { KILL(zz5vecz8z5bvz9)(rop); CREATE(zz5vecz8z5bvz9)(rop); }

static void undefined_vector_zz5vecz8z5bvz9(zz5vecz8z5bvz9 *rop, sail_int len, lbits elem) {
  rop->len = sail_int_get_ui(len);
  rop->data = sail_malloc((rop->len) * sizeof(lbits));
  for (int i = 0; i < (rop->len); i++) {
    CREATE(lbits)((rop->data) + i);
    COPY(lbits)((rop->data) + i, elem);
  }
}

static void vector_access_zz5vecz8z5bvz9(lbits *rop, zz5vecz8z5bvz9 op, sail_int n) {
  int m = sail_int_get_ui(n);
  COPY(lbits)(rop, op.data[m]);
}

static void COPY(zz5vecz8z5bvz9)(zz5vecz8z5bvz9 *rop, zz5vecz8z5bvz9 op) {
  KILL(zz5vecz8z5bvz9)(rop);
  rop->len = op.len;
  rop->data = sail_malloc((rop->len) * sizeof(lbits));
  for (int i = 0; i < op.len; i++) {
    CREATE(lbits)((rop->data) + i);
    COPY(lbits)((rop->data) + i, op.data[i]);
  }
}

static void vector_update_zz5vecz8z5bvz9(zz5vecz8z5bvz9 *rop, zz5vecz8z5bvz9 op, sail_int n, lbits elem) {
  int m = sail_int_get_ui(n);
  if (rop->data == op.data) {
  COPY(lbits)((rop->data) + m, elem);
  } else {
    COPY(zz5vecz8z5bvz9)(rop, op);
  COPY(lbits)((rop->data) + m, elem);
  }
}

static bool EQUAL(zz5vecz8z5bvz9)(const zz5vecz8z5bvz9 op1, const zz5vecz8z5bvz9 op2) {
  if (op1.len != op2.len) return false;
  bool result = true;  for (int i = 0; i < op1.len; i++) {
    result &= EQUAL(lbits)(op1.data[i], op2.data[i]);  }
  return result;
}

static void length_zz5vecz8z5bvz9(sail_int *rop, zz5vecz8z5bvz9 op) {
  mpz_set_ui(*rop, (unsigned long int)(op.len));
}

static void internal_vector_update_zz5vecz8z5bvz9(zz5vecz8z5bvz9 *rop, zz5vecz8z5bvz9 op, const int64_t n, lbits elem) {
  COPY(lbits)((rop->data) + n, elem);
}

static void internal_vector_init_zz5vecz8z5bvz9(zz5vecz8z5bvz9 *rop, const int64_t len) {
  rop->len = len;
  rop->data = sail_malloc(len * sizeof(lbits));
  for (int i = 0; i < len; i++) {
    CREATE(lbits)((rop->data) + i);
  }
}















static bool sailgen_neq_int(sail_int, sail_int);

static bool sailgen_neq_int(sail_int zx, sail_int zy)
{
  __label__ end_function_1, end_block_exception_2;

  bool zcbz30;
  bool zgaz30;
  zgaz30 = eq_int(zx, zy);
  zcbz30 = not(zgaz30);

end_function_1: ;
  return zcbz30;
end_block_exception_2: ;

  return false;
}









static void sailgen___id(sail_int *rop, sail_int);

static void sailgen___id(sail_int *zcbz31, sail_int zx)
{
  __label__ end_function_4, end_block_exception_5, end_function_294;

  COPY(sail_int)((*(&zcbz31)), zx);
end_function_4: ;
  goto end_function_294;
end_block_exception_5: ;
  goto end_function_294;
end_function_294: ;
}















static bool sailgen_neq_bits(lbits zx, lbits zy)
{
  __label__ end_function_7, end_block_exception_8;

  bool zcbz32;
  bool zgaz31;
  zgaz31 = eq_bits(zx, zy);
  zcbz32 = not(zgaz31);

end_function_7: ;
  return zcbz32;
end_block_exception_8: ;

  return false;
}































static void sailgen_sail_ones(lbits *rop, sail_int);

lbits zghz30;

static void startup_sailgen_sail_ones(void)
{    CREATE(lbits)(&zghz30);
}

static void sailgen_sail_ones(lbits *zcbz33, sail_int zn)
{
  __label__ end_function_10, end_block_exception_11, end_function_293;

  RECREATE(lbits)(&zghz30);
  zeros(&zghz30, zn);
  not_bits((*(&zcbz33)), zghz30);
end_function_10: ;
  goto end_function_293;
end_block_exception_11: ;
  goto end_function_293;
end_function_293: ;
}



static void finish_sailgen_sail_ones(void)
{    KILL(lbits)(&zghz30);
}









static bool sailgen_eq_bits_int(lbits, sail_int);

sail_int zghz31;

static void startup_sailgen_eq_bits_int(void)
{    CREATE(sail_int)(&zghz31);
}

static bool sailgen_eq_bits_int(lbits zx, sail_int zy)
{
  __label__ end_function_13, end_block_exception_14;

  bool zcbz34;
  RECREATE(sail_int)(&zghz31);
  sail_unsigned(&zghz31, zx);
  zcbz34 = eq_int(zghz31, zy);
end_function_13: ;
  return zcbz34;
end_block_exception_14: ;

  return false;
}



static void finish_sailgen_eq_bits_int(void)
{    KILL(sail_int)(&zghz31);
}



static void sailgen_Ones(lbits *rop, sail_int);

static void sailgen_Ones(lbits *zcbz35, sail_int zn)
{
  __label__ end_function_16, end_block_exception_17, end_function_292;

  sailgen_sail_ones((*(&zcbz35)), zn);
end_function_16: ;
  goto end_function_292;
end_block_exception_17: ;
  goto end_function_292;
end_function_292: ;
}

static void sailgen_Zeros(lbits *rop, sail_int);

static void sailgen_Zeros(lbits *zcbz36, sail_int zn)
{
  __label__ end_function_19, end_block_exception_20, end_function_291;

  zeros((*(&zcbz36)), zn);
end_function_19: ;
  goto end_function_291;
end_block_exception_20: ;
  goto end_function_291;
end_function_291: ;
}

static fbits sailgen_Bit(uint64_t);

static fbits sailgen_Bit(uint64_t zb)
{
  __label__ end_function_22, end_block_exception_23;

  fbits zcbz37;
  zcbz37 = (UINT64_C(1) & (zb >> INT64_C(0)));
end_function_22: ;
  return zcbz37;
end_block_exception_23: ;

  return UINT64_C(0);
}

static void sailgen_integer_subrange(lbits *rop, sail_int, sail_int, sail_int);

sail_int zghz32;
sail_int zghz33;
sail_int zghz34;

static void startup_sailgen_integer_subrange(void)
{
  CREATE(sail_int)(&zghz32);
  CREATE(sail_int)(&zghz33);
  CREATE(sail_int)(&zghz34);
}

static void sailgen_integer_subrange(lbits *zcbz38, sail_int zi, sail_int zhi, sail_int zlo)
{
  __label__ end_function_25, end_block_exception_26, end_function_290;

  RECREATE(sail_int)(&zghz32);
  {
    RECREATE(sail_int)(&zghz33);
    sub_int(&zghz33, zhi, zlo);
    {
      RECREATE(sail_int)(&zghz34);
      CONVERT_OF(sail_int, mach_int)(&zghz34, INT64_C(1));
      add_int(&zghz32, zghz33, zghz34);
    }
  }
  get_slice_int((*(&zcbz38)), zghz32, zi, zlo);
end_function_25: ;
  goto end_function_290;
end_block_exception_26: ;
  goto end_function_290;
end_function_290: ;
}



static void finish_sailgen_integer_subrange(void)
{
  KILL(sail_int)(&zghz34);
  KILL(sail_int)(&zghz33);
  KILL(sail_int)(&zghz32);
}







static void sailgen_Slice_int(lbits *rop, sail_int, sail_int, sail_int);

static void sailgen_Slice_int(lbits *zcbz39, sail_int zi, sail_int zl, sail_int zn)
{
  __label__ end_function_28, end_block_exception_29, end_function_289;

  get_slice_int((*(&zcbz39)), zn, zi, zl);
end_function_28: ;
  goto end_function_289;
end_block_exception_29: ;
  goto end_function_289;
end_function_289: ;
}

static enum zsignal sailgen_undefined_signal(unit);

static enum zsignal sailgen_undefined_signal(unit zgsz30)
{
  __label__ end_function_31, end_block_exception_32;

  enum zsignal zcbz310;
  zcbz310 = zLOW;
end_function_31: ;
  return zcbz310;
end_block_exception_32: ;

  return zHIGH;
}

static unit sailgen___SetConfig(sail_string, sail_int);

static unit sailgen___ListConfig(unit);

// register SEE
static sail_int zSEE;

// register PCC
static lbits zPCC;

// register CNTHPS_CTL_EL2
static uint64_t zCNTHPS_CTL_EL2;

// register CNTHPS_CVAL_EL2
static uint64_t zCNTHPS_CVAL_EL2;

// register CNTHPS_TVAL_EL2
static uint64_t zCNTHPS_TVAL_EL2;

// register CNTHVS_CTL_EL2
static uint64_t zCNTHVS_CTL_EL2;

// register CNTHVS_CVAL_EL2
static uint64_t zCNTHVS_CVAL_EL2;

// register CNTHVS_TVAL_EL2
static uint64_t zCNTHVS_TVAL_EL2;

static uint64_t zEL1;
static void create_letbind_0(void) {


  uint64_t zgsz31;
  zgsz31 = UINT64_C(0b01);
  zEL1 = zgsz31;

let_end_33: ;
}
static void kill_letbind_0(void) {
}

static uint64_t zEL2;
static void create_letbind_1(void) {


  uint64_t zgsz32;
  zgsz32 = UINT64_C(0b10);
  zEL2 = zgsz32;

let_end_34: ;
}
static void kill_letbind_1(void) {
}

static uint64_t zEL3;
static void create_letbind_2(void) {


  uint64_t zgsz33;
  zgsz33 = UINT64_C(0b11);
  zEL3 = zgsz33;

let_end_35: ;
}
static void kill_letbind_2(void) {
}

// register SCR_EL3
static uint64_t zSCR_EL3;

// register HCR_EL2
static uint64_t zHCR_EL2;

static struct zProcState sailgen_undefined_ProcState(unit);

struct zProcState zghz35;
sail_int zghz36;
lbits zghz37;
sail_int zghz38;
lbits zghz39;
sail_int zghz310;
lbits zghz311;
sail_int zghz312;
lbits zghz313;
sail_int zghz314;
lbits zghz315;
sail_int zghz316;
lbits zghz317;
sail_int zghz318;
lbits zghz319;
sail_int zghz320;
lbits zghz321;
sail_int zghz322;
lbits zghz323;
sail_int zghz324;
lbits zghz325;
sail_int zghz326;
lbits zghz327;
sail_int zghz328;
lbits zghz329;
sail_int zghz330;
lbits zghz331;
sail_int zghz332;
lbits zghz333;
sail_int zghz334;
lbits zghz335;
sail_int zghz336;
lbits zghz337;
sail_int zghz338;
lbits zghz339;
sail_int zghz340;
lbits zghz341;
sail_int zghz342;
lbits zghz343;
sail_int zghz344;
lbits zghz345;
sail_int zghz346;
lbits zghz347;
sail_int zghz348;
lbits zghz349;
sail_int zghz350;
lbits zghz351;
sail_int zghz352;
lbits zghz353;
sail_int zghz354;
lbits zghz355;
sail_int zghz356;
lbits zghz357;

static void startup_sailgen_undefined_ProcState(void)
{

  CREATE(sail_int)(&zghz36);
  CREATE(lbits)(&zghz37);
  CREATE(sail_int)(&zghz38);
  CREATE(lbits)(&zghz39);
  CREATE(sail_int)(&zghz310);
  CREATE(lbits)(&zghz311);
  CREATE(sail_int)(&zghz312);
  CREATE(lbits)(&zghz313);
  CREATE(sail_int)(&zghz314);
  CREATE(lbits)(&zghz315);
  CREATE(sail_int)(&zghz316);
  CREATE(lbits)(&zghz317);
  CREATE(sail_int)(&zghz318);
  CREATE(lbits)(&zghz319);
  CREATE(sail_int)(&zghz320);
  CREATE(lbits)(&zghz321);
  CREATE(sail_int)(&zghz322);
  CREATE(lbits)(&zghz323);
  CREATE(sail_int)(&zghz324);
  CREATE(lbits)(&zghz325);
  CREATE(sail_int)(&zghz326);
  CREATE(lbits)(&zghz327);
  CREATE(sail_int)(&zghz328);
  CREATE(lbits)(&zghz329);
  CREATE(sail_int)(&zghz330);
  CREATE(lbits)(&zghz331);
  CREATE(sail_int)(&zghz332);
  CREATE(lbits)(&zghz333);
  CREATE(sail_int)(&zghz334);
  CREATE(lbits)(&zghz335);
  CREATE(sail_int)(&zghz336);
  CREATE(lbits)(&zghz337);
  CREATE(sail_int)(&zghz338);
  CREATE(lbits)(&zghz339);
  CREATE(sail_int)(&zghz340);
  CREATE(lbits)(&zghz341);
  CREATE(sail_int)(&zghz342);
  CREATE(lbits)(&zghz343);
  CREATE(sail_int)(&zghz344);
  CREATE(lbits)(&zghz345);
  CREATE(sail_int)(&zghz346);
  CREATE(lbits)(&zghz347);
  CREATE(sail_int)(&zghz348);
  CREATE(lbits)(&zghz349);
  CREATE(sail_int)(&zghz350);
  CREATE(lbits)(&zghz351);
  CREATE(sail_int)(&zghz352);
  CREATE(lbits)(&zghz353);
  CREATE(sail_int)(&zghz354);
  CREATE(lbits)(&zghz355);
  CREATE(sail_int)(&zghz356);
  CREATE(lbits)(&zghz357);
}

static struct zProcState sailgen_undefined_ProcState(unit zgsz34)
{
  __label__ end_function_37, end_block_exception_38;

  struct zProcState zghz35;
  uint64_t zgaz36;
  {
    RECREATE(sail_int)(&zghz356);
    CONVERT_OF(sail_int, mach_int)(&zghz356, INT64_C(1));
    RECREATE(lbits)(&zghz357);
    UNDEFINED(lbits)(&zghz357, zghz356);
    zgaz36 = CONVERT_OF(fbits, lbits)(zghz357, true);
  }
  uint64_t zgaz37;
  {
    RECREATE(sail_int)(&zghz354);
    CONVERT_OF(sail_int, mach_int)(&zghz354, INT64_C(1));
    RECREATE(lbits)(&zghz355);
    UNDEFINED(lbits)(&zghz355, zghz354);
    zgaz37 = CONVERT_OF(fbits, lbits)(zghz355, true);
  }
  uint64_t zgaz38;
  {
    RECREATE(sail_int)(&zghz352);
    CONVERT_OF(sail_int, mach_int)(&zghz352, INT64_C(1));
    RECREATE(lbits)(&zghz353);
    UNDEFINED(lbits)(&zghz353, zghz352);
    zgaz38 = CONVERT_OF(fbits, lbits)(zghz353, true);
  }
  uint64_t zgaz39;
  {
    RECREATE(sail_int)(&zghz350);
    CONVERT_OF(sail_int, mach_int)(&zghz350, INT64_C(1));
    RECREATE(lbits)(&zghz351);
    UNDEFINED(lbits)(&zghz351, zghz350);
    zgaz39 = CONVERT_OF(fbits, lbits)(zghz351, true);
  }
  uint64_t zgaz310;
  {
    RECREATE(sail_int)(&zghz348);
    CONVERT_OF(sail_int, mach_int)(&zghz348, INT64_C(1));
    RECREATE(lbits)(&zghz349);
    UNDEFINED(lbits)(&zghz349, zghz348);
    zgaz310 = CONVERT_OF(fbits, lbits)(zghz349, true);
  }
  uint64_t zgaz311;
  {
    RECREATE(sail_int)(&zghz346);
    CONVERT_OF(sail_int, mach_int)(&zghz346, INT64_C(1));
    RECREATE(lbits)(&zghz347);
    UNDEFINED(lbits)(&zghz347, zghz346);
    zgaz311 = CONVERT_OF(fbits, lbits)(zghz347, true);
  }
  uint64_t zgaz312;
  {
    RECREATE(sail_int)(&zghz344);
    CONVERT_OF(sail_int, mach_int)(&zghz344, INT64_C(1));
    RECREATE(lbits)(&zghz345);
    UNDEFINED(lbits)(&zghz345, zghz344);
    zgaz312 = CONVERT_OF(fbits, lbits)(zghz345, true);
  }
  uint64_t zgaz313;
  {
    RECREATE(sail_int)(&zghz342);
    CONVERT_OF(sail_int, mach_int)(&zghz342, INT64_C(1));
    RECREATE(lbits)(&zghz343);
    UNDEFINED(lbits)(&zghz343, zghz342);
    zgaz313 = CONVERT_OF(fbits, lbits)(zghz343, true);
  }
  uint64_t zgaz314;
  {
    RECREATE(sail_int)(&zghz340);
    CONVERT_OF(sail_int, mach_int)(&zghz340, INT64_C(1));
    RECREATE(lbits)(&zghz341);
    UNDEFINED(lbits)(&zghz341, zghz340);
    zgaz314 = CONVERT_OF(fbits, lbits)(zghz341, true);
  }
  uint64_t zgaz315;
  {
    RECREATE(sail_int)(&zghz338);
    CONVERT_OF(sail_int, mach_int)(&zghz338, INT64_C(1));
    RECREATE(lbits)(&zghz339);
    UNDEFINED(lbits)(&zghz339, zghz338);
    zgaz315 = CONVERT_OF(fbits, lbits)(zghz339, true);
  }
  uint64_t zgaz316;
  {
    RECREATE(sail_int)(&zghz336);
    CONVERT_OF(sail_int, mach_int)(&zghz336, INT64_C(1));
    RECREATE(lbits)(&zghz337);
    UNDEFINED(lbits)(&zghz337, zghz336);
    zgaz316 = CONVERT_OF(fbits, lbits)(zghz337, true);
  }
  uint64_t zgaz317;
  {
    RECREATE(sail_int)(&zghz334);
    CONVERT_OF(sail_int, mach_int)(&zghz334, INT64_C(1));
    RECREATE(lbits)(&zghz335);
    UNDEFINED(lbits)(&zghz335, zghz334);
    zgaz317 = CONVERT_OF(fbits, lbits)(zghz335, true);
  }
  uint64_t zgaz318;
  {
    RECREATE(sail_int)(&zghz332);
    CONVERT_OF(sail_int, mach_int)(&zghz332, INT64_C(1));
    RECREATE(lbits)(&zghz333);
    UNDEFINED(lbits)(&zghz333, zghz332);
    zgaz318 = CONVERT_OF(fbits, lbits)(zghz333, true);
  }
  uint64_t zgaz319;
  {
    RECREATE(sail_int)(&zghz330);
    CONVERT_OF(sail_int, mach_int)(&zghz330, INT64_C(1));
    RECREATE(lbits)(&zghz331);
    UNDEFINED(lbits)(&zghz331, zghz330);
    zgaz319 = CONVERT_OF(fbits, lbits)(zghz331, true);
  }
  uint64_t zgaz320;
  {
    RECREATE(sail_int)(&zghz328);
    CONVERT_OF(sail_int, mach_int)(&zghz328, INT64_C(1));
    RECREATE(lbits)(&zghz329);
    UNDEFINED(lbits)(&zghz329, zghz328);
    zgaz320 = CONVERT_OF(fbits, lbits)(zghz329, true);
  }
  uint64_t zgaz321;
  {
    RECREATE(sail_int)(&zghz326);
    CONVERT_OF(sail_int, mach_int)(&zghz326, INT64_C(2));
    RECREATE(lbits)(&zghz327);
    UNDEFINED(lbits)(&zghz327, zghz326);
    zgaz321 = CONVERT_OF(fbits, lbits)(zghz327, true);
  }
  uint64_t zgaz322;
  {
    RECREATE(sail_int)(&zghz324);
    CONVERT_OF(sail_int, mach_int)(&zghz324, INT64_C(1));
    RECREATE(lbits)(&zghz325);
    UNDEFINED(lbits)(&zghz325, zghz324);
    zgaz322 = CONVERT_OF(fbits, lbits)(zghz325, true);
  }
  uint64_t zgaz323;
  {
    RECREATE(sail_int)(&zghz322);
    CONVERT_OF(sail_int, mach_int)(&zghz322, INT64_C(1));
    RECREATE(lbits)(&zghz323);
    UNDEFINED(lbits)(&zghz323, zghz322);
    zgaz323 = CONVERT_OF(fbits, lbits)(zghz323, true);
  }
  uint64_t zgaz324;
  {
    RECREATE(sail_int)(&zghz320);
    CONVERT_OF(sail_int, mach_int)(&zghz320, INT64_C(1));
    RECREATE(lbits)(&zghz321);
    UNDEFINED(lbits)(&zghz321, zghz320);
    zgaz324 = CONVERT_OF(fbits, lbits)(zghz321, true);
  }
  uint64_t zgaz325;
  {
    RECREATE(sail_int)(&zghz318);
    CONVERT_OF(sail_int, mach_int)(&zghz318, INT64_C(4));
    RECREATE(lbits)(&zghz319);
    UNDEFINED(lbits)(&zghz319, zghz318);
    zgaz325 = CONVERT_OF(fbits, lbits)(zghz319, true);
  }
  uint64_t zgaz326;
  {
    RECREATE(sail_int)(&zghz316);
    CONVERT_OF(sail_int, mach_int)(&zghz316, INT64_C(1));
    RECREATE(lbits)(&zghz317);
    UNDEFINED(lbits)(&zghz317, zghz316);
    zgaz326 = CONVERT_OF(fbits, lbits)(zghz317, true);
  }
  uint64_t zgaz327;
  {
    RECREATE(sail_int)(&zghz314);
    CONVERT_OF(sail_int, mach_int)(&zghz314, INT64_C(8));
    RECREATE(lbits)(&zghz315);
    UNDEFINED(lbits)(&zghz315, zghz314);
    zgaz327 = CONVERT_OF(fbits, lbits)(zghz315, true);
  }
  uint64_t zgaz328;
  {
    RECREATE(sail_int)(&zghz312);
    CONVERT_OF(sail_int, mach_int)(&zghz312, INT64_C(1));
    RECREATE(lbits)(&zghz313);
    UNDEFINED(lbits)(&zghz313, zghz312);
    zgaz328 = CONVERT_OF(fbits, lbits)(zghz313, true);
  }
  uint64_t zgaz329;
  {
    RECREATE(sail_int)(&zghz310);
    CONVERT_OF(sail_int, mach_int)(&zghz310, INT64_C(1));
    RECREATE(lbits)(&zghz311);
    UNDEFINED(lbits)(&zghz311, zghz310);
    zgaz329 = CONVERT_OF(fbits, lbits)(zghz311, true);
  }
  uint64_t zgaz330;
  {
    RECREATE(sail_int)(&zghz38);
    CONVERT_OF(sail_int, mach_int)(&zghz38, INT64_C(1));
    RECREATE(lbits)(&zghz39);
    UNDEFINED(lbits)(&zghz39, zghz38);
    zgaz330 = CONVERT_OF(fbits, lbits)(zghz39, true);
  }
  uint64_t zgaz331;
  {
    RECREATE(sail_int)(&zghz36);
    CONVERT_OF(sail_int, mach_int)(&zghz36, INT64_C(5));
    RECREATE(lbits)(&zghz37);
    UNDEFINED(lbits)(&zghz37, zghz36);
    zgaz331 = CONVERT_OF(fbits, lbits)(zghz37, true);
  }
  zghz35.zA = zgaz311;
  zghz35.zC = zgaz38;
  zghz35.zC64 = zgaz318;
  zghz35.zD = zgaz310;
  zghz35.zDIT = zgaz316;
  zghz35.zE = zgaz330;
  zghz35.zEL = zgaz321;
  zghz35.zF = zgaz313;
  zghz35.zGE = zgaz325;
  zghz35.zI = zgaz312;
  zghz35.zIL = zgaz320;
  zghz35.zIT = zgaz327;
  zghz35.zJ = zgaz328;
  zghz35.zM = zgaz331;
  zghz35.zN = zgaz36;
  zghz35.zPAN = zgaz314;
  zghz35.zQ = zgaz324;
  zghz35.zSP = zgaz323;
  zghz35.zSS = zgaz319;
  zghz35.zSSBS = zgaz326;
  zghz35.zT = zgaz329;
  zghz35.zTCO = zgaz317;
  zghz35.zUAO = zgaz315;
  zghz35.zV = zgaz39;
  zghz35.zZ = zgaz37;
  zghz35.znRW = zgaz322;


























end_function_37: ;
  return zghz35;
end_block_exception_38: ;
  struct zProcState zcbz340 = { .znRW = UINT64_C(0xdeadc0de), .zZ = UINT64_C(0xdeadc0de), .zV = UINT64_C(0xdeadc0de), .zUAO = UINT64_C(0xdeadc0de), .zTCO = UINT64_C(0xdeadc0de), .zT = UINT64_C(0xdeadc0de), .zSSBS = UINT64_C(0xdeadc0de), .zSS = UINT64_C(0xdeadc0de), .zSP = UINT64_C(0xdeadc0de), .zQ = UINT64_C(0xdeadc0de), .zPAN = UINT64_C(0xdeadc0de), .zN = UINT64_C(0xdeadc0de), .zM = UINT64_C(0xdeadc0de), .zJ = UINT64_C(0xdeadc0de), .zIT = UINT64_C(0xdeadc0de), .zIL = UINT64_C(0xdeadc0de), .zI = UINT64_C(0xdeadc0de), .zGE = UINT64_C(0xdeadc0de), .zF = UINT64_C(0xdeadc0de), .zEL = UINT64_C(0xdeadc0de), .zE = UINT64_C(0xdeadc0de), .zDIT = UINT64_C(0xdeadc0de), .zD = UINT64_C(0xdeadc0de), .zC64 = UINT64_C(0xdeadc0de), .zC = UINT64_C(0xdeadc0de), .zA = UINT64_C(0xdeadc0de) };
  return zcbz340;
}



static void finish_sailgen_undefined_ProcState(void)
{
  KILL(lbits)(&zghz357);
  KILL(sail_int)(&zghz356);
  KILL(lbits)(&zghz355);
  KILL(sail_int)(&zghz354);
  KILL(lbits)(&zghz353);
  KILL(sail_int)(&zghz352);
  KILL(lbits)(&zghz351);
  KILL(sail_int)(&zghz350);
  KILL(lbits)(&zghz349);
  KILL(sail_int)(&zghz348);
  KILL(lbits)(&zghz347);
  KILL(sail_int)(&zghz346);
  KILL(lbits)(&zghz345);
  KILL(sail_int)(&zghz344);
  KILL(lbits)(&zghz343);
  KILL(sail_int)(&zghz342);
  KILL(lbits)(&zghz341);
  KILL(sail_int)(&zghz340);
  KILL(lbits)(&zghz339);
  KILL(sail_int)(&zghz338);
  KILL(lbits)(&zghz337);
  KILL(sail_int)(&zghz336);
  KILL(lbits)(&zghz335);
  KILL(sail_int)(&zghz334);
  KILL(lbits)(&zghz333);
  KILL(sail_int)(&zghz332);
  KILL(lbits)(&zghz331);
  KILL(sail_int)(&zghz330);
  KILL(lbits)(&zghz329);
  KILL(sail_int)(&zghz328);
  KILL(lbits)(&zghz327);
  KILL(sail_int)(&zghz326);
  KILL(lbits)(&zghz325);
  KILL(sail_int)(&zghz324);
  KILL(lbits)(&zghz323);
  KILL(sail_int)(&zghz322);
  KILL(lbits)(&zghz321);
  KILL(sail_int)(&zghz320);
  KILL(lbits)(&zghz319);
  KILL(sail_int)(&zghz318);
  KILL(lbits)(&zghz317);
  KILL(sail_int)(&zghz316);
  KILL(lbits)(&zghz315);
  KILL(sail_int)(&zghz314);
  KILL(lbits)(&zghz313);
  KILL(sail_int)(&zghz312);
  KILL(lbits)(&zghz311);
  KILL(sail_int)(&zghz310);
  KILL(lbits)(&zghz39);
  KILL(sail_int)(&zghz38);
  KILL(lbits)(&zghz37);
  KILL(sail_int)(&zghz36);

}

// register PSTATE
static struct zProcState zPSTATE;

static uint64_t zEL0;
static void create_letbind_3(void) {


  uint64_t zgsz36;
  zgsz36 = UINT64_C(0b00);
  zEL0 = zgsz36;

let_end_39: ;
}
static void kill_letbind_3(void) {
}

// register _R00
static lbits z_R00;

// register _R01
static lbits z_R01;

// register _R02
static lbits z_R02;

// register _R03
static lbits z_R03;

// register _R04
static lbits z_R04;

// register _R05
static lbits z_R05;

// register _R06
static lbits z_R06;

// register _R07
static lbits z_R07;

// register _R08
static lbits z_R08;

// register _R09
static lbits z_R09;

// register _R10
static lbits z_R10;

// register _R11
static lbits z_R11;

// register _R12
static lbits z_R12;

// register _R13
static lbits z_R13;

// register _R14
static lbits z_R14;

// register _R15
static lbits z_R15;

// register _R16
static lbits z_R16;

// register _R17
static lbits z_R17;

// register _R18
static lbits z_R18;

// register _R19
static lbits z_R19;

// register _R20
static lbits z_R20;

// register _R21
static lbits z_R21;

// register _R22
static lbits z_R22;

// register _R23
static lbits z_R23;

// register _R24
static lbits z_R24;

// register _R25
static lbits z_R25;

// register _R26
static lbits z_R26;

// register _R27
static lbits z_R27;

// register _R28
static lbits z_R28;

// register _R29
static lbits z_R29;

// register _R30
static lbits z_R30;

// register ESR_EL1
static uint64_t zESR_EL1;

// register ESR_EL2
static uint64_t zESR_EL2;

// register ESR_EL3
static uint64_t zESR_EL3;

// register FAR_EL1
static uint64_t zFAR_EL1;

// register FAR_EL2
static uint64_t zFAR_EL2;

// register FAR_EL3
static uint64_t zFAR_EL3;

// register HPFAR_EL2
static uint64_t zHPFAR_EL2;

// register TCR_EL1
static uint64_t zTCR_EL1;

// register TCR_EL2
static uint64_t zTCR_EL2;

// register TCR_EL3
static uint64_t zTCR_EL3;

static int64_t zCAP_FLAGS_LO_BIT;
static void create_letbind_4(void) {


  int64_t zgsz37;
  zgsz37 = INT64_C(56);
  zCAP_FLAGS_LO_BIT = zgsz37;

let_end_40: ;
}
static void kill_letbind_4(void) {
}

static int64_t zCAP_VALUE_HI_BIT;
static void create_letbind_5(void) {


  int64_t zgsz38;
  zgsz38 = INT64_C(63);
  zCAP_VALUE_HI_BIT = zgsz38;

let_end_41: ;
}
static void kill_letbind_5(void) {
}

static int64_t zCAP_VALUE_LO_BIT;
static void create_letbind_6(void) {


  int64_t zgsz39;
  zgsz39 = INT64_C(0);
  zCAP_VALUE_LO_BIT = zgsz39;

let_end_42: ;
}
static void kill_letbind_6(void) {
}

static int64_t zCAP_VALUE_NUM_BITS;
static void create_letbind_7(void) {


  int64_t zgsz310;
  int64_t zgaz332;
  {
    sail_int zgsz3610;
    CREATE(sail_int)(&zgsz3610);
    CONVERT_OF(sail_int, mach_int)(&zgsz3610, zCAP_VALUE_HI_BIT);
    sail_int zgsz3611;
    CREATE(sail_int)(&zgsz3611);
    CONVERT_OF(sail_int, mach_int)(&zgsz3611, zCAP_VALUE_LO_BIT);
    sail_int zgsz3612;
    CREATE(sail_int)(&zgsz3612);
    sub_int(&zgsz3612, zgsz3610, zgsz3611);
    zgaz332 = CONVERT_OF(mach_int, sail_int)(zgsz3612);
    KILL(sail_int)(&zgsz3612);
    KILL(sail_int)(&zgsz3611);
    KILL(sail_int)(&zgsz3610);
  }
  zgsz310 = (zgaz332 + INT64_C(1));

  zCAP_VALUE_NUM_BITS = zgsz310;

let_end_43: ;
}
static void kill_letbind_7(void) {
}

static int64_t zCAP_BASE_HI_BIT;
static void create_letbind_8(void) {


  int64_t zgsz311;
  zgsz311 = INT64_C(79);
  zCAP_BASE_HI_BIT = zgsz311;

let_end_44: ;
}
static void kill_letbind_8(void) {
}

static int64_t zCAP_BASE_LO_BIT;
static void create_letbind_9(void) {


  int64_t zgsz312;
  zgsz312 = INT64_C(64);
  zCAP_BASE_LO_BIT = zgsz312;

let_end_45: ;
}
static void kill_letbind_9(void) {
}

static int64_t zCAP_MW;
static void create_letbind_10(void) {


  int64_t zgsz313;
  int64_t zgaz333;
  {
    sail_int zgsz3613;
    CREATE(sail_int)(&zgsz3613);
    CONVERT_OF(sail_int, mach_int)(&zgsz3613, zCAP_BASE_HI_BIT);
    sail_int zgsz3614;
    CREATE(sail_int)(&zgsz3614);
    CONVERT_OF(sail_int, mach_int)(&zgsz3614, zCAP_BASE_LO_BIT);
    sail_int zgsz3615;
    CREATE(sail_int)(&zgsz3615);
    sub_int(&zgsz3615, zgsz3613, zgsz3614);
    zgaz333 = CONVERT_OF(mach_int, sail_int)(zgsz3615);
    KILL(sail_int)(&zgsz3615);
    KILL(sail_int)(&zgsz3614);
    KILL(sail_int)(&zgsz3613);
  }
  zgsz313 = (zgaz333 + INT64_C(1));

  zCAP_MW = zgsz313;

let_end_46: ;
}
static void kill_letbind_10(void) {
}

static bool sailgen_CapBoundsUsesValue(sail_int);

sail_int zghz358;
sail_int zghz359;
sail_int zghz360;

static void startup_sailgen_CapBoundsUsesValue(void)
{
  CREATE(sail_int)(&zghz358);
  CREATE(sail_int)(&zghz359);
  CREATE(sail_int)(&zghz360);
}

static bool sailgen_CapBoundsUsesValue(sail_int zexp)
{
  __label__ cleanup_49, end_cleanup_50, end_function_48, end_block_exception_51;

  bool zcbz312;
  {
    RECREATE(sail_int)(&zghz358);
    {
      RECREATE(sail_int)(&zghz360);
      CONVERT_OF(sail_int, mach_int)(&zghz360, zCAP_MW);
      add_int(&zghz358, zexp, zghz360);
    }
    {
      RECREATE(sail_int)(&zghz359);
      CONVERT_OF(sail_int, mach_int)(&zghz359, zCAP_VALUE_NUM_BITS);
      zcbz312 = lt(zghz358, zghz359);
    }
  }
  goto cleanup_49;
  /* unreachable after return */
  goto end_cleanup_50;
cleanup_49: ;
  goto end_function_48;
end_cleanup_50: ;
end_function_48: ;
  return zcbz312;
end_block_exception_51: ;

  return false;
}



static void finish_sailgen_CapBoundsUsesValue(void)
{
  KILL(sail_int)(&zghz360);
  KILL(sail_int)(&zghz359);
  KILL(sail_int)(&zghz358);
}

static int64_t zCAP_BASE_EXP_HI_BIT;
static void create_letbind_11(void) {


  int64_t zgsz314;
  zgsz314 = INT64_C(66);
  zCAP_BASE_EXP_HI_BIT = zgsz314;

let_end_52: ;
}
static void kill_letbind_11(void) {
}

static int64_t zCAP_LIMIT_EXP_HI_BIT;
static void create_letbind_12(void) {


  int64_t zgsz315;
  zgsz315 = INT64_C(82);
  zCAP_LIMIT_EXP_HI_BIT = zgsz315;

let_end_53: ;
}
static void kill_letbind_12(void) {
}

static int64_t zCAP_LIMIT_LO_BIT;
static void create_letbind_13(void) {


  int64_t zgsz316;
  zgsz316 = INT64_C(80);
  zCAP_LIMIT_LO_BIT = zgsz316;

let_end_54: ;
}
static void kill_letbind_13(void) {
}

static int64_t zCAP_IE_BIT;
static void create_letbind_14(void) {


  int64_t zgsz317;
  zgsz317 = INT64_C(94);
  zCAP_IE_BIT = zgsz317;

let_end_55: ;
}
static void kill_letbind_14(void) {
}

static bool sailgen_CapIsInternalExponent(lbits);

sail_int zghz361;

static void startup_sailgen_CapIsInternalExponent(void)
{    CREATE(sail_int)(&zghz361);
}

static bool sailgen_CapIsInternalExponent(lbits zc)
{
  __label__ cleanup_58, end_cleanup_59, end_function_57, end_block_exception_60;

  bool zcbz313;
  {
    uint64_t zgaz337;
    {
      fbits zgaz336;
      {
        RECREATE(sail_int)(&zghz361);
        CONVERT_OF(sail_int, mach_int)(&zghz361, zCAP_IE_BIT);
        zgaz336 = bitvector_access(zc, zghz361);
      }
      zgaz337 = UINT64_C(0b0);
      zgaz337 = update_fbits(zgaz337, INT64_C(0), zgaz336);
    }
    zcbz313 = (zgaz337 == UINT64_C(0b0));
  }
  goto cleanup_58;
  /* unreachable after return */
  goto end_cleanup_59;
cleanup_58: ;
  goto end_function_57;
end_cleanup_59: ;
end_function_57: ;
  return zcbz313;
end_block_exception_60: ;

  return false;
}



static void finish_sailgen_CapIsInternalExponent(void)
{    KILL(sail_int)(&zghz361);
}

static int64_t sailgen_CapGetExponent(lbits);

lbits zghz362;
sail_int zghz363;
sail_int zghz364;
sail_int zghz365;
lbits zghz366;
sail_int zghz367;
sail_int zghz368;
lbits zghz369;

static void startup_sailgen_CapGetExponent(void)
{
  CREATE(lbits)(&zghz362);
  CREATE(sail_int)(&zghz363);
  CREATE(sail_int)(&zghz364);
  CREATE(sail_int)(&zghz365);
  CREATE(lbits)(&zghz366);
  CREATE(sail_int)(&zghz367);
  CREATE(sail_int)(&zghz368);
  CREATE(lbits)(&zghz369);
}

static int64_t sailgen_CapGetExponent(lbits zc)
{
  __label__ end_function_62, end_block_exception_67;

  int64_t zcbz314;
  bool zgaz339;
  zgaz339 = sailgen_CapIsInternalExponent(zc);
  if (zgaz339) {
    __label__ cleanup_65, end_cleanup_66;

    uint64_t znexp;
    {
      uint64_t zgaz342;
      {
        RECREATE(sail_int)(&zghz367);
        CONVERT_OF(sail_int, mach_int)(&zghz367, zCAP_LIMIT_EXP_HI_BIT);
        RECREATE(sail_int)(&zghz368);
        CONVERT_OF(sail_int, mach_int)(&zghz368, zCAP_LIMIT_LO_BIT);
        RECREATE(lbits)(&zghz369);
        vector_subrange_lbits(&zghz369, zc, zghz367, zghz368);
        zgaz342 = CONVERT_OF(fbits, lbits)(zghz369, true);
      }
      uint64_t zgaz343;
      {
        RECREATE(sail_int)(&zghz364);
        CONVERT_OF(sail_int, mach_int)(&zghz364, zCAP_BASE_EXP_HI_BIT);
        RECREATE(sail_int)(&zghz365);
        CONVERT_OF(sail_int, mach_int)(&zghz365, zCAP_BASE_LO_BIT);
        RECREATE(lbits)(&zghz366);
        vector_subrange_lbits(&zghz366, zc, zghz364, zghz365);
        zgaz343 = CONVERT_OF(fbits, lbits)(zghz366, true);
      }
      znexp = (zgaz342 << 3) | zgaz343;
    }
    int64_t zgaz341;
    {
      uint64_t zgaz340;
      zgaz340 = (~(znexp) & UINT64_C(0b111111));
      {
        RECREATE(lbits)(&zghz362);
        CONVERT_OF(lbits, fbits)(&zghz362, zgaz340, UINT64_C(6) , true);
        RECREATE(sail_int)(&zghz363);
        sail_unsigned(&zghz363, zghz362);
        zgaz341 = CONVERT_OF(mach_int, sail_int)(zghz363);
      }
    }
    zcbz314 = zgaz341;
    goto cleanup_65;
    /* unreachable after return */
    goto end_cleanup_66;
  cleanup_65: ;
    goto end_function_62;
  end_cleanup_66: ;
  } else {
    __label__ cleanup_63, end_cleanup_64;

    zcbz314 = INT64_C(0);
    goto cleanup_63;
    /* unreachable after return */
    goto end_cleanup_64;
  cleanup_63: ;
    goto end_function_62;
  end_cleanup_64: ;
  }

end_function_62: ;
  return zcbz314;
end_block_exception_67: ;

  return INT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetExponent(void)
{
  KILL(lbits)(&zghz369);
  KILL(sail_int)(&zghz368);
  KILL(sail_int)(&zghz367);
  KILL(lbits)(&zghz366);
  KILL(sail_int)(&zghz365);
  KILL(sail_int)(&zghz364);
  KILL(sail_int)(&zghz363);
  KILL(lbits)(&zghz362);
}

static uint64_t sailgen_CapGetValue(lbits);

sail_int zghz370;
sail_int zghz371;
lbits zghz372;

static void startup_sailgen_CapGetValue(void)
{
  CREATE(sail_int)(&zghz370);
  CREATE(sail_int)(&zghz371);
  CREATE(lbits)(&zghz372);
}

static uint64_t sailgen_CapGetValue(lbits zc)
{
  __label__ cleanup_70, end_cleanup_71, end_function_69, end_block_exception_72;

  uint64_t zcbz315;
  {
    RECREATE(sail_int)(&zghz370);
    CONVERT_OF(sail_int, mach_int)(&zghz370, zCAP_VALUE_HI_BIT);
    RECREATE(sail_int)(&zghz371);
    CONVERT_OF(sail_int, mach_int)(&zghz371, zCAP_VALUE_LO_BIT);
    RECREATE(lbits)(&zghz372);
    vector_subrange_lbits(&zghz372, zc, zghz370, zghz371);
    zcbz315 = CONVERT_OF(fbits, lbits)(zghz372, true);
  }
  goto cleanup_70;
  /* unreachable after return */
  goto end_cleanup_71;
cleanup_70: ;
  goto end_function_69;
end_cleanup_71: ;
end_function_69: ;
  return zcbz315;
end_block_exception_72: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetValue(void)
{
  KILL(lbits)(&zghz372);
  KILL(sail_int)(&zghz371);
  KILL(sail_int)(&zghz370);
}

static int64_t zCAP_BOUND_NUM_BITS;
static void create_letbind_15(void) {


  int64_t zgsz319;
  zgsz319 = (zCAP_VALUE_NUM_BITS + INT64_C(1));
  zCAP_BOUND_NUM_BITS = zgsz319;

let_end_73: ;
}
static void kill_letbind_15(void) {
}

static lbits zCAP_BOUND_MAX;
static void create_letbind_16(void) {    CREATE(lbits)(&zCAP_BOUND_MAX);


  lbits zgsz320;
  CREATE(lbits)(&zgsz320);
  sail_int zgaz345;
  CREATE(sail_int)(&zgaz345);
  {
    sail_int zgsz3632;
    CREATE(sail_int)(&zgsz3632);
    CONVERT_OF(sail_int, mach_int)(&zgsz3632, INT64_C(1));
    sail_int zgsz3633;
    CREATE(sail_int)(&zgsz3633);
    CONVERT_OF(sail_int, mach_int)(&zgsz3633, zCAP_VALUE_NUM_BITS);
    shl_int(&zgaz345, zgsz3632, zgsz3633);
    KILL(sail_int)(&zgsz3633);
    KILL(sail_int)(&zgsz3632);
  }
  {
    sail_int zgsz3630;
    CREATE(sail_int)(&zgsz3630);
    CONVERT_OF(sail_int, mach_int)(&zgsz3630, INT64_C(0));
    sail_int zgsz3631;
    CREATE(sail_int)(&zgsz3631);
    CONVERT_OF(sail_int, mach_int)(&zgsz3631, zCAP_BOUND_NUM_BITS);
    sailgen_Slice_int(&zgsz320, zgaz345, zgsz3630, zgsz3631);
    KILL(sail_int)(&zgsz3631);
    KILL(sail_int)(&zgsz3630);
  }
  KILL(sail_int)(&zgaz345);
  COPY(lbits)(&zCAP_BOUND_MAX, zgsz320);
  KILL(lbits)(&zgsz320);
let_end_74: ;
}
static void kill_letbind_16(void) {    KILL(lbits)(&zCAP_BOUND_MAX);
}

static lbits zCAP_BOUND_MIN;
static void create_letbind_17(void) {    CREATE(lbits)(&zCAP_BOUND_MIN);


  lbits zgsz321;
  CREATE(lbits)(&zgsz321);
  int64_t zgaz346;
  {
    lbits zgsz3637;
    CREATE(lbits)(&zgsz3637);
    CONVERT_OF(lbits, fbits)(&zgsz3637, UINT64_C(0x0), UINT64_C(4) , true);
    sail_int zgsz3638;
    CREATE(sail_int)(&zgsz3638);
    sail_unsigned(&zgsz3638, zgsz3637);
    zgaz346 = CONVERT_OF(mach_int, sail_int)(zgsz3638);
    KILL(sail_int)(&zgsz3638);
    KILL(lbits)(&zgsz3637);
  }
  {
    sail_int zgsz3634;
    CREATE(sail_int)(&zgsz3634);
    CONVERT_OF(sail_int, mach_int)(&zgsz3634, zgaz346);
    sail_int zgsz3635;
    CREATE(sail_int)(&zgsz3635);
    CONVERT_OF(sail_int, mach_int)(&zgsz3635, INT64_C(0));
    sail_int zgsz3636;
    CREATE(sail_int)(&zgsz3636);
    CONVERT_OF(sail_int, mach_int)(&zgsz3636, zCAP_BOUND_NUM_BITS);
    sailgen_Slice_int(&zgsz321, zgsz3634, zgsz3635, zgsz3636);
    KILL(sail_int)(&zgsz3636);
    KILL(sail_int)(&zgsz3635);
    KILL(sail_int)(&zgsz3634);
  }

  COPY(lbits)(&zCAP_BOUND_MIN, zgsz321);
  KILL(lbits)(&zgsz321);
let_end_75: ;
}
static void kill_letbind_17(void) {    KILL(lbits)(&zCAP_BOUND_MIN);
}

static int64_t zCAP_MAX_ENCODEABLE_EXPONENT;
static void create_letbind_18(void) {


  int64_t zgsz322;
  zgsz322 = INT64_C(63);
  zCAP_MAX_ENCODEABLE_EXPONENT = zgsz322;

let_end_76: ;
}
static void kill_letbind_18(void) {
}

static int64_t zCAP_MAX_EXPONENT;
static void create_letbind_19(void) {


  int64_t zgsz323;
  int64_t zgaz347;
  {
    sail_int zgsz3639;
    CREATE(sail_int)(&zgsz3639);
    CONVERT_OF(sail_int, mach_int)(&zgsz3639, zCAP_VALUE_NUM_BITS);
    sail_int zgsz3640;
    CREATE(sail_int)(&zgsz3640);
    CONVERT_OF(sail_int, mach_int)(&zgsz3640, zCAP_MW);
    sail_int zgsz3641;
    CREATE(sail_int)(&zgsz3641);
    sub_int(&zgsz3641, zgsz3639, zgsz3640);
    zgaz347 = CONVERT_OF(mach_int, sail_int)(zgsz3641);
    KILL(sail_int)(&zgsz3641);
    KILL(sail_int)(&zgsz3640);
    KILL(sail_int)(&zgsz3639);
  }
  zgsz323 = (zgaz347 + INT64_C(2));

  zCAP_MAX_EXPONENT = zgsz323;

let_end_77: ;
}
static void kill_letbind_19(void) {
}

static uint64_t sailgen_CapBoundsAddress(uint64_t);

lbits zghz373;
sail_int zghz374;
lbits zghz375;
sail_int zghz376;
sail_int zghz377;
sail_int zghz378;

static void startup_sailgen_CapBoundsAddress(void)
{
  CREATE(lbits)(&zghz373);
  CREATE(sail_int)(&zghz374);
  CREATE(lbits)(&zghz375);
  CREATE(sail_int)(&zghz376);
  CREATE(sail_int)(&zghz377);
  CREATE(sail_int)(&zghz378);
}

static uint64_t sailgen_CapBoundsAddress(uint64_t zaddress)
{
  __label__ cleanup_80, end_cleanup_81, end_function_79, end_block_exception_82;

  uint64_t zcbz316;
  {
    uint64_t zgaz349;
    {
      int64_t zgaz348;
      {
        RECREATE(sail_int)(&zghz376);
        CONVERT_OF(sail_int, mach_int)(&zghz376, zCAP_FLAGS_LO_BIT);
        RECREATE(sail_int)(&zghz377);
        CONVERT_OF(sail_int, mach_int)(&zghz377, INT64_C(1));
        RECREATE(sail_int)(&zghz378);
        sub_int(&zghz378, zghz376, zghz377);
        zgaz348 = CONVERT_OF(mach_int, sail_int)(zghz378);
      }
      zgaz349 = (safe_rshift(UINT64_MAX, 64 - 56) & (zaddress >> INT64_C(0)));
    }
    {
      RECREATE(lbits)(&zghz373);
      CONVERT_OF(lbits, fbits)(&zghz373, zgaz349, UINT64_C(56) , true);
      RECREATE(sail_int)(&zghz374);
      CONVERT_OF(sail_int, mach_int)(&zghz374, zCAP_VALUE_NUM_BITS);
      RECREATE(lbits)(&zghz375);
      sign_extend(&zghz375, zghz373, zghz374);
      zcbz316 = CONVERT_OF(fbits, lbits)(zghz375, true);
    }
  }
  goto cleanup_80;
  /* unreachable after return */
  goto end_cleanup_81;
cleanup_80: ;
  goto end_function_79;
end_cleanup_81: ;
end_function_79: ;
  return zcbz316;
end_block_exception_82: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapBoundsAddress(void)
{
  KILL(sail_int)(&zghz378);
  KILL(sail_int)(&zghz377);
  KILL(sail_int)(&zghz376);
  KILL(lbits)(&zghz375);
  KILL(sail_int)(&zghz374);
  KILL(lbits)(&zghz373);
}

static int64_t zCAP_BASE_MANTISSA_LO_BIT;
static void create_letbind_20(void) {


  int64_t zgsz324;
  zgsz324 = INT64_C(67);
  zCAP_BASE_MANTISSA_LO_BIT = zgsz324;

let_end_83: ;
}
static void kill_letbind_20(void) {
}

static uint64_t sailgen_CapGetBottom(lbits);

sail_int zghz379;
sail_int zghz380;
lbits zghz381;
sail_int zghz382;
sail_int zghz383;
lbits zghz384;

static void startup_sailgen_CapGetBottom(void)
{
  CREATE(sail_int)(&zghz379);
  CREATE(sail_int)(&zghz380);
  CREATE(lbits)(&zghz381);
  CREATE(sail_int)(&zghz382);
  CREATE(sail_int)(&zghz383);
  CREATE(lbits)(&zghz384);
}

static uint64_t sailgen_CapGetBottom(lbits zc)
{
  __label__ end_function_85, end_block_exception_90;

  uint64_t zcbz317;
  bool zgaz351;
  zgaz351 = sailgen_CapIsInternalExponent(zc);
  if (zgaz351) {
    __label__ cleanup_88, end_cleanup_89;

    uint64_t zgaz353;
    {
      uint64_t zgaz352;
      {
        RECREATE(sail_int)(&zghz382);
        CONVERT_OF(sail_int, mach_int)(&zghz382, zCAP_BASE_HI_BIT);
        RECREATE(sail_int)(&zghz383);
        CONVERT_OF(sail_int, mach_int)(&zghz383, zCAP_BASE_MANTISSA_LO_BIT);
        RECREATE(lbits)(&zghz384);
        vector_subrange_lbits(&zghz384, zc, zghz382, zghz383);
        zgaz352 = CONVERT_OF(fbits, lbits)(zghz384, true);
      }
      zgaz353 = (zgaz352 << 3) | UINT64_C(0b000);
    }
    zcbz317 = zgaz353;
    goto cleanup_88;
    /* unreachable after return */
    goto end_cleanup_89;
  cleanup_88: ;
    goto end_function_85;
  end_cleanup_89: ;
  } else {
    __label__ cleanup_86, end_cleanup_87;

    uint64_t zgaz354;
    {
      RECREATE(sail_int)(&zghz379);
      CONVERT_OF(sail_int, mach_int)(&zghz379, zCAP_BASE_HI_BIT);
      RECREATE(sail_int)(&zghz380);
      CONVERT_OF(sail_int, mach_int)(&zghz380, zCAP_BASE_LO_BIT);
      RECREATE(lbits)(&zghz381);
      vector_subrange_lbits(&zghz381, zc, zghz379, zghz380);
      zgaz354 = CONVERT_OF(fbits, lbits)(zghz381, true);
    }
    zcbz317 = zgaz354;
    goto cleanup_86;
    /* unreachable after return */
    goto end_cleanup_87;
  cleanup_86: ;
    goto end_function_85;
  end_cleanup_87: ;
  }

end_function_85: ;
  return zcbz317;
end_block_exception_90: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetBottom(void)
{
  KILL(lbits)(&zghz384);
  KILL(sail_int)(&zghz383);
  KILL(sail_int)(&zghz382);
  KILL(lbits)(&zghz381);
  KILL(sail_int)(&zghz380);
  KILL(sail_int)(&zghz379);
}

static int64_t zCAP_LIMIT_HI_BIT;
static void create_letbind_21(void) {


  int64_t zgsz325;
  zgsz325 = INT64_C(93);
  zCAP_LIMIT_HI_BIT = zgsz325;

let_end_91: ;
}
static void kill_letbind_21(void) {
}

static int64_t zCAP_LIMIT_MANTISSA_LO_BIT;
static void create_letbind_22(void) {


  int64_t zgsz326;
  zgsz326 = INT64_C(83);
  zCAP_LIMIT_MANTISSA_LO_BIT = zgsz326;

let_end_92: ;
}
static void kill_letbind_22(void) {
}

static bool sailgen_CapUnsignedLessThan(lbits, lbits);

sail_int zghz385;
sail_int zghz386;

static void startup_sailgen_CapUnsignedLessThan(void)
{
  CREATE(sail_int)(&zghz385);
  CREATE(sail_int)(&zghz386);
}

static bool sailgen_CapUnsignedLessThan(lbits za, lbits zb)
{
  __label__ cleanup_95, end_cleanup_96, end_function_94, end_block_exception_97;

  bool zcbz318;
  {
    RECREATE(sail_int)(&zghz385);
    sail_unsigned(&zghz385, za);
    RECREATE(sail_int)(&zghz386);
    sail_unsigned(&zghz386, zb);
    zcbz318 = lt(zghz385, zghz386);
  }
  goto cleanup_95;
  /* unreachable after return */
  goto end_cleanup_96;
cleanup_95: ;
  goto end_function_94;
end_cleanup_96: ;
end_function_94: ;
  return zcbz318;
end_block_exception_97: ;

  return false;
}



static void finish_sailgen_CapUnsignedLessThan(void)
{
  KILL(sail_int)(&zghz386);
  KILL(sail_int)(&zghz385);
}

static uint64_t sailgen_CapGetTop(lbits);

lbits zghz387;
sail_int zghz388;
sail_int zghz389;
lbits zghz390;
lbits zghz391;
sail_int zghz392;
sail_int zghz393;
sail_int zghz394;
sail_int zghz395;
sail_int zghz396;
sail_int zghz397;
sail_int zghz398;
sail_int zghz399;
sail_int zghz3100;
sail_int zghz3101;
sail_int zghz3102;
sail_int zghz3103;
lbits zghz3104;
lbits zghz3105;
sail_int zghz3106;
sail_int zghz3107;
sail_int zghz3108;
sail_int zghz3109;
sail_int zghz3110;
sail_int zghz3111;
sail_int zghz3112;
sail_int zghz3113;
lbits zghz3114;
sail_int zghz3115;
sail_int zghz3116;
lbits zghz3117;
sail_int zghz3118;
lbits zghz3119;
sail_int zghz3120;
sail_int zghz3121;
sail_int zghz3122;

static void startup_sailgen_CapGetTop(void)
{
  CREATE(lbits)(&zghz387);
  CREATE(sail_int)(&zghz388);
  CREATE(sail_int)(&zghz389);
  CREATE(lbits)(&zghz390);
  CREATE(lbits)(&zghz391);
  CREATE(sail_int)(&zghz392);
  CREATE(sail_int)(&zghz393);
  CREATE(sail_int)(&zghz394);
  CREATE(sail_int)(&zghz395);
  CREATE(sail_int)(&zghz396);
  CREATE(sail_int)(&zghz397);
  CREATE(sail_int)(&zghz398);
  CREATE(sail_int)(&zghz399);
  CREATE(sail_int)(&zghz3100);
  CREATE(sail_int)(&zghz3101);
  CREATE(sail_int)(&zghz3102);
  CREATE(sail_int)(&zghz3103);
  CREATE(lbits)(&zghz3104);
  CREATE(lbits)(&zghz3105);
  CREATE(sail_int)(&zghz3106);
  CREATE(sail_int)(&zghz3107);
  CREATE(sail_int)(&zghz3108);
  CREATE(sail_int)(&zghz3109);
  CREATE(sail_int)(&zghz3110);
  CREATE(sail_int)(&zghz3111);
  CREATE(sail_int)(&zghz3112);
  CREATE(sail_int)(&zghz3113);
  CREATE(lbits)(&zghz3114);
  CREATE(sail_int)(&zghz3115);
  CREATE(sail_int)(&zghz3116);
  CREATE(lbits)(&zghz3117);
  CREATE(sail_int)(&zghz3118);
  CREATE(lbits)(&zghz3119);
  CREATE(sail_int)(&zghz3120);
  CREATE(sail_int)(&zghz3121);
  CREATE(sail_int)(&zghz3122);
}

static uint64_t sailgen_CapGetTop(lbits zc)
{
  __label__ cleanup_100, end_cleanup_101, end_function_99, end_block_exception_102;

  uint64_t zcbz319;
  uint64_t zlmsb;
  zlmsb = UINT64_C(0b00);
  uint64_t zlcarry;
  zlcarry = UINT64_C(0b00);
  uint64_t zb;
  zb = sailgen_CapGetBottom(zc);
  uint64_t zt;
  {
    int64_t zgaz375;
    {
      int64_t zgaz374;
      {
        RECREATE(sail_int)(&zghz3120);
        CONVERT_OF(sail_int, mach_int)(&zghz3120, INT64_C(79));
        RECREATE(sail_int)(&zghz3121);
        CONVERT_OF(sail_int, mach_int)(&zghz3121, INT64_C(64));
        RECREATE(sail_int)(&zghz3122);
        sub_int(&zghz3122, zghz3120, zghz3121);
        zgaz374 = CONVERT_OF(mach_int, sail_int)(zghz3122);
      }
      zgaz375 = (zgaz374 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3118);
      CONVERT_OF(sail_int, mach_int)(&zghz3118, zgaz375);
      RECREATE(lbits)(&zghz3119);
      UNDEFINED(lbits)(&zghz3119, zghz3118);
      zt = CONVERT_OF(fbits, lbits)(zghz3119, true);
    }
  }
  {
    bool zgaz358;
    zgaz358 = sailgen_CapIsInternalExponent(zc);
    unit zgsz329;
    if (zgaz358) {
      {
        zlmsb = UINT64_C(0b01);
        unit zgsz330;
        zgsz330 = UNIT;
      }
      uint64_t zgaz360;
      {
        uint64_t zgaz359;
        {
          RECREATE(sail_int)(&zghz3115);
          CONVERT_OF(sail_int, mach_int)(&zghz3115, zCAP_LIMIT_HI_BIT);
          RECREATE(sail_int)(&zghz3116);
          CONVERT_OF(sail_int, mach_int)(&zghz3116, zCAP_LIMIT_MANTISSA_LO_BIT);
          RECREATE(lbits)(&zghz3117);
          vector_subrange_lbits(&zghz3117, zc, zghz3115, zghz3116);
          zgaz359 = CONVERT_OF(fbits, lbits)(zghz3117, true);
        }
        zgaz360 = (UINT64_C(0b00) << 11) | zgaz359;
      }
      zt = (zgaz360 << 3) | UINT64_C(0b000);
      zgsz329 = UNIT;
    } else {
      uint64_t zgaz361;
      {
        RECREATE(sail_int)(&zghz3112);
        CONVERT_OF(sail_int, mach_int)(&zghz3112, zCAP_LIMIT_HI_BIT);
        RECREATE(sail_int)(&zghz3113);
        CONVERT_OF(sail_int, mach_int)(&zghz3113, zCAP_LIMIT_LO_BIT);
        RECREATE(lbits)(&zghz3114);
        vector_subrange_lbits(&zghz3114, zc, zghz3112, zghz3113);
        zgaz361 = CONVERT_OF(fbits, lbits)(zghz3114, true);
      }
      zt = (UINT64_C(0b00) << 14) | zgaz361;
      zgsz329 = UNIT;
    }
  }
  {
    bool zgaz366;
    {
      uint64_t zgaz364;
      {
        int64_t zgaz362;
        {
          RECREATE(sail_int)(&zghz3109);
          CONVERT_OF(sail_int, mach_int)(&zghz3109, zCAP_MW);
          RECREATE(sail_int)(&zghz3110);
          CONVERT_OF(sail_int, mach_int)(&zghz3110, INT64_C(3));
          RECREATE(sail_int)(&zghz3111);
          sub_int(&zghz3111, zghz3109, zghz3110);
          zgaz362 = CONVERT_OF(mach_int, sail_int)(zghz3111);
        }
        zgaz364 = (safe_rshift(UINT64_MAX, 64 - 14) & (zt >> INT64_C(0)));
      }
      uint64_t zgaz365;
      {
        int64_t zgaz363;
        {
          RECREATE(sail_int)(&zghz3106);
          CONVERT_OF(sail_int, mach_int)(&zghz3106, zCAP_MW);
          RECREATE(sail_int)(&zghz3107);
          CONVERT_OF(sail_int, mach_int)(&zghz3107, INT64_C(3));
          RECREATE(sail_int)(&zghz3108);
          sub_int(&zghz3108, zghz3106, zghz3107);
          zgaz363 = CONVERT_OF(mach_int, sail_int)(zghz3108);
        }
        zgaz365 = (safe_rshift(UINT64_MAX, 64 - 14) & (zb >> INT64_C(0)));
      }
      {
        RECREATE(lbits)(&zghz3104);
        CONVERT_OF(lbits, fbits)(&zghz3104, zgaz364, UINT64_C(14) , true);
        RECREATE(lbits)(&zghz3105);
        CONVERT_OF(lbits, fbits)(&zghz3105, zgaz365, UINT64_C(14) , true);
        zgaz366 = sailgen_CapUnsignedLessThan(zghz3104, zghz3105);
      }
    }
    unit zgsz328;
    if (zgaz366) {
      zlcarry = UINT64_C(0b01);
      zgsz328 = UNIT;
    } else {  zgsz328 = UNIT;  }
  }
  {
    int64_t zgaz371;
    {
      RECREATE(sail_int)(&zghz3101);
      CONVERT_OF(sail_int, mach_int)(&zghz3101, zCAP_MW);
      RECREATE(sail_int)(&zghz3102);
      CONVERT_OF(sail_int, mach_int)(&zghz3102, INT64_C(1));
      RECREATE(sail_int)(&zghz3103);
      sub_int(&zghz3103, zghz3101, zghz3102);
      zgaz371 = CONVERT_OF(mach_int, sail_int)(zghz3103);
    }
    int64_t zgaz372;
    {
      RECREATE(sail_int)(&zghz398);
      CONVERT_OF(sail_int, mach_int)(&zghz398, zCAP_MW);
      RECREATE(sail_int)(&zghz399);
      CONVERT_OF(sail_int, mach_int)(&zghz399, INT64_C(2));
      RECREATE(sail_int)(&zghz3100);
      sub_int(&zghz3100, zghz398, zghz399);
      zgaz372 = CONVERT_OF(mach_int, sail_int)(zghz3100);
    }
    uint64_t zgaz373;
    {
      uint64_t zgaz370;
      {
        uint64_t zgaz369;
        {
          int64_t zgaz367;
          {
            RECREATE(sail_int)(&zghz395);
            CONVERT_OF(sail_int, mach_int)(&zghz395, zCAP_MW);
            RECREATE(sail_int)(&zghz396);
            CONVERT_OF(sail_int, mach_int)(&zghz396, INT64_C(1));
            RECREATE(sail_int)(&zghz397);
            sub_int(&zghz397, zghz395, zghz396);
            zgaz367 = CONVERT_OF(mach_int, sail_int)(zghz397);
          }
          int64_t zgaz368;
          {
            RECREATE(sail_int)(&zghz392);
            CONVERT_OF(sail_int, mach_int)(&zghz392, zCAP_MW);
            RECREATE(sail_int)(&zghz393);
            CONVERT_OF(sail_int, mach_int)(&zghz393, INT64_C(2));
            RECREATE(sail_int)(&zghz394);
            sub_int(&zghz394, zghz392, zghz393);
            zgaz368 = CONVERT_OF(mach_int, sail_int)(zghz394);
          }
          zgaz369 = (safe_rshift(UINT64_MAX, 64 - 2) & (zb >> zgaz368));
        }
        zgaz370 = ((zgaz369 + zlmsb) & UINT64_C(0b11));
      }
      zgaz373 = ((zgaz370 + zlcarry) & UINT64_C(0b11));
    }
    {
      RECREATE(lbits)(&zghz387);
      CONVERT_OF(lbits, fbits)(&zghz387, zt, UINT64_C(16) , true);
      RECREATE(sail_int)(&zghz388);
      CONVERT_OF(sail_int, mach_int)(&zghz388, zgaz371);
      RECREATE(sail_int)(&zghz389);
      CONVERT_OF(sail_int, mach_int)(&zghz389, zgaz372);
      RECREATE(lbits)(&zghz390);
      CONVERT_OF(lbits, fbits)(&zghz390, zgaz373, UINT64_C(2) , true);
      RECREATE(lbits)(&zghz391);
      vector_update_subrange_lbits(&zghz391, zghz387, zghz388, zghz389, zghz390);
      zt = CONVERT_OF(fbits, lbits)(zghz391, true);
    }
    unit zgsz327;
    zgsz327 = UNIT;
  }
  zcbz319 = zt;
  goto cleanup_100;
  /* unreachable after return */




  goto end_cleanup_101;
cleanup_100: ;




  goto end_function_99;
end_cleanup_101: ;
end_function_99: ;
  return zcbz319;
end_block_exception_102: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetTop(void)
{
  KILL(sail_int)(&zghz3122);
  KILL(sail_int)(&zghz3121);
  KILL(sail_int)(&zghz3120);
  KILL(lbits)(&zghz3119);
  KILL(sail_int)(&zghz3118);
  KILL(lbits)(&zghz3117);
  KILL(sail_int)(&zghz3116);
  KILL(sail_int)(&zghz3115);
  KILL(lbits)(&zghz3114);
  KILL(sail_int)(&zghz3113);
  KILL(sail_int)(&zghz3112);
  KILL(sail_int)(&zghz3111);
  KILL(sail_int)(&zghz3110);
  KILL(sail_int)(&zghz3109);
  KILL(sail_int)(&zghz3108);
  KILL(sail_int)(&zghz3107);
  KILL(sail_int)(&zghz3106);
  KILL(lbits)(&zghz3105);
  KILL(lbits)(&zghz3104);
  KILL(sail_int)(&zghz3103);
  KILL(sail_int)(&zghz3102);
  KILL(sail_int)(&zghz3101);
  KILL(sail_int)(&zghz3100);
  KILL(sail_int)(&zghz399);
  KILL(sail_int)(&zghz398);
  KILL(sail_int)(&zghz397);
  KILL(sail_int)(&zghz396);
  KILL(sail_int)(&zghz395);
  KILL(sail_int)(&zghz394);
  KILL(sail_int)(&zghz393);
  KILL(sail_int)(&zghz392);
  KILL(lbits)(&zghz391);
  KILL(lbits)(&zghz390);
  KILL(sail_int)(&zghz389);
  KILL(sail_int)(&zghz388);
  KILL(lbits)(&zghz387);
}

static bool sailgen_CapIsExponentOutOfRange(lbits);

static bool sailgen_CapIsExponentOutOfRange(lbits zc)
{
  __label__ case_105, finish_match_104, end_function_106, end_block_exception_109;

  bool zcbz320;
  int64_t zgaz376;
  zgaz376 = sailgen_CapGetExponent(zc);
  {
    __label__ cleanup_107, end_cleanup_108;

    int64_t zexp;
    zexp = zgaz376;
    {
      bool zgaz377;
      zgaz377 = (zexp > zCAP_MAX_EXPONENT);
      bool zgsz332;
      if (zgaz377) {  zgsz332 = (zexp < zCAP_MAX_ENCODEABLE_EXPONENT);  } else {  zgsz332 = false;  }
      zcbz320 = zgsz332;
    }
    goto cleanup_107;
    /* unreachable after return */
    goto finish_match_104;
    goto end_cleanup_108;
  cleanup_107: ;
    goto end_function_106;
  end_cleanup_108: ;
  }
case_105: ;
  sail_match_failure("CapIsExponentOutOfRange");
finish_match_104: ;

end_function_106: ;
  return zcbz320;
end_block_exception_109: ;

  return false;
}

static bool sailgen_CapUnsignedGreaterThan(lbits, lbits);

sail_int zghz3123;
sail_int zghz3124;

static void startup_sailgen_CapUnsignedGreaterThan(void)
{
  CREATE(sail_int)(&zghz3123);
  CREATE(sail_int)(&zghz3124);
}

static bool sailgen_CapUnsignedGreaterThan(lbits za, lbits zb)
{
  __label__ cleanup_112, end_cleanup_113, end_function_111, end_block_exception_114;

  bool zcbz321;
  {
    RECREATE(sail_int)(&zghz3123);
    sail_unsigned(&zghz3123, za);
    RECREATE(sail_int)(&zghz3124);
    sail_unsigned(&zghz3124, zb);
    zcbz321 = gt(zghz3123, zghz3124);
  }
  goto cleanup_112;
  /* unreachable after return */
  goto end_cleanup_113;
cleanup_112: ;
  goto end_function_111;
end_cleanup_113: ;
end_function_111: ;
  return zcbz321;
end_block_exception_114: ;

  return false;
}



static void finish_sailgen_CapUnsignedGreaterThan(void)
{
  KILL(sail_int)(&zghz3124);
  KILL(sail_int)(&zghz3123);
}

// struct tuple_(%bv, %bv, %bool)
struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 {
  lbits ztup0;
  lbits ztup1;
  bool ztup2;
};

static void COPY(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *rop, const struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 op) {
  COPY(lbits)(&rop->ztup0, op.ztup0);
  COPY(lbits)(&rop->ztup1, op.ztup1);
  rop->ztup2 = op.ztup2;
}

static void CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *op) {
  CREATE(lbits)(&op->ztup0);
  CREATE(lbits)(&op->ztup1);
}

static void RECREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *op) {
  RECREATE(lbits)(&op->ztup0);
  RECREATE(lbits)(&op->ztup1);
}

static void KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *op) {
  KILL(lbits)(&op->ztup0);
  KILL(lbits)(&op->ztup1);
}

static bool EQUAL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 op1, struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 op2) {
  return EQUAL(lbits)(op1.ztup0, op2.ztup0) && EQUAL(lbits)(op1.ztup1, op2.ztup1) && EQUAL(bool)(op1.ztup2, op2.ztup2);
}

static void sailgen_CapGetBounds(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *rop, lbits);

lbits zghz3125;
lbits zghz3126;
lbits zghz3127;
lbits zghz3128;
lbits zghz3129;
sail_int zghz3130;
sail_int zghz3131;
lbits zghz3132;
lbits zghz3133;
sail_int zghz3134;
sail_int zghz3135;
lbits zghz3136;
sail_int zghz3137;
sail_int zghz3138;
lbits zghz3139;
lbits zghz3140;
sail_int zghz3141;
sail_int zghz3142;
sail_int zghz3143;
sail_int zghz3144;
sail_int zghz3145;
sail_int zghz3146;
lbits zghz3147;
sail_int zghz3148;
sail_int zghz3149;
lbits zghz3150;
lbits zghz3151;
sail_int zghz3152;
lbits zghz3153;
sail_int zghz3154;
sail_int zghz3155;
lbits zghz3156;
lbits zghz3157;
sail_int zghz3158;
lbits zghz3159;
sail_int zghz3160;
sail_int zghz3161;
lbits zghz3162;
sail_int zghz3163;
sail_int zghz3164;
sail_int zghz3165;
sail_int zghz3166;
sail_int zghz3167;
sail_int zghz3168;
lbits zghz3169;
lbits zghz3170;
lbits zghz3171;
lbits zghz3172;
lbits zghz3173;
lbits zghz3174;
sail_int zghz3175;
sail_int zghz3176;
sail_int zghz3177;
sail_int zghz3178;
sail_int zghz3179;
sail_int zghz3180;
sail_int zghz3181;
sail_int zghz3182;
sail_int zghz3183;
sail_int zghz3184;
sail_int zghz3185;
sail_int zghz3186;
sail_int zghz3187;
sail_int zghz3188;
lbits zghz3189;
sail_int zghz3190;
sail_int zghz3191;
sail_int zghz3192;
sail_int zghz3193;
sail_int zghz3194;
sail_int zghz3195;
lbits zghz3196;
lbits zghz3197;
sail_int zghz3198;
sail_int zghz3199;
lbits zghz3200;
sail_int zghz3201;
sail_int zghz3202;
sail_int zghz3203;
sail_int zghz3204;
sail_int zghz3205;
lbits zghz3206;
sail_int zghz3207;
sail_int zghz3208;
sail_int zghz3209;
sail_int zghz3210;
sail_int zghz3211;
sail_int zghz3212;
sail_int zghz3213;
sail_int zghz3214;
sail_int zghz3215;
sail_int zghz3216;
sail_int zghz3217;
lbits zghz3218;
sail_int zghz3219;
lbits zghz3220;
sail_int zghz3221;
sail_int zghz3222;
sail_int zghz3223;
lbits zghz3224;
sail_int zghz3225;
lbits zghz3226;
sail_int zghz3227;
sail_int zghz3228;

static void startup_sailgen_CapGetBounds(void)
{
  CREATE(lbits)(&zghz3125);
  CREATE(lbits)(&zghz3126);
  CREATE(lbits)(&zghz3127);
  CREATE(lbits)(&zghz3128);
  CREATE(lbits)(&zghz3129);
  CREATE(sail_int)(&zghz3130);
  CREATE(sail_int)(&zghz3131);
  CREATE(lbits)(&zghz3132);
  CREATE(lbits)(&zghz3133);
  CREATE(sail_int)(&zghz3134);
  CREATE(sail_int)(&zghz3135);
  CREATE(lbits)(&zghz3136);
  CREATE(sail_int)(&zghz3137);
  CREATE(sail_int)(&zghz3138);
  CREATE(lbits)(&zghz3139);
  CREATE(lbits)(&zghz3140);
  CREATE(sail_int)(&zghz3141);
  CREATE(sail_int)(&zghz3142);
  CREATE(sail_int)(&zghz3143);
  CREATE(sail_int)(&zghz3144);
  CREATE(sail_int)(&zghz3145);
  CREATE(sail_int)(&zghz3146);
  CREATE(lbits)(&zghz3147);
  CREATE(sail_int)(&zghz3148);
  CREATE(sail_int)(&zghz3149);
  CREATE(lbits)(&zghz3150);
  CREATE(lbits)(&zghz3151);
  CREATE(sail_int)(&zghz3152);
  CREATE(lbits)(&zghz3153);
  CREATE(sail_int)(&zghz3154);
  CREATE(sail_int)(&zghz3155);
  CREATE(lbits)(&zghz3156);
  CREATE(lbits)(&zghz3157);
  CREATE(sail_int)(&zghz3158);
  CREATE(lbits)(&zghz3159);
  CREATE(sail_int)(&zghz3160);
  CREATE(sail_int)(&zghz3161);
  CREATE(lbits)(&zghz3162);
  CREATE(sail_int)(&zghz3163);
  CREATE(sail_int)(&zghz3164);
  CREATE(sail_int)(&zghz3165);
  CREATE(sail_int)(&zghz3166);
  CREATE(sail_int)(&zghz3167);
  CREATE(sail_int)(&zghz3168);
  CREATE(lbits)(&zghz3169);
  CREATE(lbits)(&zghz3170);
  CREATE(lbits)(&zghz3171);
  CREATE(lbits)(&zghz3172);
  CREATE(lbits)(&zghz3173);
  CREATE(lbits)(&zghz3174);
  CREATE(sail_int)(&zghz3175);
  CREATE(sail_int)(&zghz3176);
  CREATE(sail_int)(&zghz3177);
  CREATE(sail_int)(&zghz3178);
  CREATE(sail_int)(&zghz3179);
  CREATE(sail_int)(&zghz3180);
  CREATE(sail_int)(&zghz3181);
  CREATE(sail_int)(&zghz3182);
  CREATE(sail_int)(&zghz3183);
  CREATE(sail_int)(&zghz3184);
  CREATE(sail_int)(&zghz3185);
  CREATE(sail_int)(&zghz3186);
  CREATE(sail_int)(&zghz3187);
  CREATE(sail_int)(&zghz3188);
  CREATE(lbits)(&zghz3189);
  CREATE(sail_int)(&zghz3190);
  CREATE(sail_int)(&zghz3191);
  CREATE(sail_int)(&zghz3192);
  CREATE(sail_int)(&zghz3193);
  CREATE(sail_int)(&zghz3194);
  CREATE(sail_int)(&zghz3195);
  CREATE(lbits)(&zghz3196);
  CREATE(lbits)(&zghz3197);
  CREATE(sail_int)(&zghz3198);
  CREATE(sail_int)(&zghz3199);
  CREATE(lbits)(&zghz3200);
  CREATE(sail_int)(&zghz3201);
  CREATE(sail_int)(&zghz3202);
  CREATE(sail_int)(&zghz3203);
  CREATE(sail_int)(&zghz3204);
  CREATE(sail_int)(&zghz3205);
  CREATE(lbits)(&zghz3206);
  CREATE(sail_int)(&zghz3207);
  CREATE(sail_int)(&zghz3208);
  CREATE(sail_int)(&zghz3209);
  CREATE(sail_int)(&zghz3210);
  CREATE(sail_int)(&zghz3211);
  CREATE(sail_int)(&zghz3212);
  CREATE(sail_int)(&zghz3213);
  CREATE(sail_int)(&zghz3214);
  CREATE(sail_int)(&zghz3215);
  CREATE(sail_int)(&zghz3216);
  CREATE(sail_int)(&zghz3217);
  CREATE(lbits)(&zghz3218);
  CREATE(sail_int)(&zghz3219);
  CREATE(lbits)(&zghz3220);
  CREATE(sail_int)(&zghz3221);
  CREATE(sail_int)(&zghz3222);
  CREATE(sail_int)(&zghz3223);
  CREATE(lbits)(&zghz3224);
  CREATE(sail_int)(&zghz3225);
  CREATE(lbits)(&zghz3226);
  CREATE(sail_int)(&zghz3227);
  CREATE(sail_int)(&zghz3228);
}

static void sailgen_CapGetBounds(struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 *zcbz322, lbits zc)
{
  __label__ case_117, finish_match_116, end_function_128, end_block_exception_135, end_function_288;

  int64_t zgaz382;
  zgaz382 = sailgen_CapGetExponent(zc);
  struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgsz334;
  CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz334);
  {
    __label__ case_119, finish_match_118;

    int64_t zexp;
    zexp = zgaz382;
    {
      bool zgaz383;
      zgaz383 = (zexp == zCAP_MAX_ENCODEABLE_EXPONENT);
      unit zgsz337;
      if (zgaz383) {
        __label__ cleanup_133, end_cleanup_134;

        struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgsz338;
        CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz338);
        COPY(lbits)(&((&zgsz338)->ztup0), zCAP_BOUND_MIN);
        COPY(lbits)(&((&zgsz338)->ztup1), zCAP_BOUND_MAX);
        zgsz338.ztup2 = true;
        COPY(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)((*(&zcbz322)), zgsz338);
        goto cleanup_133;
        /* unreachable after return */
        goto end_cleanup_134;
      cleanup_133: ;
        KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz334);
        KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz338);
        goto end_function_128;
      end_cleanup_134: ;
      } else {  zgsz337 = UNIT;  }
    }
    {
      bool zgaz384;
      zgaz384 = sailgen_CapIsExponentOutOfRange(zc);
      unit zgsz335;
      if (zgaz384) {
        __label__ cleanup_131, end_cleanup_132;

        struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgsz336;
        CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz336);
        COPY(lbits)(&((&zgsz336)->ztup0), zCAP_BOUND_MIN);
        COPY(lbits)(&((&zgsz336)->ztup1), zCAP_BOUND_MAX);
        zgsz336.ztup2 = false;
        COPY(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)((*(&zcbz322)), zgsz336);
        goto cleanup_131;
        /* unreachable after return */
        goto end_cleanup_132;
      cleanup_131: ;
        KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz334);
        KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz336);
        goto end_function_128;
      end_cleanup_132: ;
      } else {  zgsz335 = UNIT;  }
    }
    RECREATE(lbits)(&zghz3125);
    {
      RECREATE(sail_int)(&zghz3228);
      CONVERT_OF(sail_int, mach_int)(&zghz3228, INT64_C(66));
      UNDEFINED(lbits)(&zghz3125, zghz3228);
    }
    RECREATE(lbits)(&zghz3126);
    {
      RECREATE(sail_int)(&zghz3227);
      CONVERT_OF(sail_int, mach_int)(&zghz3227, INT64_C(66));
      UNDEFINED(lbits)(&zghz3126, zghz3227);
    }
    uint64_t zbottom;
    zbottom = sailgen_CapGetBottom(zc);
    uint64_t ztop;
    ztop = sailgen_CapGetTop(zc);
    {
      sbits zgaz385;
      {
        RECREATE(sail_int)(&zghz3225);
        CONVERT_OF(sail_int, mach_int)(&zghz3225, zexp);
        RECREATE(lbits)(&zghz3226);
        sailgen_Zeros(&zghz3226, zghz3225);
        zgaz385 = CONVERT_OF(sbits, lbits)(zghz3226, true);
      }
      {
        RECREATE(sail_int)(&zghz3221);
        CONVERT_OF(sail_int, mach_int)(&zghz3221, INT64_C(66));
        RECREATE(sail_int)(&zghz3222);
        CONVERT_OF(sail_int, mach_int)(&zghz3222, zexp);
        RECREATE(sail_int)(&zghz3223);
        CONVERT_OF(sail_int, mach_int)(&zghz3223, INT64_C(0));
        RECREATE(lbits)(&zghz3224);
        CONVERT_OF(lbits, sbits)(&zghz3224, zgaz385, true);
        set_slice(&zghz3125, zghz3221, zghz3222, zghz3125, zghz3223, zghz3224);
      }
      unit zgsz343;
      zgsz343 = UNIT;
    }
    {
      sbits zgaz386;
      {
        RECREATE(sail_int)(&zghz3219);
        CONVERT_OF(sail_int, mach_int)(&zghz3219, zexp);
        RECREATE(lbits)(&zghz3220);
        sailgen_Zeros(&zghz3220, zghz3219);
        zgaz386 = CONVERT_OF(sbits, lbits)(zghz3220, true);
      }
      {
        RECREATE(sail_int)(&zghz3215);
        CONVERT_OF(sail_int, mach_int)(&zghz3215, INT64_C(66));
        RECREATE(sail_int)(&zghz3216);
        CONVERT_OF(sail_int, mach_int)(&zghz3216, zexp);
        RECREATE(sail_int)(&zghz3217);
        CONVERT_OF(sail_int, mach_int)(&zghz3217, INT64_C(0));
        RECREATE(lbits)(&zghz3218);
        CONVERT_OF(lbits, sbits)(&zghz3218, zgaz386, true);
        set_slice(&zghz3126, zghz3215, zghz3216, zghz3126, zghz3217, zghz3218);
      }
      unit zgsz342;
      zgsz342 = UNIT;
    }
    {
      bool zgaz390;
      {
        int64_t zgaz389;
        {
          int64_t zgaz388;
          {
            int64_t zgaz387;
            {
              RECREATE(sail_int)(&zghz3213);
              CONVERT_OF(sail_int, mach_int)(&zghz3213, zexp);
              RECREATE(sail_int)(&zghz3214);
              sailgen___id(&zghz3214, zghz3213);
              zgaz387 = CONVERT_OF(mach_int, sail_int)(zghz3214);
            }
            zgaz388 = (zgaz387 + INT64_C(16));
          }
          {
            RECREATE(sail_int)(&zghz3210);
            CONVERT_OF(sail_int, mach_int)(&zghz3210, zgaz388);
            RECREATE(sail_int)(&zghz3211);
            CONVERT_OF(sail_int, mach_int)(&zghz3211, INT64_C(1));
            RECREATE(sail_int)(&zghz3212);
            sub_int(&zghz3212, zghz3210, zghz3211);
            zgaz389 = CONVERT_OF(mach_int, sail_int)(zghz3212);
          }
        }
        zgaz390 = (zgaz389 < INT64_C(66));
      }
      unit zgsz341;
      zgsz341 = sail_assert(zgaz390, "v8_base.sail:1391.41-1391.42");
    }
    {
      int64_t zgaz392;
      {
        int64_t zgaz391;
        zgaz391 = (zexp + zCAP_MW);
        {
          RECREATE(sail_int)(&zghz3207);
          CONVERT_OF(sail_int, mach_int)(&zghz3207, zgaz391);
          RECREATE(sail_int)(&zghz3208);
          CONVERT_OF(sail_int, mach_int)(&zghz3208, INT64_C(1));
          RECREATE(sail_int)(&zghz3209);
          sub_int(&zghz3209, zghz3207, zghz3208);
          zgaz392 = CONVERT_OF(mach_int, sail_int)(zghz3209);
        }
      }
      {
        RECREATE(sail_int)(&zghz3204);
        CONVERT_OF(sail_int, mach_int)(&zghz3204, zgaz392);
        RECREATE(sail_int)(&zghz3205);
        CONVERT_OF(sail_int, mach_int)(&zghz3205, zexp);
        RECREATE(lbits)(&zghz3206);
        CONVERT_OF(lbits, fbits)(&zghz3206, zbottom, UINT64_C(16) , true);
        vector_update_subrange_lbits(&zghz3125, zghz3125, zghz3204, zghz3205, zghz3206);
      }
      unit zgsz340;
      zgsz340 = UNIT;
    }
    {
      int64_t zgaz394;
      {
        int64_t zgaz393;
        zgaz393 = (zexp + zCAP_MW);
        {
          RECREATE(sail_int)(&zghz3201);
          CONVERT_OF(sail_int, mach_int)(&zghz3201, zgaz393);
          RECREATE(sail_int)(&zghz3202);
          CONVERT_OF(sail_int, mach_int)(&zghz3202, INT64_C(1));
          RECREATE(sail_int)(&zghz3203);
          sub_int(&zghz3203, zghz3201, zghz3202);
          zgaz394 = CONVERT_OF(mach_int, sail_int)(zghz3203);
        }
      }
      {
        RECREATE(sail_int)(&zghz3198);
        CONVERT_OF(sail_int, mach_int)(&zghz3198, zgaz394);
        RECREATE(sail_int)(&zghz3199);
        CONVERT_OF(sail_int, mach_int)(&zghz3199, zexp);
        RECREATE(lbits)(&zghz3200);
        CONVERT_OF(lbits, fbits)(&zghz3200, ztop, UINT64_C(16) , true);
        vector_update_subrange_lbits(&zghz3126, zghz3126, zghz3198, zghz3199, zghz3200);
      }
      unit zgsz339;
      zgsz339 = UNIT;
    }
    RECREATE(lbits)(&zghz3127);
    {
      uint64_t zgaz3131;
      {
        uint64_t zgaz3130;
        zgaz3130 = sailgen_CapGetValue(zc);
        zgaz3131 = sailgen_CapBoundsAddress(zgaz3130);
      }
      {
        RECREATE(lbits)(&zghz3196);
        CONVERT_OF(lbits, fbits)(&zghz3196, UINT64_C(0b00), UINT64_C(2) , true);
        RECREATE(lbits)(&zghz3197);
        CONVERT_OF(lbits, fbits)(&zghz3197, zgaz3131, UINT64_C(64) , true);
        append(&zghz3127, zghz3196, zghz3197);
      }
    }
    uint64_t zA3;
    {
      int64_t zgaz3128;
      {
        int64_t zgaz3126;
        zgaz3126 = (zexp + zCAP_MW);
        {
          RECREATE(sail_int)(&zghz3193);
          CONVERT_OF(sail_int, mach_int)(&zghz3193, zgaz3126);
          RECREATE(sail_int)(&zghz3194);
          CONVERT_OF(sail_int, mach_int)(&zghz3194, INT64_C(1));
          RECREATE(sail_int)(&zghz3195);
          sub_int(&zghz3195, zghz3193, zghz3194);
          zgaz3128 = CONVERT_OF(mach_int, sail_int)(zghz3195);
        }
      }
      int64_t zgaz3129;
      {
        int64_t zgaz3127;
        zgaz3127 = (zexp + zCAP_MW);
        {
          RECREATE(sail_int)(&zghz3190);
          CONVERT_OF(sail_int, mach_int)(&zghz3190, zgaz3127);
          RECREATE(sail_int)(&zghz3191);
          CONVERT_OF(sail_int, mach_int)(&zghz3191, INT64_C(3));
          RECREATE(sail_int)(&zghz3192);
          sub_int(&zghz3192, zghz3190, zghz3191);
          zgaz3129 = CONVERT_OF(mach_int, sail_int)(zghz3192);
        }
      }
      {
        RECREATE(sail_int)(&zghz3187);
        CONVERT_OF(sail_int, mach_int)(&zghz3187, zgaz3128);
        RECREATE(sail_int)(&zghz3188);
        CONVERT_OF(sail_int, mach_int)(&zghz3188, zgaz3129);
        RECREATE(lbits)(&zghz3189);
        vector_subrange_lbits(&zghz3189, zghz3127, zghz3187, zghz3188);
        zA3 = CONVERT_OF(fbits, lbits)(zghz3189, true);
      }
    }
    uint64_t zB3;
    {
      int64_t zgaz3124;
      {
        RECREATE(sail_int)(&zghz3184);
        CONVERT_OF(sail_int, mach_int)(&zghz3184, zCAP_MW);
        RECREATE(sail_int)(&zghz3185);
        CONVERT_OF(sail_int, mach_int)(&zghz3185, INT64_C(1));
        RECREATE(sail_int)(&zghz3186);
        sub_int(&zghz3186, zghz3184, zghz3185);
        zgaz3124 = CONVERT_OF(mach_int, sail_int)(zghz3186);
      }
      int64_t zgaz3125;
      {
        RECREATE(sail_int)(&zghz3181);
        CONVERT_OF(sail_int, mach_int)(&zghz3181, zCAP_MW);
        RECREATE(sail_int)(&zghz3182);
        CONVERT_OF(sail_int, mach_int)(&zghz3182, INT64_C(3));
        RECREATE(sail_int)(&zghz3183);
        sub_int(&zghz3183, zghz3181, zghz3182);
        zgaz3125 = CONVERT_OF(mach_int, sail_int)(zghz3183);
      }
      zB3 = (safe_rshift(UINT64_MAX, 64 - 3) & (zbottom >> zgaz3125));
    }
    uint64_t zT3;
    {
      int64_t zgaz3122;
      {
        RECREATE(sail_int)(&zghz3178);
        CONVERT_OF(sail_int, mach_int)(&zghz3178, zCAP_MW);
        RECREATE(sail_int)(&zghz3179);
        CONVERT_OF(sail_int, mach_int)(&zghz3179, INT64_C(1));
        RECREATE(sail_int)(&zghz3180);
        sub_int(&zghz3180, zghz3178, zghz3179);
        zgaz3122 = CONVERT_OF(mach_int, sail_int)(zghz3180);
      }
      int64_t zgaz3123;
      {
        RECREATE(sail_int)(&zghz3175);
        CONVERT_OF(sail_int, mach_int)(&zghz3175, zCAP_MW);
        RECREATE(sail_int)(&zghz3176);
        CONVERT_OF(sail_int, mach_int)(&zghz3176, INT64_C(3));
        RECREATE(sail_int)(&zghz3177);
        sub_int(&zghz3177, zghz3175, zghz3176);
        zgaz3123 = CONVERT_OF(mach_int, sail_int)(zghz3177);
      }
      zT3 = (safe_rshift(UINT64_MAX, 64 - 3) & (ztop >> zgaz3123));
    }
    uint64_t zR3;
    zR3 = ((zB3 - UINT64_C(0b001)) & UINT64_C(0b111));
    int64_t zaHi;
    zaHi = INT64_C(0);
    {
      bool zgaz395;
      {
        RECREATE(lbits)(&zghz3173);
        CONVERT_OF(lbits, fbits)(&zghz3173, zA3, UINT64_C(3) , true);
        RECREATE(lbits)(&zghz3174);
        CONVERT_OF(lbits, fbits)(&zghz3174, zR3, UINT64_C(3) , true);
        zgaz395 = sailgen_CapUnsignedLessThan(zghz3173, zghz3174);
      }
      unit zgsz344;
      if (zgaz395) {
        zaHi = INT64_C(1);
        zgsz344 = UNIT;
      } else {
        zaHi = INT64_C(0);
        zgsz344 = UNIT;
      }
    }
    {
      __label__ case_121, finish_match_120;

      int64_t zaHishadowz30;
      zaHishadowz30 = zaHi;
      int64_t zbHi;
      zbHi = INT64_C(0);
      {
        bool zgaz396;
        {
          RECREATE(lbits)(&zghz3171);
          CONVERT_OF(lbits, fbits)(&zghz3171, zB3, UINT64_C(3) , true);
          RECREATE(lbits)(&zghz3172);
          CONVERT_OF(lbits, fbits)(&zghz3172, zR3, UINT64_C(3) , true);
          zgaz396 = sailgen_CapUnsignedLessThan(zghz3171, zghz3172);
        }
        unit zgsz346;
        if (zgaz396) {
          zbHi = INT64_C(1);
          zgsz346 = UNIT;
        } else {
          zbHi = INT64_C(0);
          zgsz346 = UNIT;
        }
      }
      {
        __label__ case_123, finish_match_122;

        int64_t zbHishadowz31;
        zbHishadowz31 = zbHi;
        int64_t ztHi;
        ztHi = INT64_C(0);
        {
          bool zgaz397;
          {
            RECREATE(lbits)(&zghz3169);
            CONVERT_OF(lbits, fbits)(&zghz3169, zT3, UINT64_C(3) , true);
            RECREATE(lbits)(&zghz3170);
            CONVERT_OF(lbits, fbits)(&zghz3170, zR3, UINT64_C(3) , true);
            zgaz397 = sailgen_CapUnsignedLessThan(zghz3169, zghz3170);
          }
          unit zgsz348;
          if (zgaz397) {
            ztHi = INT64_C(1);
            zgsz348 = UNIT;
          } else {
            ztHi = INT64_C(0);
            zgsz348 = UNIT;
          }
        }
        {
          __label__ case_125, finish_match_124;

          int64_t ztHishadowz32;
          ztHishadowz32 = ztHi;
          int64_t zgaz398;
          {
            RECREATE(sail_int)(&zghz3166);
            CONVERT_OF(sail_int, mach_int)(&zghz3166, zbHishadowz31);
            RECREATE(sail_int)(&zghz3167);
            CONVERT_OF(sail_int, mach_int)(&zghz3167, zaHishadowz30);
            RECREATE(sail_int)(&zghz3168);
            sub_int(&zghz3168, zghz3166, zghz3167);
            zgaz398 = CONVERT_OF(mach_int, sail_int)(zghz3168);
          }
          {
            __label__ case_127, finish_match_126;

            int64_t zcorrection_base;
            zcorrection_base = zgaz398;
            int64_t zgaz399;
            {
              RECREATE(sail_int)(&zghz3163);
              CONVERT_OF(sail_int, mach_int)(&zghz3163, ztHishadowz32);
              RECREATE(sail_int)(&zghz3164);
              CONVERT_OF(sail_int, mach_int)(&zghz3164, zaHishadowz30);
              RECREATE(sail_int)(&zghz3165);
              sub_int(&zghz3165, zghz3163, zghz3164);
              zgaz399 = CONVERT_OF(mach_int, sail_int)(zghz3165);
            }
            {
              __label__ cleanup_129, end_cleanup_130;

              int64_t zcorrection_limit;
              zcorrection_limit = zgaz399;
              {
                bool zgaz3102;
                {
                  int64_t zgaz3100;
                  zgaz3100 = (zexp + zCAP_MW);
                  int64_t zgaz3101;
                  zgaz3101 = (zCAP_MAX_EXPONENT + zCAP_MW);
                  zgaz3102 = (zgaz3100 < zgaz3101);
                }
                unit zgsz352;
                if (zgaz3102) {
                  sbits zatop;
                  {
                    int64_t zgaz3107;
                    zgaz3107 = (zexp + zCAP_MW);
                    {
                      RECREATE(sail_int)(&zghz3160);
                      CONVERT_OF(sail_int, mach_int)(&zghz3160, INT64_C(65));
                      RECREATE(sail_int)(&zghz3161);
                      CONVERT_OF(sail_int, mach_int)(&zghz3161, zgaz3107);
                      RECREATE(lbits)(&zghz3162);
                      vector_subrange_lbits(&zghz3162, zghz3127, zghz3160, zghz3161);
                      zatop = CONVERT_OF(sbits, lbits)(zghz3162, true);
                    }
                  }
                  {
                    int64_t zgaz3103;
                    zgaz3103 = (zexp + zCAP_MW);
                    sbits zgaz3104;
                    {
                      RECREATE(lbits)(&zghz3157);
                      CONVERT_OF(lbits, sbits)(&zghz3157, zatop, true);
                      RECREATE(sail_int)(&zghz3158);
                      CONVERT_OF(sail_int, mach_int)(&zghz3158, zcorrection_base);
                      RECREATE(lbits)(&zghz3159);
                      add_bits_int(&zghz3159, zghz3157, zghz3158);
                      zgaz3104 = CONVERT_OF(sbits, lbits)(zghz3159, true);
                    }
                    {
                      RECREATE(sail_int)(&zghz3154);
                      CONVERT_OF(sail_int, mach_int)(&zghz3154, INT64_C(65));
                      RECREATE(sail_int)(&zghz3155);
                      CONVERT_OF(sail_int, mach_int)(&zghz3155, zgaz3103);
                      RECREATE(lbits)(&zghz3156);
                      CONVERT_OF(lbits, sbits)(&zghz3156, zgaz3104, true);
                      vector_update_subrange_lbits(&zghz3125, zghz3125, zghz3154, zghz3155, zghz3156);
                    }
                    unit zgsz353;
                    zgsz353 = UNIT;
                  }
                  int64_t zgaz3105;
                  zgaz3105 = (zexp + zCAP_MW);
                  sbits zgaz3106;
                  {
                    RECREATE(lbits)(&zghz3151);
                    CONVERT_OF(lbits, sbits)(&zghz3151, zatop, true);
                    RECREATE(sail_int)(&zghz3152);
                    CONVERT_OF(sail_int, mach_int)(&zghz3152, zcorrection_limit);
                    RECREATE(lbits)(&zghz3153);
                    add_bits_int(&zghz3153, zghz3151, zghz3152);
                    zgaz3106 = CONVERT_OF(sbits, lbits)(zghz3153, true);
                  }
                  {
                    RECREATE(sail_int)(&zghz3148);
                    CONVERT_OF(sail_int, mach_int)(&zghz3148, INT64_C(65));
                    RECREATE(sail_int)(&zghz3149);
                    CONVERT_OF(sail_int, mach_int)(&zghz3149, zgaz3105);
                    RECREATE(lbits)(&zghz3150);
                    CONVERT_OF(lbits, sbits)(&zghz3150, zgaz3106, true);
                    vector_update_subrange_lbits(&zghz3126, zghz3126, zghz3148, zghz3149, zghz3150);
                  }
                  zgsz352 = UNIT;
                } else {  zgsz352 = UNIT;  }
              }
              uint64_t zl2;
              {
                RECREATE(sail_int)(&zghz3145);
                CONVERT_OF(sail_int, mach_int)(&zghz3145, INT64_C(64));
                RECREATE(sail_int)(&zghz3146);
                CONVERT_OF(sail_int, mach_int)(&zghz3146, INT64_C(63));
                RECREATE(lbits)(&zghz3147);
                vector_subrange_lbits(&zghz3147, zghz3126, zghz3145, zghz3146);
                zl2 = CONVERT_OF(fbits, lbits)(zghz3147, true);
              }
              uint64_t zb2;
              {
                uint64_t zgaz3121;
                {
                  fbits zgaz3120;
                  {
                    RECREATE(sail_int)(&zghz3144);
                    CONVERT_OF(sail_int, mach_int)(&zghz3144, INT64_C(63));
                    zgaz3120 = bitvector_access(zghz3125, zghz3144);
                  }
                  zgaz3121 = UINT64_C(0b0);
                  zgaz3121 = update_fbits(zgaz3121, INT64_C(0), zgaz3120);
                }
                zb2 = (UINT64_C(0b0) << 1) | zgaz3121;
              }
              {
                bool zgaz3111;
                {
                  bool zgaz3110;
                  {
                    int64_t zgaz3108;
                    {
                      RECREATE(sail_int)(&zghz3141);
                      CONVERT_OF(sail_int, mach_int)(&zghz3141, zCAP_MAX_EXPONENT);
                      RECREATE(sail_int)(&zghz3142);
                      CONVERT_OF(sail_int, mach_int)(&zghz3142, INT64_C(1));
                      RECREATE(sail_int)(&zghz3143);
                      sub_int(&zghz3143, zghz3141, zghz3142);
                      zgaz3108 = CONVERT_OF(mach_int, sail_int)(zghz3143);
                    }
                    zgaz3110 = (zexp < zgaz3108);
                  }
                  bool zgsz355;
                  if (zgaz3110) {
                    uint64_t zgaz3109;
                    zgaz3109 = ((zl2 - zb2) & UINT64_C(0b11));
                    {
                      RECREATE(lbits)(&zghz3139);
                      CONVERT_OF(lbits, fbits)(&zghz3139, zgaz3109, UINT64_C(2) , true);
                      RECREATE(lbits)(&zghz3140);
                      CONVERT_OF(lbits, fbits)(&zghz3140, UINT64_C(0b01), UINT64_C(2) , true);
                      zgsz355 = sailgen_CapUnsignedGreaterThan(zghz3139, zghz3140);
                    }
                  } else {  zgsz355 = false;  }
                  zgaz3111 = zgsz355;
                }
                unit zgsz356;
                if (zgaz3111) {
                  fbits zgaz3115;
                  {
                    uint64_t zgaz3114;
                    {
                      uint64_t zgaz3113;
                      {
                        fbits zgaz3112;
                        {
                          RECREATE(sail_int)(&zghz3138);
                          CONVERT_OF(sail_int, mach_int)(&zghz3138, INT64_C(64));
                          zgaz3112 = bitvector_access(zghz3126, zghz3138);
                        }
                        zgaz3113 = UINT64_C(0b0);
                        zgaz3113 = update_fbits(zgaz3113, INT64_C(0), zgaz3112);
                      }
                      zgaz3114 = (~(zgaz3113) & UINT64_C(0b1));
                    }
                    zgaz3115 = sailgen_Bit(zgaz3114);
                  }
                  {
                    RECREATE(sail_int)(&zghz3137);
                    CONVERT_OF(sail_int, mach_int)(&zghz3137, INT64_C(64));
                    update_lbits(&zghz3126, zghz3126, zghz3137, zgaz3115);
                  }
                  zgsz356 = UNIT;
                } else {  zgsz356 = UNIT;  }
              }
              struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgaz3119;
              CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3119);
              {
                RECREATE(lbits)(&zghz3128);
                {
                  uint64_t zgaz3116;
                  {
                    RECREATE(sail_int)(&zghz3134);
                    CONVERT_OF(sail_int, mach_int)(&zghz3134, INT64_C(63));
                    RECREATE(sail_int)(&zghz3135);
                    CONVERT_OF(sail_int, mach_int)(&zghz3135, INT64_C(0));
                    RECREATE(lbits)(&zghz3136);
                    vector_subrange_lbits(&zghz3136, zghz3125, zghz3134, zghz3135);
                    zgaz3116 = CONVERT_OF(fbits, lbits)(zghz3136, true);
                  }
                  {
                    RECREATE(lbits)(&zghz3132);
                    CONVERT_OF(lbits, fbits)(&zghz3132, UINT64_C(0b0), UINT64_C(1) , true);
                    RECREATE(lbits)(&zghz3133);
                    CONVERT_OF(lbits, fbits)(&zghz3133, zgaz3116, UINT64_C(64) , true);
                    append(&zghz3128, zghz3132, zghz3133);
                  }
                }
                RECREATE(lbits)(&zghz3129);
                {
                  RECREATE(sail_int)(&zghz3130);
                  CONVERT_OF(sail_int, mach_int)(&zghz3130, INT64_C(64));
                  RECREATE(sail_int)(&zghz3131);
                  CONVERT_OF(sail_int, mach_int)(&zghz3131, INT64_C(0));
                  vector_subrange_lbits(&zghz3129, zghz3126, zghz3130, zghz3131);
                }
                COPY(lbits)(&((&zgaz3119)->ztup0), zghz3128);
                COPY(lbits)(&((&zgaz3119)->ztup1), zghz3129);
                zgaz3119.ztup2 = true;
              }
              COPY(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)((*(&zcbz322)), zgaz3119);
              goto cleanup_129;
              /* unreachable after return */
              KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3119);
              goto finish_match_126;
              goto end_cleanup_130;
            cleanup_129: ;
              KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz334);
              KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3119);
              goto end_function_128;
            end_cleanup_130: ;
            }
          case_127: ;
            sail_match_failure("CapGetBounds");
          finish_match_126: ;
            goto finish_match_124;
          }
        case_125: ;
          sail_match_failure("CapGetBounds");
        finish_match_124: ;
          goto finish_match_122;
        }
      case_123: ;
        sail_match_failure("CapGetBounds");
      finish_match_122: ;
        goto finish_match_120;
      }
    case_121: ;
      sail_match_failure("CapGetBounds");
    finish_match_120: ;
      goto finish_match_118;
    }
  case_119: ;
    sail_match_failure("CapGetBounds");
  finish_match_118: ;
    goto finish_match_116;
  }
case_117: ;
  sail_match_failure("CapGetBounds");
finish_match_116: ;
  COPY(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)((*(&zcbz322)), zgsz334);
  KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgsz334);

end_function_128: ;
  goto end_function_288;
end_block_exception_135: ;
  goto end_function_288;
end_function_288: ;
}



static void finish_sailgen_CapGetBounds(void)
{
  KILL(sail_int)(&zghz3228);
  KILL(sail_int)(&zghz3227);
  KILL(lbits)(&zghz3226);
  KILL(sail_int)(&zghz3225);
  KILL(lbits)(&zghz3224);
  KILL(sail_int)(&zghz3223);
  KILL(sail_int)(&zghz3222);
  KILL(sail_int)(&zghz3221);
  KILL(lbits)(&zghz3220);
  KILL(sail_int)(&zghz3219);
  KILL(lbits)(&zghz3218);
  KILL(sail_int)(&zghz3217);
  KILL(sail_int)(&zghz3216);
  KILL(sail_int)(&zghz3215);
  KILL(sail_int)(&zghz3214);
  KILL(sail_int)(&zghz3213);
  KILL(sail_int)(&zghz3212);
  KILL(sail_int)(&zghz3211);
  KILL(sail_int)(&zghz3210);
  KILL(sail_int)(&zghz3209);
  KILL(sail_int)(&zghz3208);
  KILL(sail_int)(&zghz3207);
  KILL(lbits)(&zghz3206);
  KILL(sail_int)(&zghz3205);
  KILL(sail_int)(&zghz3204);
  KILL(sail_int)(&zghz3203);
  KILL(sail_int)(&zghz3202);
  KILL(sail_int)(&zghz3201);
  KILL(lbits)(&zghz3200);
  KILL(sail_int)(&zghz3199);
  KILL(sail_int)(&zghz3198);
  KILL(lbits)(&zghz3197);
  KILL(lbits)(&zghz3196);
  KILL(sail_int)(&zghz3195);
  KILL(sail_int)(&zghz3194);
  KILL(sail_int)(&zghz3193);
  KILL(sail_int)(&zghz3192);
  KILL(sail_int)(&zghz3191);
  KILL(sail_int)(&zghz3190);
  KILL(lbits)(&zghz3189);
  KILL(sail_int)(&zghz3188);
  KILL(sail_int)(&zghz3187);
  KILL(sail_int)(&zghz3186);
  KILL(sail_int)(&zghz3185);
  KILL(sail_int)(&zghz3184);
  KILL(sail_int)(&zghz3183);
  KILL(sail_int)(&zghz3182);
  KILL(sail_int)(&zghz3181);
  KILL(sail_int)(&zghz3180);
  KILL(sail_int)(&zghz3179);
  KILL(sail_int)(&zghz3178);
  KILL(sail_int)(&zghz3177);
  KILL(sail_int)(&zghz3176);
  KILL(sail_int)(&zghz3175);
  KILL(lbits)(&zghz3174);
  KILL(lbits)(&zghz3173);
  KILL(lbits)(&zghz3172);
  KILL(lbits)(&zghz3171);
  KILL(lbits)(&zghz3170);
  KILL(lbits)(&zghz3169);
  KILL(sail_int)(&zghz3168);
  KILL(sail_int)(&zghz3167);
  KILL(sail_int)(&zghz3166);
  KILL(sail_int)(&zghz3165);
  KILL(sail_int)(&zghz3164);
  KILL(sail_int)(&zghz3163);
  KILL(lbits)(&zghz3162);
  KILL(sail_int)(&zghz3161);
  KILL(sail_int)(&zghz3160);
  KILL(lbits)(&zghz3159);
  KILL(sail_int)(&zghz3158);
  KILL(lbits)(&zghz3157);
  KILL(lbits)(&zghz3156);
  KILL(sail_int)(&zghz3155);
  KILL(sail_int)(&zghz3154);
  KILL(lbits)(&zghz3153);
  KILL(sail_int)(&zghz3152);
  KILL(lbits)(&zghz3151);
  KILL(lbits)(&zghz3150);
  KILL(sail_int)(&zghz3149);
  KILL(sail_int)(&zghz3148);
  KILL(lbits)(&zghz3147);
  KILL(sail_int)(&zghz3146);
  KILL(sail_int)(&zghz3145);
  KILL(sail_int)(&zghz3144);
  KILL(sail_int)(&zghz3143);
  KILL(sail_int)(&zghz3142);
  KILL(sail_int)(&zghz3141);
  KILL(lbits)(&zghz3140);
  KILL(lbits)(&zghz3139);
  KILL(sail_int)(&zghz3138);
  KILL(sail_int)(&zghz3137);
  KILL(lbits)(&zghz3136);
  KILL(sail_int)(&zghz3135);
  KILL(sail_int)(&zghz3134);
  KILL(lbits)(&zghz3133);
  KILL(lbits)(&zghz3132);
  KILL(sail_int)(&zghz3131);
  KILL(sail_int)(&zghz3130);
  KILL(lbits)(&zghz3129);
  KILL(lbits)(&zghz3128);
  KILL(lbits)(&zghz3127);
  KILL(lbits)(&zghz3126);
  KILL(lbits)(&zghz3125);
}

static bool sailgen_CapBoundsEqual(lbits, lbits);

lbits zghz3229;
lbits zghz3230;
lbits zghz3231;
lbits zghz3232;
lbits zghz3233;
lbits zghz3234;
lbits zghz3235;
lbits zghz3236;
sail_int zghz3237;
sail_int zghz3238;
sail_int zghz3239;
sail_int zghz3240;
sail_int zghz3241;
sail_int zghz3242;
sail_int zghz3243;
sail_int zghz3244;
sail_int zghz3245;
sail_int zghz3246;
sail_int zghz3247;
sail_int zghz3248;
sail_int zghz3249;
sail_int zghz3250;
sail_int zghz3251;
sail_int zghz3252;

static void startup_sailgen_CapBoundsEqual(void)
{
  CREATE(lbits)(&zghz3229);
  CREATE(lbits)(&zghz3230);
  CREATE(lbits)(&zghz3231);
  CREATE(lbits)(&zghz3232);
  CREATE(lbits)(&zghz3233);
  CREATE(lbits)(&zghz3234);
  CREATE(lbits)(&zghz3235);
  CREATE(lbits)(&zghz3236);
  CREATE(sail_int)(&zghz3237);
  CREATE(sail_int)(&zghz3238);
  CREATE(sail_int)(&zghz3239);
  CREATE(sail_int)(&zghz3240);
  CREATE(sail_int)(&zghz3241);
  CREATE(sail_int)(&zghz3242);
  CREATE(sail_int)(&zghz3243);
  CREATE(sail_int)(&zghz3244);
  CREATE(sail_int)(&zghz3245);
  CREATE(sail_int)(&zghz3246);
  CREATE(sail_int)(&zghz3247);
  CREATE(sail_int)(&zghz3248);
  CREATE(sail_int)(&zghz3249);
  CREATE(sail_int)(&zghz3250);
  CREATE(sail_int)(&zghz3251);
  CREATE(sail_int)(&zghz3252);
}

static bool sailgen_CapBoundsEqual(lbits za, lbits zb)
{
  __label__ cleanup_142, end_cleanup_143, end_function_141, end_block_exception_144;

  bool zcbz323;
  RECREATE(lbits)(&zghz3229);
  {
    int64_t zgaz3149;
    {
      int64_t zgaz3148;
      {
        int64_t zgaz3147;
        {
          RECREATE(sail_int)(&zghz3250);
          CONVERT_OF(sail_int, mach_int)(&zghz3250, INT64_C(63));
          RECREATE(sail_int)(&zghz3251);
          CONVERT_OF(sail_int, mach_int)(&zghz3251, INT64_C(0));
          RECREATE(sail_int)(&zghz3252);
          sub_int(&zghz3252, zghz3250, zghz3251);
          zgaz3147 = CONVERT_OF(mach_int, sail_int)(zghz3252);
        }
        zgaz3148 = (zgaz3147 + INT64_C(1));
      }
      zgaz3149 = (zgaz3148 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3249);
      CONVERT_OF(sail_int, mach_int)(&zghz3249, zgaz3149);
      UNDEFINED(lbits)(&zghz3229, zghz3249);
    }
  }
  RECREATE(lbits)(&zghz3230);
  {
    int64_t zgaz3146;
    {
      int64_t zgaz3145;
      {
        int64_t zgaz3144;
        {
          RECREATE(sail_int)(&zghz3246);
          CONVERT_OF(sail_int, mach_int)(&zghz3246, INT64_C(63));
          RECREATE(sail_int)(&zghz3247);
          CONVERT_OF(sail_int, mach_int)(&zghz3247, INT64_C(0));
          RECREATE(sail_int)(&zghz3248);
          sub_int(&zghz3248, zghz3246, zghz3247);
          zgaz3144 = CONVERT_OF(mach_int, sail_int)(zghz3248);
        }
        zgaz3145 = (zgaz3144 + INT64_C(1));
      }
      zgaz3146 = (zgaz3145 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3245);
      CONVERT_OF(sail_int, mach_int)(&zghz3245, zgaz3146);
      UNDEFINED(lbits)(&zghz3230, zghz3245);
    }
  }
  bool zavalid;
  zavalid = false;
  RECREATE(lbits)(&zghz3231);
  {
    int64_t zgaz3143;
    {
      int64_t zgaz3142;
      {
        int64_t zgaz3141;
        {
          RECREATE(sail_int)(&zghz3242);
          CONVERT_OF(sail_int, mach_int)(&zghz3242, INT64_C(63));
          RECREATE(sail_int)(&zghz3243);
          CONVERT_OF(sail_int, mach_int)(&zghz3243, INT64_C(0));
          RECREATE(sail_int)(&zghz3244);
          sub_int(&zghz3244, zghz3242, zghz3243);
          zgaz3141 = CONVERT_OF(mach_int, sail_int)(zghz3244);
        }
        zgaz3142 = (zgaz3141 + INT64_C(1));
      }
      zgaz3143 = (zgaz3142 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3241);
      CONVERT_OF(sail_int, mach_int)(&zghz3241, zgaz3143);
      UNDEFINED(lbits)(&zghz3231, zghz3241);
    }
  }
  RECREATE(lbits)(&zghz3232);
  {
    int64_t zgaz3140;
    {
      int64_t zgaz3139;
      {
        int64_t zgaz3138;
        {
          RECREATE(sail_int)(&zghz3238);
          CONVERT_OF(sail_int, mach_int)(&zghz3238, INT64_C(63));
          RECREATE(sail_int)(&zghz3239);
          CONVERT_OF(sail_int, mach_int)(&zghz3239, INT64_C(0));
          RECREATE(sail_int)(&zghz3240);
          sub_int(&zghz3240, zghz3238, zghz3239);
          zgaz3138 = CONVERT_OF(mach_int, sail_int)(zghz3240);
        }
        zgaz3139 = (zgaz3138 + INT64_C(1));
      }
      zgaz3140 = (zgaz3139 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3237);
      CONVERT_OF(sail_int, mach_int)(&zghz3237, zgaz3140);
      UNDEFINED(lbits)(&zghz3232, zghz3237);
    }
  }
  bool zbvalid;
  zbvalid = false;
  {
    __label__ case_138, finish_match_137;

    struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgaz3132;
    CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3132);
    sailgen_CapGetBounds(&zgaz3132, za);
    unit zgsz365;
    {
      RECREATE(lbits)(&zghz3235);
      COPY(lbits)(&zghz3235, zgaz3132.ztup0);
      RECREATE(lbits)(&zghz3236);
      COPY(lbits)(&zghz3236, zgaz3132.ztup1);
      bool ztup__2;
      ztup__2 = zgaz3132.ztup2;
      {
        COPY(lbits)(&zghz3229, zghz3235);
        unit zgsz367;
        zgsz367 = UNIT;
      }
      {
        COPY(lbits)(&zghz3230, zghz3236);
        unit zgsz366;
        zgsz366 = UNIT;
      }
      zavalid = ztup__2;
      zgsz365 = UNIT;
      goto finish_match_137;
    }
  case_138: ;
    sail_match_failure("CapBoundsEqual");
  finish_match_137: ;
    unit zgsz374;
    zgsz374 = zgsz365;
    KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3132);
  }
  {
    __label__ case_140, finish_match_139;

    struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgaz3133;
    CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3133);
    sailgen_CapGetBounds(&zgaz3133, zb);
    unit zgsz369;
    {
      RECREATE(lbits)(&zghz3233);
      COPY(lbits)(&zghz3233, zgaz3133.ztup0);
      RECREATE(lbits)(&zghz3234);
      COPY(lbits)(&zghz3234, zgaz3133.ztup1);
      bool zuz32;
      zuz32 = zgaz3133.ztup2;
      {
        COPY(lbits)(&zghz3231, zghz3233);
        unit zgsz371;
        zgsz371 = UNIT;
      }
      {
        COPY(lbits)(&zghz3232, zghz3234);
        unit zgsz370;
        zgsz370 = UNIT;
      }
      zbvalid = zuz32;
      zgsz369 = UNIT;
      goto finish_match_139;
    }
  case_140: ;
    sail_match_failure("CapBoundsEqual");
  finish_match_139: ;
    unit zgsz373;
    zgsz373 = zgsz369;
    KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3133);
  }
  bool zgaz3137;
  {
    bool zgaz3136;
    {
      bool zgaz3135;
      {
        bool zgaz3134;
        zgaz3134 = eq_bits(zghz3229, zghz3231);
        bool zgsz375;
        if (zgaz3134) {  zgsz375 = eq_bits(zghz3230, zghz3232);  } else {  zgsz375 = false;  }
        zgaz3135 = zgsz375;
      }
      bool zgsz376;
      if (zgaz3135) {  zgsz376 = zavalid;  } else {  zgsz376 = false;  }
      zgaz3136 = zgsz376;
    }
    bool zgsz377;
    if (zgaz3136) {  zgsz377 = zbvalid;  } else {  zgsz377 = false;  }
    zgaz3137 = zgsz377;
  }
  zcbz323 = zgaz3137;
  goto cleanup_142;
  /* unreachable after return */



  goto end_cleanup_143;
cleanup_142: ;



  goto end_function_141;
end_cleanup_143: ;
end_function_141: ;
  return zcbz323;
end_block_exception_144: ;

  return false;
}



static void finish_sailgen_CapBoundsEqual(void)
{
  KILL(sail_int)(&zghz3252);
  KILL(sail_int)(&zghz3251);
  KILL(sail_int)(&zghz3250);
  KILL(sail_int)(&zghz3249);
  KILL(sail_int)(&zghz3248);
  KILL(sail_int)(&zghz3247);
  KILL(sail_int)(&zghz3246);
  KILL(sail_int)(&zghz3245);
  KILL(sail_int)(&zghz3244);
  KILL(sail_int)(&zghz3243);
  KILL(sail_int)(&zghz3242);
  KILL(sail_int)(&zghz3241);
  KILL(sail_int)(&zghz3240);
  KILL(sail_int)(&zghz3239);
  KILL(sail_int)(&zghz3238);
  KILL(sail_int)(&zghz3237);
  KILL(lbits)(&zghz3236);
  KILL(lbits)(&zghz3235);
  KILL(lbits)(&zghz3234);
  KILL(lbits)(&zghz3233);
  KILL(lbits)(&zghz3232);
  KILL(lbits)(&zghz3231);
  KILL(lbits)(&zghz3230);
  KILL(lbits)(&zghz3229);
}

static bool sailgen_CapIsRepresentable(lbits, uint64_t);

lbits zghz3253;
sail_int zghz3254;
sail_int zghz3255;
lbits zghz3256;

static void startup_sailgen_CapIsRepresentable(void)
{
  CREATE(lbits)(&zghz3253);
  CREATE(sail_int)(&zghz3254);
  CREATE(sail_int)(&zghz3255);
  CREATE(lbits)(&zghz3256);
}

static bool sailgen_CapIsRepresentable(lbits zc, uint64_t zaddress)
{
  __label__ cleanup_147, end_cleanup_148, end_function_146, end_block_exception_149;

  bool zcbz324;
  RECREATE(lbits)(&zghz3253);
  COPY(lbits)(&zghz3253, zc);
  {
    {
      RECREATE(sail_int)(&zghz3254);
      CONVERT_OF(sail_int, mach_int)(&zghz3254, zCAP_VALUE_HI_BIT);
      RECREATE(sail_int)(&zghz3255);
      CONVERT_OF(sail_int, mach_int)(&zghz3255, zCAP_VALUE_LO_BIT);
      RECREATE(lbits)(&zghz3256);
      CONVERT_OF(lbits, fbits)(&zghz3256, zaddress, UINT64_C(64) , true);
      vector_update_subrange_lbits(&zghz3253, zghz3253, zghz3254, zghz3255, zghz3256);
    }
    unit zgsz378;
    zgsz378 = UNIT;
  }
  zcbz324 = sailgen_CapBoundsEqual(zc, zghz3253);
  goto cleanup_147;
  /* unreachable after return */
  goto end_cleanup_148;
cleanup_147: ;
  goto end_function_146;
end_cleanup_148: ;
end_function_146: ;
  return zcbz324;
end_block_exception_149: ;

  return false;
}



static void finish_sailgen_CapIsRepresentable(void)
{
  KILL(lbits)(&zghz3256);
  KILL(sail_int)(&zghz3255);
  KILL(sail_int)(&zghz3254);
  KILL(lbits)(&zghz3253);
}

static int64_t zCAP_TAG_BIT;
static void create_letbind_23(void) {


  int64_t zgsz379;
  zgsz379 = INT64_C(128);
  zCAP_TAG_BIT = zgsz379;

let_end_150: ;
}
static void kill_letbind_23(void) {
}

static void sailgen_CapSetTag(lbits *rop, lbits, uint64_t);

lbits zghz3257;
sail_int zghz3258;

static void startup_sailgen_CapSetTag(void)
{
  CREATE(lbits)(&zghz3257);
  CREATE(sail_int)(&zghz3258);
}

static void sailgen_CapSetTag(lbits *zcbz325, lbits zc, uint64_t zt)
{
  __label__ cleanup_153, end_cleanup_154, end_function_152, end_block_exception_155, end_function_287;

  RECREATE(lbits)(&zghz3257);
  COPY(lbits)(&zghz3257, zc);
  {
    fbits zgaz3153;
    {
      uint64_t zgaz3152;
      {
        fbits zgaz3151;
        zgaz3151 = (UINT64_C(1) & (zt >> INT64_C(0)));
        zgaz3152 = UINT64_C(0b0);
        zgaz3152 = update_fbits(zgaz3152, INT64_C(0), zgaz3151);
      }
      zgaz3153 = sailgen_Bit(zgaz3152);
    }
    {
      RECREATE(sail_int)(&zghz3258);
      CONVERT_OF(sail_int, mach_int)(&zghz3258, zCAP_TAG_BIT);
      update_lbits(&zghz3257, zghz3257, zghz3258, zgaz3153);
    }
    unit zgsz381;
    zgsz381 = UNIT;
  }
  COPY(lbits)((*(&zcbz325)), zghz3257);
  goto cleanup_153;
  /* unreachable after return */
  goto end_cleanup_154;
cleanup_153: ;
  goto end_function_152;
end_cleanup_154: ;
end_function_152: ;
  goto end_function_287;
end_block_exception_155: ;
  goto end_function_287;
end_function_287: ;
}



static void finish_sailgen_CapSetTag(void)
{
  KILL(sail_int)(&zghz3258);
  KILL(lbits)(&zghz3257);
}

static void sailgen_CapWithTagClear(lbits *rop, lbits);

lbits zghz3259;
lbits zghz3260;
sail_int zghz3261;
lbits zghz3262;

static void startup_sailgen_CapWithTagClear(void)
{
  CREATE(lbits)(&zghz3259);
  CREATE(lbits)(&zghz3260);
  CREATE(sail_int)(&zghz3261);
  CREATE(lbits)(&zghz3262);
}

static void sailgen_CapWithTagClear(lbits *zcbz326, lbits zc)
{
  __label__ cleanup_158, end_cleanup_159, end_function_157, end_block_exception_160, end_function_286;

  RECREATE(lbits)(&zghz3259);
  {
    uint64_t zgaz3154;
    {
      RECREATE(lbits)(&zghz3260);
      CONVERT_OF(lbits, fbits)(&zghz3260, UINT64_C(0b0), UINT64_C(1) , true);
      RECREATE(sail_int)(&zghz3261);
      CONVERT_OF(sail_int, mach_int)(&zghz3261, INT64_C(64));
      RECREATE(lbits)(&zghz3262);
      zero_extend(&zghz3262, zghz3260, zghz3261);
      zgaz3154 = CONVERT_OF(fbits, lbits)(zghz3262, true);
    }
    sailgen_CapSetTag(&zghz3259, zc, zgaz3154);
  }
  COPY(lbits)((*(&zcbz326)), zghz3259);
  goto cleanup_158;
  /* unreachable after return */
  goto end_cleanup_159;
cleanup_158: ;
  goto end_function_157;
end_cleanup_159: ;
end_function_157: ;
  goto end_function_286;
end_block_exception_160: ;
  goto end_function_286;
end_function_286: ;
}



static void finish_sailgen_CapWithTagClear(void)
{
  KILL(lbits)(&zghz3262);
  KILL(sail_int)(&zghz3261);
  KILL(lbits)(&zghz3260);
  KILL(lbits)(&zghz3259);
}

// register _PC
static uint64_t z_PC;

// register __BranchTaken
static bool z__BranchTaken;

static int64_t zCAP_OTYPE_HI_BIT;
static void create_letbind_24(void) {


  int64_t zgsz382;
  zgsz382 = INT64_C(109);
  zCAP_OTYPE_HI_BIT = zgsz382;

let_end_161: ;
}
static void kill_letbind_24(void) {
}

static int64_t zCAP_OTYPE_LO_BIT;
static void create_letbind_25(void) {


  int64_t zgsz383;
  zgsz383 = INT64_C(95);
  zCAP_OTYPE_LO_BIT = zgsz383;

let_end_162: ;
}
static void kill_letbind_25(void) {
}

static uint64_t sailgen_CapGetObjectType(lbits);

lbits zghz3263;
sail_int zghz3264;
lbits zghz3265;
sail_int zghz3266;
sail_int zghz3267;
lbits zghz3268;

static void startup_sailgen_CapGetObjectType(void)
{
  CREATE(lbits)(&zghz3263);
  CREATE(sail_int)(&zghz3264);
  CREATE(lbits)(&zghz3265);
  CREATE(sail_int)(&zghz3266);
  CREATE(sail_int)(&zghz3267);
  CREATE(lbits)(&zghz3268);
}

static uint64_t sailgen_CapGetObjectType(lbits zc)
{
  __label__ cleanup_165, end_cleanup_166, end_function_164, end_block_exception_167;

  uint64_t zcbz327;
  {
    uint64_t zgaz3156;
    {
      RECREATE(sail_int)(&zghz3266);
      CONVERT_OF(sail_int, mach_int)(&zghz3266, zCAP_OTYPE_HI_BIT);
      RECREATE(sail_int)(&zghz3267);
      CONVERT_OF(sail_int, mach_int)(&zghz3267, zCAP_OTYPE_LO_BIT);
      RECREATE(lbits)(&zghz3268);
      vector_subrange_lbits(&zghz3268, zc, zghz3266, zghz3267);
      zgaz3156 = CONVERT_OF(fbits, lbits)(zghz3268, true);
    }
    {
      RECREATE(lbits)(&zghz3263);
      CONVERT_OF(lbits, fbits)(&zghz3263, zgaz3156, UINT64_C(15) , true);
      RECREATE(sail_int)(&zghz3264);
      CONVERT_OF(sail_int, mach_int)(&zghz3264, zCAP_VALUE_NUM_BITS);
      RECREATE(lbits)(&zghz3265);
      zero_extend(&zghz3265, zghz3263, zghz3264);
      zcbz327 = CONVERT_OF(fbits, lbits)(zghz3265, true);
    }
  }
  goto cleanup_165;
  /* unreachable after return */
  goto end_cleanup_166;
cleanup_165: ;
  goto end_function_164;
end_cleanup_166: ;
end_function_164: ;
  return zcbz327;
end_block_exception_167: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetObjectType(void)
{
  KILL(lbits)(&zghz3268);
  KILL(sail_int)(&zghz3267);
  KILL(sail_int)(&zghz3266);
  KILL(lbits)(&zghz3265);
  KILL(sail_int)(&zghz3264);
  KILL(lbits)(&zghz3263);
}

static bool sailgen_CapIsSealed(lbits);

sail_int zghz3269;
lbits zghz3270;

static void startup_sailgen_CapIsSealed(void)
{
  CREATE(sail_int)(&zghz3269);
  CREATE(lbits)(&zghz3270);
}

static bool sailgen_CapIsSealed(lbits zc)
{
  __label__ cleanup_170, end_cleanup_171, end_function_169, end_block_exception_172;

  bool zcbz328;
  {
    uint64_t zgaz3158;
    zgaz3158 = sailgen_CapGetObjectType(zc);
    uint64_t zgaz3159;
    {
      RECREATE(sail_int)(&zghz3269);
      CONVERT_OF(sail_int, mach_int)(&zghz3269, zCAP_VALUE_NUM_BITS);
      RECREATE(lbits)(&zghz3270);
      sailgen_Zeros(&zghz3270, zghz3269);
      zgaz3159 = CONVERT_OF(fbits, lbits)(zghz3270, true);
    }
    zcbz328 = (zgaz3158 != zgaz3159);
  }
  goto cleanup_170;
  /* unreachable after return */
  goto end_cleanup_171;
cleanup_170: ;
  goto end_function_169;
end_cleanup_171: ;
end_function_169: ;
  return zcbz328;
end_block_exception_172: ;

  return false;
}



static void finish_sailgen_CapIsSealed(void)
{
  KILL(lbits)(&zghz3270);
  KILL(sail_int)(&zghz3269);
}

static int64_t zCAP_FLAGS_HI_BIT;
static void create_letbind_26(void) {


  int64_t zgsz384;
  zgsz384 = INT64_C(63);
  zCAP_FLAGS_HI_BIT = zgsz384;

let_end_173: ;
}
static void kill_letbind_26(void) {
}

// register CCTLR_EL0
static uint64_t zCCTLR_EL0;

// register CCTLR_EL1
static uint64_t zCCTLR_EL1;

// register CCTLR_EL2
static uint64_t zCCTLR_EL2;

// register CCTLR_EL3
static uint64_t zCCTLR_EL3;

// register ELR_EL1
static lbits zELR_EL1;

// register ELR_EL2
static lbits zELR_EL2;

// register ELR_EL3
static lbits zELR_EL3;

// register VBAR_EL1
static lbits zVBAR_EL1;

// register VBAR_EL2
static lbits zVBAR_EL2;

// register VBAR_EL3
static lbits zVBAR_EL3;

// register CPACR_EL1
static uint64_t zCPACR_EL1;

// register CPTR_EL2
static uint64_t zCPTR_EL2;

// register CPTR_EL3
static uint64_t zCPTR_EL3;

static uint64_t zM32_Monitor;
static void create_letbind_27(void) {


  uint64_t zgsz385;
  zgsz385 = UINT64_C(0b10110);
  zM32_Monitor = zgsz385;

let_end_174: ;
}
static void kill_letbind_27(void) {
}

static uint64_t zCAP_PERM_EXECUTIVE;
static void create_letbind_28(void) {


  uint64_t zgsz386;
  int64_t zgaz3161;
  zgaz3161 = shl_mach_int(INT64_C(1), INT64_C(1));
  {
    sail_int zgsz3820;
    CREATE(sail_int)(&zgsz3820);
    CONVERT_OF(sail_int, mach_int)(&zgsz3820, zgaz3161);
    sail_int zgsz3821;
    CREATE(sail_int)(&zgsz3821);
    CONVERT_OF(sail_int, mach_int)(&zgsz3821, INT64_C(63));
    sail_int zgsz3822;
    CREATE(sail_int)(&zgsz3822);
    CONVERT_OF(sail_int, mach_int)(&zgsz3822, INT64_C(0));
    lbits zgsz3823;
    CREATE(lbits)(&zgsz3823);
    sailgen_integer_subrange(&zgsz3823, zgsz3820, zgsz3821, zgsz3822);
    zgsz386 = CONVERT_OF(fbits, lbits)(zgsz3823, true);
    KILL(lbits)(&zgsz3823);
    KILL(sail_int)(&zgsz3822);
    KILL(sail_int)(&zgsz3821);
    KILL(sail_int)(&zgsz3820);
  }

  zCAP_PERM_EXECUTIVE = zgsz386;

let_end_175: ;
}
static void kill_letbind_28(void) {
}

static int64_t zCAP_PERMS_HI_BIT;
static void create_letbind_29(void) {


  int64_t zgsz387;
  zgsz387 = INT64_C(127);
  zCAP_PERMS_HI_BIT = zgsz387;

let_end_176: ;
}
static void kill_letbind_29(void) {
}

static int64_t zCAP_PERMS_LO_BIT;
static void create_letbind_30(void) {


  int64_t zgsz388;
  zgsz388 = INT64_C(110);
  zCAP_PERMS_LO_BIT = zgsz388;

let_end_177: ;
}
static void kill_letbind_30(void) {
}

static int64_t zCAP_PERMS_NUM_BITS;
static void create_letbind_31(void) {


  int64_t zgsz389;
  int64_t zgaz3162;
  {
    sail_int zgsz3824;
    CREATE(sail_int)(&zgsz3824);
    CONVERT_OF(sail_int, mach_int)(&zgsz3824, zCAP_PERMS_HI_BIT);
    sail_int zgsz3825;
    CREATE(sail_int)(&zgsz3825);
    CONVERT_OF(sail_int, mach_int)(&zgsz3825, zCAP_PERMS_LO_BIT);
    sail_int zgsz3826;
    CREATE(sail_int)(&zgsz3826);
    sub_int(&zgsz3826, zgsz3824, zgsz3825);
    zgaz3162 = CONVERT_OF(mach_int, sail_int)(zgsz3826);
    KILL(sail_int)(&zgsz3826);
    KILL(sail_int)(&zgsz3825);
    KILL(sail_int)(&zgsz3824);
  }
  zgsz389 = (zgaz3162 + INT64_C(1));

  zCAP_PERMS_NUM_BITS = zgsz389;

let_end_178: ;
}
static void kill_letbind_31(void) {
}

static uint64_t sailgen_CapGetPermissions(lbits);

sail_int zghz3271;
sail_int zghz3272;
lbits zghz3273;

static void startup_sailgen_CapGetPermissions(void)
{
  CREATE(sail_int)(&zghz3271);
  CREATE(sail_int)(&zghz3272);
  CREATE(lbits)(&zghz3273);
}

static uint64_t sailgen_CapGetPermissions(lbits zc)
{
  __label__ cleanup_181, end_cleanup_182, end_function_180, end_block_exception_183;

  uint64_t zcbz329;
  {
    RECREATE(sail_int)(&zghz3271);
    CONVERT_OF(sail_int, mach_int)(&zghz3271, zCAP_PERMS_HI_BIT);
    RECREATE(sail_int)(&zghz3272);
    CONVERT_OF(sail_int, mach_int)(&zghz3272, zCAP_PERMS_LO_BIT);
    RECREATE(lbits)(&zghz3273);
    vector_subrange_lbits(&zghz3273, zc, zghz3271, zghz3272);
    zcbz329 = CONVERT_OF(fbits, lbits)(zghz3273, true);
  }
  goto cleanup_181;
  /* unreachable after return */
  goto end_cleanup_182;
cleanup_181: ;
  goto end_function_180;
end_cleanup_182: ;
end_function_180: ;
  return zcbz329;
end_block_exception_183: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetPermissions(void)
{
  KILL(lbits)(&zghz3273);
  KILL(sail_int)(&zghz3272);
  KILL(sail_int)(&zghz3271);
}

// register EDSCR
static uint64_t zEDSCR;

// register SCTLR_EL1
static uint64_t zSCTLR_EL1;

// register SCTLR_EL2
static uint64_t zSCTLR_EL2;

// register SCTLR_EL3
static uint64_t zSCTLR_EL3;

// register SPSR_EL1
static uint64_t zSPSR_EL1;

// register SPSR_EL2
static uint64_t zSPSR_EL2;

// register SPSR_EL3
static uint64_t zSPSR_EL3;

// register ACTLR_EL1
static uint64_t zACTLR_EL1;

static uint64_t zCAP_PERM_EXECUTE;
static void create_letbind_32(void) {


  uint64_t zgsz390;
  sail_int zgaz3164;
  CREATE(sail_int)(&zgaz3164);
  {
    sail_int zgsz3833;
    CREATE(sail_int)(&zgsz3833);
    CONVERT_OF(sail_int, mach_int)(&zgsz3833, INT64_C(1));
    sail_int zgsz3834;
    CREATE(sail_int)(&zgsz3834);
    CONVERT_OF(sail_int, mach_int)(&zgsz3834, INT64_C(15));
    shl_int(&zgaz3164, zgsz3833, zgsz3834);
    KILL(sail_int)(&zgsz3834);
    KILL(sail_int)(&zgsz3833);
  }
  {
    sail_int zgsz3830;
    CREATE(sail_int)(&zgsz3830);
    CONVERT_OF(sail_int, mach_int)(&zgsz3830, INT64_C(63));
    sail_int zgsz3831;
    CREATE(sail_int)(&zgsz3831);
    CONVERT_OF(sail_int, mach_int)(&zgsz3831, INT64_C(0));
    lbits zgsz3832;
    CREATE(lbits)(&zgsz3832);
    sailgen_integer_subrange(&zgsz3832, zgaz3164, zgsz3830, zgsz3831);
    zgsz390 = CONVERT_OF(fbits, lbits)(zgsz3832, true);
    KILL(lbits)(&zgsz3832);
    KILL(sail_int)(&zgsz3831);
    KILL(sail_int)(&zgsz3830);
  }
  KILL(sail_int)(&zgaz3164);
  zCAP_PERM_EXECUTE = zgsz390;

let_end_184: ;
}
static void kill_letbind_32(void) {
}

static uint64_t zCAP_PERM_SYSTEM;
static void create_letbind_33(void) {


  uint64_t zgsz391;
  sail_int zgaz3165;
  CREATE(sail_int)(&zgaz3165);
  {
    sail_int zgsz3838;
    CREATE(sail_int)(&zgsz3838);
    CONVERT_OF(sail_int, mach_int)(&zgsz3838, INT64_C(1));
    sail_int zgsz3839;
    CREATE(sail_int)(&zgsz3839);
    CONVERT_OF(sail_int, mach_int)(&zgsz3839, INT64_C(9));
    shl_int(&zgaz3165, zgsz3838, zgsz3839);
    KILL(sail_int)(&zgsz3839);
    KILL(sail_int)(&zgsz3838);
  }
  {
    sail_int zgsz3835;
    CREATE(sail_int)(&zgsz3835);
    CONVERT_OF(sail_int, mach_int)(&zgsz3835, INT64_C(63));
    sail_int zgsz3836;
    CREATE(sail_int)(&zgsz3836);
    CONVERT_OF(sail_int, mach_int)(&zgsz3836, INT64_C(0));
    lbits zgsz3837;
    CREATE(lbits)(&zgsz3837);
    sailgen_integer_subrange(&zgsz3837, zgaz3165, zgsz3835, zgsz3836);
    zgsz391 = CONVERT_OF(fbits, lbits)(zgsz3837, true);
    KILL(lbits)(&zgsz3837);
    KILL(sail_int)(&zgsz3836);
    KILL(sail_int)(&zgsz3835);
  }
  KILL(sail_int)(&zgaz3165);
  zCAP_PERM_SYSTEM = zgsz391;

let_end_185: ;
}
static void kill_letbind_33(void) {
}

// register ACTLR_EL2
static uint64_t zACTLR_EL2;

// register ACTLR_EL3
static uint64_t zACTLR_EL3;

// register AFSR0_EL1
static uint64_t zAFSR0_EL1;

// register AFSR0_EL2
static uint64_t zAFSR0_EL2;

// register AFSR0_EL3
static uint64_t zAFSR0_EL3;

// register AFSR1_EL1
static uint64_t zAFSR1_EL1;

// register AFSR1_EL2
static uint64_t zAFSR1_EL2;

// register AFSR1_EL3
static uint64_t zAFSR1_EL3;

// register AIDR_EL1
static uint64_t zAIDR_EL1;

// register AMAIR_EL1
static uint64_t zAMAIR_EL1;

// register AMAIR_EL2
static uint64_t zAMAIR_EL2;

// register AMAIR_EL3
static uint64_t zAMAIR_EL3;

// register CCSIDR_EL1
static uint64_t zCCSIDR_EL1;

// register CHCR_EL2
static uint64_t zCHCR_EL2;

// register CLIDR_EL1
static uint64_t zCLIDR_EL1;

// register CNTFRQ_EL0
static uint64_t zCNTFRQ_EL0;

// register CNTHCTL_EL2
static uint64_t zCNTHCTL_EL2;

// register CNTKCTL_EL1
static uint64_t zCNTKCTL_EL1;

// register CNTHP_CTL_EL2
static uint64_t zCNTHP_CTL_EL2;

// register CNTHP_CVAL_EL2
static uint64_t zCNTHP_CVAL_EL2;

// register CNTHP_TVAL_EL2
static uint64_t zCNTHP_TVAL_EL2;

// register CNTHV_CTL_EL2
static uint64_t zCNTHV_CTL_EL2;

// register CNTHV_CVAL_EL2
static uint64_t zCNTHV_CVAL_EL2;

// register CNTHV_TVAL_EL2
static uint64_t zCNTHV_TVAL_EL2;

// register CNTPCT_EL0
static uint64_t zCNTPCT_EL0;

// register CNTPS_CTL_EL1
static uint64_t zCNTPS_CTL_EL1;

// register CNTPS_CVAL_EL1
static uint64_t zCNTPS_CVAL_EL1;

// register CNTPS_TVAL_EL1
static uint64_t zCNTPS_TVAL_EL1;

// register CNTP_CTL_EL0
static uint64_t zCNTP_CTL_EL0;

// register CNTP_CVAL_EL0
static uint64_t zCNTP_CVAL_EL0;

// register CNTP_TVAL_EL0
static uint64_t zCNTP_TVAL_EL0;

// register CNTVCT_EL0
static uint64_t zCNTVCT_EL0;

// register CNTVOFF_EL2
static uint64_t zCNTVOFF_EL2;

// register CNTV_CTL_EL0
static uint64_t zCNTV_CTL_EL0;

// register CNTV_CVAL_EL0
static uint64_t zCNTV_CVAL_EL0;

// register CNTV_TVAL_EL0
static uint64_t zCNTV_TVAL_EL0;

// register CONTEXTIDR_EL1
static uint64_t zCONTEXTIDR_EL1;

// register CONTEXTIDR_EL2
static uint64_t zCONTEXTIDR_EL2;

// register CSCR_EL3
static uint64_t zCSCR_EL3;

// register CSSELR_EL1
static uint64_t zCSSELR_EL1;

// register CTR_EL0
static uint64_t zCTR_EL0;

// register DACR32_EL2
static uint64_t zDACR32_EL2;

// register DBGAUTHSTATUS_EL1
static uint64_t zDBGAUTHSTATUS_EL1;

// register MDCR_EL2
static uint64_t zMDCR_EL2;

// register MDCR_EL3
static uint64_t zMDCR_EL3;

// register DBGBCR_EL1
static zz5vecz8z5bv32z9 zDBGBCR_EL1;

static uint64_t zDebugHalt_SoftwareAccess;
static void create_letbind_34(void) {


  uint64_t zgsz392;
  zgsz392 = UINT64_C(0b110011);
  zDebugHalt_SoftwareAccess = zgsz392;

let_end_186: ;
}
static void kill_letbind_34(void) {
}

// register CDLR_EL0
static lbits zCDLR_EL0;

// register DSPSR_EL0
static uint64_t zDSPSR_EL0;

// register DBGEN
static enum zsignal zDBGEN;

// register SPIDEN
static enum zsignal zSPIDEN;

// register DBGPRCR_EL1
static uint64_t zDBGPRCR_EL1;

// register OSDLR_EL1
static uint64_t zOSDLR_EL1;

// register OSLSR_EL1
static uint64_t zOSLSR_EL1;

// register DBGBVR_EL1
static zz5vecz8z5bv64z9 zDBGBVR_EL1;

// register DBGCLAIMCLR_EL1
static uint64_t zDBGCLAIMCLR_EL1;

// register DBGCLAIMSET_EL1
static uint64_t zDBGCLAIMSET_EL1;

// register DBGDTRRX_EL0
static uint64_t zDBGDTRRX_EL0;

// register MDSCR_EL1
static uint64_t zMDSCR_EL1;

// register CDBGDTR_EL0
static lbits zCDBGDTR_EL0;

// register DBGVCR32_EL2
static uint64_t zDBGVCR32_EL2;

// register DBGWCR_EL1
static zz5vecz8z5bv32z9 zDBGWCR_EL1;

// register DBGWVR_EL1
static zz5vecz8z5bv64z9 zDBGWVR_EL1;

// register DCZID_EL0
static uint64_t zDCZID_EL0;

// register DISR_EL1
static uint64_t zDISR_EL1;

// register VDISR_EL2
static uint64_t zVDISR_EL2;

// register ERRIDR_EL1
static uint64_t zERRIDR_EL1;

// register ERRSELR_EL1
static uint64_t zERRSELR_EL1;

// register ERXADDR_EL1
static uint64_t zERXADDR_EL1;

// register ERXCTLR_EL1
static uint64_t zERXCTLR_EL1;

// register ERXFR_EL1
static uint64_t zERXFR_EL1;

// register ERXMISC0_EL1
static uint64_t zERXMISC0_EL1;

// register ERXMISC1_EL1
static uint64_t zERXMISC1_EL1;

// register ERXSTATUS_EL1
static uint64_t zERXSTATUS_EL1;

// register FPCR
static uint64_t zFPCR;

// register FPEXC32_EL2
static uint64_t zFPEXC32_EL2;

// register FPSR
static uint64_t zFPSR;

// register HACR_EL2
static uint64_t zHACR_EL2;

// register HSTR_EL2
static uint64_t zHSTR_EL2;

// register ICC_AP0R_EL1
static zz5vecz8z5bv32z9 zICC_AP0R_EL1;

// register ICC_SRE_EL1_NS
static uint64_t zICC_SRE_EL1_NS;

// register ICC_SRE_EL1_S
static uint64_t zICC_SRE_EL1_S;

// register ICC_SRE_EL2
static uint64_t zICC_SRE_EL2;

// register ICC_SRE_EL3
static uint64_t zICC_SRE_EL3;

// register ICH_HCR_EL2
static uint64_t zICH_HCR_EL2;

// register ICV_AP0R_EL1
static zz5vecz8z5bv32z9 zICV_AP0R_EL1;

// register ICC_AP1R_EL1
static zz5vecz8z5bv32z9 zICC_AP1R_EL1;

// register ICC_AP1R_EL1_NS
static zz5vecz8z5bv32z9 zICC_AP1R_EL1_NS;

// register ICC_AP1R_EL1_S
static zz5vecz8z5bv32z9 zICC_AP1R_EL1_S;

// register ICV_AP1R_EL1
static zz5vecz8z5bv32z9 zICV_AP1R_EL1;

// register ICC_BPR0_EL1
static uint64_t zICC_BPR0_EL1;

// register ICV_BPR0_EL1
static uint64_t zICV_BPR0_EL1;

// register ICC_BPR1_EL1_NS
static uint64_t zICC_BPR1_EL1_NS;

// register ICC_BPR1_EL1_S
static uint64_t zICC_BPR1_EL1_S;

// register ICV_BPR1_EL1
static uint64_t zICV_BPR1_EL1;

// register ICC_CTLR_EL1_NS
static uint64_t zICC_CTLR_EL1_NS;

// register ICC_CTLR_EL1_S
static uint64_t zICC_CTLR_EL1_S;

// register ICV_CTLR_EL1
static uint64_t zICV_CTLR_EL1;

// register ICC_CTLR_EL3
static uint64_t zICC_CTLR_EL3;

// register ICC_HPPIR0_EL1
static uint64_t zICC_HPPIR0_EL1;

// register ICV_HPPIR0_EL1
static uint64_t zICV_HPPIR0_EL1;

// register ICC_HPPIR1_EL1
static uint64_t zICC_HPPIR1_EL1;

// register ICV_HPPIR1_EL1
static uint64_t zICV_HPPIR1_EL1;

// register ICC_IAR0_EL1
static uint64_t zICC_IAR0_EL1;

// register ICV_IAR0_EL1
static uint64_t zICV_IAR0_EL1;

// register ICC_IAR1_EL1
static uint64_t zICC_IAR1_EL1;

// register ICV_IAR1_EL1
static uint64_t zICV_IAR1_EL1;

// register ICC_IGRPEN0_EL1
static uint64_t zICC_IGRPEN0_EL1;

// register ICV_IGRPEN0_EL1
static uint64_t zICV_IGRPEN0_EL1;

// register ICC_IGRPEN1_EL1_NS
static uint64_t zICC_IGRPEN1_EL1_NS;

// register ICC_IGRPEN1_EL1_S
static uint64_t zICC_IGRPEN1_EL1_S;

// register ICV_IGRPEN1_EL1
static uint64_t zICV_IGRPEN1_EL1;

// register ICC_IGRPEN1_EL3
static uint64_t zICC_IGRPEN1_EL3;

// register ICC_PMR_EL1
static uint64_t zICC_PMR_EL1;

// register ICV_PMR_EL1
static uint64_t zICV_PMR_EL1;

// register ICC_RPR_EL1
static uint64_t zICC_RPR_EL1;

// register ICV_RPR_EL1
static uint64_t zICV_RPR_EL1;

// register ICH_AP0R_EL2
static zz5vecz8z5bv32z9 zICH_AP0R_EL2;

// register ICH_AP1R_EL2
static zz5vecz8z5bv32z9 zICH_AP1R_EL2;

// register ICH_EISR_EL2
static uint64_t zICH_EISR_EL2;

// register ICH_ELRSR_EL2
static uint64_t zICH_ELRSR_EL2;

// register ICH_LR_EL2
static zz5vecz8z5bv64z9 zICH_LR_EL2;

// register ICH_MISR_EL2
static uint64_t zICH_MISR_EL2;

// register ICH_VMCR_EL2
static uint64_t zICH_VMCR_EL2;

// register ICH_VTR_EL2
static uint64_t zICH_VTR_EL2;

// register ID_AA64AFR0_EL1
static uint64_t zID_AA64AFR0_EL1;

// register ID_AA64AFR1_EL1
static uint64_t zID_AA64AFR1_EL1;

// register ID_AA64DFR0_EL1
static uint64_t zID_AA64DFR0_EL1;

// register ID_AA64DFR1_EL1
static uint64_t zID_AA64DFR1_EL1;

// register ID_AA64ISAR0_EL1
static uint64_t zID_AA64ISAR0_EL1;

// register ID_AA64ISAR1_EL1
static uint64_t zID_AA64ISAR1_EL1;

// register ID_AA64MMFR0_EL1
static uint64_t zID_AA64MMFR0_EL1;

// register ID_AA64MMFR1_EL1
static uint64_t zID_AA64MMFR1_EL1;

// register ID_AA64MMFR2_EL1
static uint64_t zID_AA64MMFR2_EL1;

// register ID_AA64PFR0_EL1
static uint64_t zID_AA64PFR0_EL1;

// register ID_AA64PFR1_EL1
static uint64_t zID_AA64PFR1_EL1;

// register ID_AA64ZFR0_EL1
static uint64_t zID_AA64ZFR0_EL1;

// register ID_AFR0_EL1
static uint64_t zID_AFR0_EL1;

// register ID_DFR0_EL1
static uint64_t zID_DFR0_EL1;

// register ID_ISAR0_EL1
static uint64_t zID_ISAR0_EL1;

// register ID_ISAR1_EL1
static uint64_t zID_ISAR1_EL1;

// register ID_ISAR2_EL1
static uint64_t zID_ISAR2_EL1;

// register ID_ISAR3_EL1
static uint64_t zID_ISAR3_EL1;

// register ID_ISAR4_EL1
static uint64_t zID_ISAR4_EL1;

// register ID_ISAR5_EL1
static uint64_t zID_ISAR5_EL1;

// register ID_ISAR6_EL1
static uint64_t zID_ISAR6_EL1;

// register ID_MMFR0_EL1
static uint64_t zID_MMFR0_EL1;

// register ID_MMFR1_EL1
static uint64_t zID_MMFR1_EL1;

// register ID_MMFR2_EL1
static uint64_t zID_MMFR2_EL1;

// register ID_MMFR3_EL1
static uint64_t zID_MMFR3_EL1;

// register ID_MMFR4_EL1
static uint64_t zID_MMFR4_EL1;

// register ID_MMFR5_EL1
static uint64_t zID_MMFR5_EL1;

// register ID_PFR0_EL1
static uint64_t zID_PFR0_EL1;

// register ID_PFR1_EL1
static uint64_t zID_PFR1_EL1;

// register ID_PFR2_EL1
static uint64_t zID_PFR2_EL1;

// register IFSR32_EL2
static uint64_t zIFSR32_EL2;

// register ISR_EL1
static uint64_t zISR_EL1;

// register LORC_EL1
static uint64_t zLORC_EL1;

// register LOREA_EL1
static uint64_t zLOREA_EL1;

// register LORID_EL1
static uint64_t zLORID_EL1;

// register LORN_EL1
static uint64_t zLORN_EL1;

// register LORSA_EL1
static uint64_t zLORSA_EL1;

// register MAIR_EL1
static uint64_t zMAIR_EL1;

// register MAIR_EL2
static uint64_t zMAIR_EL2;

// register MAIR_EL3
static uint64_t zMAIR_EL3;

// register MDCCINT_EL1
static uint64_t zMDCCINT_EL1;

// register MDCCSR_EL0
static uint64_t zMDCCSR_EL0;

// register MDRAR_EL1
static uint64_t zMDRAR_EL1;

// register MIDR_EL1
static uint64_t zMIDR_EL1;

// register VPIDR_EL2
static uint64_t zVPIDR_EL2;

// register MPAM0_EL1
static uint64_t zMPAM0_EL1;

// register MPAM3_EL3
static uint64_t zMPAM3_EL3;

// register _MPAM2_EL2_0_62
static uint64_t z_MPAM2_EL2_0_62;

// register _MPAM1_EL1_0_62
static uint64_t z_MPAM1_EL1_0_62;

// register MPAMHCR_EL2
static uint64_t zMPAMHCR_EL2;

// register MPAMIDR_EL1
static uint64_t zMPAMIDR_EL1;

// register MPAMVPM0_EL2
static uint64_t zMPAMVPM0_EL2;

// register MPAMVPM1_EL2
static uint64_t zMPAMVPM1_EL2;

// register MPAMVPM2_EL2
static uint64_t zMPAMVPM2_EL2;

// register MPAMVPM3_EL2
static uint64_t zMPAMVPM3_EL2;

// register MPAMVPM4_EL2
static uint64_t zMPAMVPM4_EL2;

// register MPAMVPM5_EL2
static uint64_t zMPAMVPM5_EL2;

// register MPAMVPM6_EL2
static uint64_t zMPAMVPM6_EL2;

// register MPAMVPM7_EL2
static uint64_t zMPAMVPM7_EL2;

// register MPAMVPMV_EL2
static uint64_t zMPAMVPMV_EL2;

// register MPIDR_EL1
static uint64_t zMPIDR_EL1;

// register VMPIDR_EL2
static uint64_t zVMPIDR_EL2;

// register MVFR0_EL1
static uint64_t zMVFR0_EL1;

// register MVFR1_EL1
static uint64_t zMVFR1_EL1;

// register MVFR2_EL1
static uint64_t zMVFR2_EL1;

// register OSDTRRX_EL1
static uint64_t zOSDTRRX_EL1;

// register OSDTRTX_EL1
static uint64_t zOSDTRTX_EL1;

// register OSECCR_EL1
static uint64_t zOSECCR_EL1;

// register PAR_EL1
static uint64_t zPAR_EL1;

// register PMBIDR_EL1
static uint64_t zPMBIDR_EL1;

// register PMBLIMITR_EL1
static uint64_t zPMBLIMITR_EL1;

// register PMBPTR_EL1
static uint64_t zPMBPTR_EL1;

// register PMBSR_EL1
static uint64_t zPMBSR_EL1;

// register PMCCFILTR_EL0
static uint64_t zPMCCFILTR_EL0;

// register PMUSERENR_EL0
static uint64_t zPMUSERENR_EL0;

// register PMCCNTR_EL0
static uint64_t zPMCCNTR_EL0;

// register PMCEID0_EL0
static uint64_t zPMCEID0_EL0;

// register PMCEID1_EL0
static uint64_t zPMCEID1_EL0;

// register PMCNTENCLR_EL0
static uint64_t zPMCNTENCLR_EL0;

// register PMCNTENSET_EL0
static uint64_t zPMCNTENSET_EL0;

// register PMCR_EL0
static uint64_t zPMCR_EL0;

// register PMEVCNTR_EL0
static zz5vecz8z5bv32z9 zPMEVCNTR_EL0;

// register PMEVTYPER_EL0
static zz5vecz8z5bv32z9 zPMEVTYPER_EL0;

// register PMINTENCLR_EL1
static uint64_t zPMINTENCLR_EL1;

// register PMINTENSET_EL1
static uint64_t zPMINTENSET_EL1;

// register PMOVSCLR_EL0
static uint64_t zPMOVSCLR_EL0;

// register PMOVSSET_EL0
static uint64_t zPMOVSSET_EL0;

// register PMSCR_EL1
static uint64_t zPMSCR_EL1;

// register PMSCR_EL2
static uint64_t zPMSCR_EL2;

// register PMSELR_EL0
static uint64_t zPMSELR_EL0;

// register PMSEVFR_EL1
static uint64_t zPMSEVFR_EL1;

// register PMSFCR_EL1
static uint64_t zPMSFCR_EL1;

// register PMSICR_EL1
static uint64_t zPMSICR_EL1;

// register PMSIDR_EL1
static uint64_t zPMSIDR_EL1;

// register PMSIRR_EL1
static uint64_t zPMSIRR_EL1;

// register PMSLATFR_EL1
static uint64_t zPMSLATFR_EL1;

// register PMXEVCNTR_EL0
static uint64_t zPMXEVCNTR_EL0;

// register PMXEVTYPER_EL0
static uint64_t zPMXEVTYPER_EL0;

// register REVIDR_EL1
static uint64_t zREVIDR_EL1;

// register RMR_EL1
static uint64_t zRMR_EL1;

// register RMR_EL2
static uint64_t zRMR_EL2;

// register RMR_EL3
static uint64_t zRMR_EL3;

// register RSP_EL0
static lbits zRSP_EL0;

// register RTPIDR_EL0
static lbits zRTPIDR_EL0;

// register RVBAR_EL1
static uint64_t zRVBAR_EL1;

// register RVBAR_EL2
static uint64_t zRVBAR_EL2;

// register RVBAR_EL3
static uint64_t zRVBAR_EL3;

// register S3_op1_Cn_Cm_op2
static uint64_t zS3_op1_Cn_Cm_op2;

// register CID_EL0
static lbits zCID_EL0;

// register SCXTNUM_EL1
static uint64_t zSCXTNUM_EL1;

// register SCXTNUM_EL2
static uint64_t zSCXTNUM_EL2;

// register SCXTNUM_EL3
static uint64_t zSCXTNUM_EL3;

// register SDER32_EL3
static uint64_t zSDER32_EL3;

// register SPSR_abt
static uint64_t zSPSR_abt;

// register SPSR_fiq
static uint64_t zSPSR_fiq;

// register SPSR_irq
static uint64_t zSPSR_irq;

// register SPSR_und
static uint64_t zSPSR_und;

// register SP_EL0
static lbits zSP_EL0;

// register SP_EL1
static lbits zSP_EL1;

// register SP_EL2
static lbits zSP_EL2;

// register TPIDRRO_EL0
static lbits zTPIDRRO_EL0;

// register TPIDR_EL0
static lbits zTPIDR_EL0;

// register TPIDR_EL1
static lbits zTPIDR_EL1;

// register TPIDR_EL2
static lbits zTPIDR_EL2;

// register TPIDR_EL3
static lbits zTPIDR_EL3;

// register TTBR0_EL1
static uint64_t zTTBR0_EL1;

// register TTBR0_EL2
static uint64_t zTTBR0_EL2;

// register TTBR0_EL3
static uint64_t zTTBR0_EL3;

// register TTBR1_EL1
static uint64_t zTTBR1_EL1;

// register TTBR1_EL2
static uint64_t zTTBR1_EL2;

// register VSESR_EL2
static uint64_t zVSESR_EL2;

// register VTCR_EL2
static uint64_t zVTCR_EL2;

// register VTTBR_EL2
static uint64_t zVTTBR_EL2;

// register DDC_EL0
static lbits zDDC_EL0;

// register DDC_EL1
static lbits zDDC_EL1;

// register DDC_EL2
static lbits zDDC_EL2;

// register DDC_EL3
static lbits zDDC_EL3;

// register RDDC_EL0
static lbits zRDDC_EL0;

// register DBGDTRTX_EL0
static uint64_t zDBGDTRTX_EL0;

// register ICC_ASGI1R_EL1
static uint64_t zICC_ASGI1R_EL1;

// register ICC_DIR_EL1
static uint64_t zICC_DIR_EL1;

// register ICV_DIR_EL1
static uint64_t zICV_DIR_EL1;

// register ICC_EOIR0_EL1
static uint64_t zICC_EOIR0_EL1;

// register ICV_EOIR0_EL1
static uint64_t zICV_EOIR0_EL1;

// register ICC_EOIR1_EL1
static uint64_t zICC_EOIR1_EL1;

// register ICV_EOIR1_EL1
static uint64_t zICV_EOIR1_EL1;

// register ICC_SGI0R_EL1
static uint64_t zICC_SGI0R_EL1;

// register ICC_SGI1R_EL1
static uint64_t zICC_SGI1R_EL1;

// register OSLAR_EL1
static uint64_t zOSLAR_EL1;

// register PMSWINC_EL0
static uint64_t zPMSWINC_EL0;

static uint64_t zMemAttr_NC;
static void create_letbind_35(void) {


  uint64_t zgsz393;
  zgsz393 = UINT64_C(0b00);
  zMemAttr_NC = zgsz393;

let_end_187: ;
}
static void kill_letbind_35(void) {
}

static uint64_t zMemAttr_WT;
static void create_letbind_36(void) {


  uint64_t zgsz394;
  zgsz394 = UINT64_C(0b10);
  zMemAttr_WT = zgsz394;

let_end_188: ;
}
static void kill_letbind_36(void) {
}

static uint64_t zMemAttr_WB;
static void create_letbind_37(void) {


  uint64_t zgsz395;
  zgsz395 = UINT64_C(0b11);
  zMemAttr_WB = zgsz395;

let_end_189: ;
}
static void kill_letbind_37(void) {
}

static uint64_t zMemHint_No;
static void create_letbind_38(void) {


  uint64_t zgsz396;
  zgsz396 = UINT64_C(0b00);
  zMemHint_No = zgsz396;

let_end_190: ;
}
static void kill_letbind_38(void) {
}

static uint64_t zMemHint_RA;
static void create_letbind_39(void) {


  uint64_t zgsz397;
  zgsz397 = UINT64_C(0b10);
  zMemHint_RA = zgsz397;

let_end_191: ;
}
static void kill_letbind_39(void) {
}

static uint64_t zMemHint_RWA;
static void create_letbind_40(void) {


  uint64_t zgsz398;
  zgsz398 = UINT64_C(0b11);
  zMemHint_RWA = zgsz398;

let_end_192: ;
}
static void kill_letbind_40(void) {
}

// register EventRegister
static uint64_t zEventRegister;

// register _V
static zz5vecz8z5bvz9 z_V;

static uint64_t zDefaultPARTID;
static void create_letbind_41(void) {


  uint64_t zgsz399;
  {
    sail_int zgsz3840;
    CREATE(sail_int)(&zgsz3840);
    CONVERT_OF(sail_int, mach_int)(&zgsz3840, INT64_C(0));
    sail_int zgsz3841;
    CREATE(sail_int)(&zgsz3841);
    CONVERT_OF(sail_int, mach_int)(&zgsz3841, INT64_C(15));
    sail_int zgsz3842;
    CREATE(sail_int)(&zgsz3842);
    CONVERT_OF(sail_int, mach_int)(&zgsz3842, INT64_C(0));
    lbits zgsz3843;
    CREATE(lbits)(&zgsz3843);
    sailgen_integer_subrange(&zgsz3843, zgsz3840, zgsz3841, zgsz3842);
    zgsz399 = CONVERT_OF(fbits, lbits)(zgsz3843, true);
    KILL(lbits)(&zgsz3843);
    KILL(sail_int)(&zgsz3842);
    KILL(sail_int)(&zgsz3841);
    KILL(sail_int)(&zgsz3840);
  }
  zDefaultPARTID = zgsz399;

let_end_193: ;
}
static void kill_letbind_41(void) {
}

static uint64_t zDefaultPMG;
static void create_letbind_42(void) {


  uint64_t zgsz3100;
  {
    sail_int zgsz3844;
    CREATE(sail_int)(&zgsz3844);
    CONVERT_OF(sail_int, mach_int)(&zgsz3844, INT64_C(0));
    sail_int zgsz3845;
    CREATE(sail_int)(&zgsz3845);
    CONVERT_OF(sail_int, mach_int)(&zgsz3845, INT64_C(7));
    sail_int zgsz3846;
    CREATE(sail_int)(&zgsz3846);
    CONVERT_OF(sail_int, mach_int)(&zgsz3846, INT64_C(0));
    lbits zgsz3847;
    CREATE(lbits)(&zgsz3847);
    sailgen_integer_subrange(&zgsz3847, zgsz3844, zgsz3845, zgsz3846);
    zgsz3100 = CONVERT_OF(fbits, lbits)(zgsz3847, true);
    KILL(lbits)(&zgsz3847);
    KILL(sail_int)(&zgsz3846);
    KILL(sail_int)(&zgsz3845);
    KILL(sail_int)(&zgsz3844);
  }
  zDefaultPMG = zgsz3100;

let_end_194: ;
}
static void kill_letbind_42(void) {
}

static int64_t zCAPABILITY_DBYTES;
static void create_letbind_43(void) {


  int64_t zgsz3101;
  zgsz3101 = INT64_C(16);
  zCAPABILITY_DBYTES = zgsz3101;

let_end_195: ;
}
static void kill_letbind_43(void) {
}

static int64_t zLOG2_CAPABILITY_DBYTES;
static void create_letbind_44(void) {


  int64_t zgsz3102;
  zgsz3102 = INT64_C(4);
  zLOG2_CAPABILITY_DBYTES = zgsz3102;

let_end_196: ;
}
static void kill_letbind_44(void) {
}

static uint64_t zDebugHalt_Breakpoint;
static void create_letbind_45(void) {


  uint64_t zgsz3103;
  zgsz3103 = UINT64_C(0b000111);
  zDebugHalt_Breakpoint = zgsz3103;

let_end_197: ;
}
static void kill_letbind_45(void) {
}

static uint64_t zDebugHalt_Watchpoint;
static void create_letbind_46(void) {


  uint64_t zgsz3104;
  zgsz3104 = UINT64_C(0b101011);
  zDebugHalt_Watchpoint = zgsz3104;

let_end_198: ;
}
static void kill_letbind_46(void) {
}

static uint64_t zDebugHalt_HaltInstruction;
static void create_letbind_47(void) {


  uint64_t zgsz3105;
  zgsz3105 = UINT64_C(0b101111);
  zDebugHalt_HaltInstruction = zgsz3105;

let_end_199: ;
}
static void kill_letbind_47(void) {
}

static uint64_t zCAP_NO_SEALING;
static void create_letbind_48(void) {


  uint64_t zgsz3106;
  {
    sail_int zgsz3848;
    CREATE(sail_int)(&zgsz3848);
    CONVERT_OF(sail_int, mach_int)(&zgsz3848, INT64_C(64));
    lbits zgsz3849;
    CREATE(lbits)(&zgsz3849);
    sailgen_Ones(&zgsz3849, zgsz3848);
    zgsz3106 = CONVERT_OF(fbits, lbits)(zgsz3849, true);
    KILL(lbits)(&zgsz3849);
    KILL(sail_int)(&zgsz3848);
  }
  zCAP_NO_SEALING = zgsz3106;

let_end_200: ;
}
static void kill_letbind_48(void) {
}

static uint64_t zCAP_SEAL_TYPE_RB;
static void create_letbind_49(void) {


  uint64_t zgsz3107;
  {
    lbits zgsz3850;
    CREATE(lbits)(&zgsz3850);
    CONVERT_OF(lbits, fbits)(&zgsz3850, UINT64_C(0b01), UINT64_C(2) , true);
    sail_int zgsz3851;
    CREATE(sail_int)(&zgsz3851);
    CONVERT_OF(sail_int, mach_int)(&zgsz3851, INT64_C(64));
    lbits zgsz3852;
    CREATE(lbits)(&zgsz3852);
    zero_extend(&zgsz3852, zgsz3850, zgsz3851);
    zgsz3107 = CONVERT_OF(fbits, lbits)(zgsz3852, true);
    KILL(lbits)(&zgsz3852);
    KILL(sail_int)(&zgsz3851);
    KILL(lbits)(&zgsz3850);
  }
  zCAP_SEAL_TYPE_RB = zgsz3107;

let_end_201: ;
}
static void kill_letbind_49(void) {
}

static uint64_t zCAP_SEAL_TYPE_LPB;
static void create_letbind_50(void) {


  uint64_t zgsz3108;
  {
    lbits zgsz3853;
    CREATE(lbits)(&zgsz3853);
    CONVERT_OF(lbits, fbits)(&zgsz3853, UINT64_C(0b10), UINT64_C(2) , true);
    sail_int zgsz3854;
    CREATE(sail_int)(&zgsz3854);
    CONVERT_OF(sail_int, mach_int)(&zgsz3854, INT64_C(64));
    lbits zgsz3855;
    CREATE(lbits)(&zgsz3855);
    zero_extend(&zgsz3855, zgsz3853, zgsz3854);
    zgsz3108 = CONVERT_OF(fbits, lbits)(zgsz3855, true);
    KILL(lbits)(&zgsz3855);
    KILL(sail_int)(&zgsz3854);
    KILL(lbits)(&zgsz3853);
  }
  zCAP_SEAL_TYPE_LPB = zgsz3108;

let_end_202: ;
}
static void kill_letbind_50(void) {
}

static uint64_t zCAP_SEAL_TYPE_LB;
static void create_letbind_51(void) {


  uint64_t zgsz3109;
  {
    lbits zgsz3856;
    CREATE(lbits)(&zgsz3856);
    CONVERT_OF(lbits, fbits)(&zgsz3856, UINT64_C(0b11), UINT64_C(2) , true);
    sail_int zgsz3857;
    CREATE(sail_int)(&zgsz3857);
    CONVERT_OF(sail_int, mach_int)(&zgsz3857, INT64_C(64));
    lbits zgsz3858;
    CREATE(lbits)(&zgsz3858);
    zero_extend(&zgsz3858, zgsz3856, zgsz3857);
    zgsz3109 = CONVERT_OF(fbits, lbits)(zgsz3858, true);
    KILL(lbits)(&zgsz3858);
    KILL(sail_int)(&zgsz3857);
    KILL(lbits)(&zgsz3856);
  }
  zCAP_SEAL_TYPE_LB = zgsz3109;

let_end_203: ;
}
static void kill_letbind_51(void) {
}

static int64_t zCAP_MAX_FIXED_SEAL_TYPE;
static void create_letbind_52(void) {


  int64_t zgsz3110;
  zgsz3110 = INT64_C(3);
  zCAP_MAX_FIXED_SEAL_TYPE = zgsz3110;

let_end_204: ;
}
static void kill_letbind_52(void) {
}

static uint64_t zCAP_PERM_LOAD;
static void create_letbind_53(void) {


  uint64_t zgsz3111;
  sail_int zgaz3166;
  CREATE(sail_int)(&zgaz3166);
  {
    sail_int zgsz3862;
    CREATE(sail_int)(&zgsz3862);
    CONVERT_OF(sail_int, mach_int)(&zgsz3862, INT64_C(1));
    sail_int zgsz3863;
    CREATE(sail_int)(&zgsz3863);
    CONVERT_OF(sail_int, mach_int)(&zgsz3863, INT64_C(17));
    shl_int(&zgaz3166, zgsz3862, zgsz3863);
    KILL(sail_int)(&zgsz3863);
    KILL(sail_int)(&zgsz3862);
  }
  {
    sail_int zgsz3859;
    CREATE(sail_int)(&zgsz3859);
    CONVERT_OF(sail_int, mach_int)(&zgsz3859, INT64_C(63));
    sail_int zgsz3860;
    CREATE(sail_int)(&zgsz3860);
    CONVERT_OF(sail_int, mach_int)(&zgsz3860, INT64_C(0));
    lbits zgsz3861;
    CREATE(lbits)(&zgsz3861);
    sailgen_integer_subrange(&zgsz3861, zgaz3166, zgsz3859, zgsz3860);
    zgsz3111 = CONVERT_OF(fbits, lbits)(zgsz3861, true);
    KILL(lbits)(&zgsz3861);
    KILL(sail_int)(&zgsz3860);
    KILL(sail_int)(&zgsz3859);
  }
  KILL(sail_int)(&zgaz3166);
  zCAP_PERM_LOAD = zgsz3111;

let_end_205: ;
}
static void kill_letbind_53(void) {
}

static uint64_t zCAP_PERM_STORE;
static void create_letbind_54(void) {


  uint64_t zgsz3112;
  sail_int zgaz3167;
  CREATE(sail_int)(&zgaz3167);
  {
    sail_int zgsz3867;
    CREATE(sail_int)(&zgsz3867);
    CONVERT_OF(sail_int, mach_int)(&zgsz3867, INT64_C(1));
    sail_int zgsz3868;
    CREATE(sail_int)(&zgsz3868);
    CONVERT_OF(sail_int, mach_int)(&zgsz3868, INT64_C(16));
    shl_int(&zgaz3167, zgsz3867, zgsz3868);
    KILL(sail_int)(&zgsz3868);
    KILL(sail_int)(&zgsz3867);
  }
  {
    sail_int zgsz3864;
    CREATE(sail_int)(&zgsz3864);
    CONVERT_OF(sail_int, mach_int)(&zgsz3864, INT64_C(63));
    sail_int zgsz3865;
    CREATE(sail_int)(&zgsz3865);
    CONVERT_OF(sail_int, mach_int)(&zgsz3865, INT64_C(0));
    lbits zgsz3866;
    CREATE(lbits)(&zgsz3866);
    sailgen_integer_subrange(&zgsz3866, zgaz3167, zgsz3864, zgsz3865);
    zgsz3112 = CONVERT_OF(fbits, lbits)(zgsz3866, true);
    KILL(lbits)(&zgsz3866);
    KILL(sail_int)(&zgsz3865);
    KILL(sail_int)(&zgsz3864);
  }
  KILL(sail_int)(&zgaz3167);
  zCAP_PERM_STORE = zgsz3112;

let_end_206: ;
}
static void kill_letbind_54(void) {
}

static uint64_t zCAP_PERM_LOAD_CAP;
static void create_letbind_55(void) {


  uint64_t zgsz3113;
  sail_int zgaz3168;
  CREATE(sail_int)(&zgaz3168);
  {
    sail_int zgsz3872;
    CREATE(sail_int)(&zgsz3872);
    CONVERT_OF(sail_int, mach_int)(&zgsz3872, INT64_C(1));
    sail_int zgsz3873;
    CREATE(sail_int)(&zgsz3873);
    CONVERT_OF(sail_int, mach_int)(&zgsz3873, INT64_C(14));
    shl_int(&zgaz3168, zgsz3872, zgsz3873);
    KILL(sail_int)(&zgsz3873);
    KILL(sail_int)(&zgsz3872);
  }
  {
    sail_int zgsz3869;
    CREATE(sail_int)(&zgsz3869);
    CONVERT_OF(sail_int, mach_int)(&zgsz3869, INT64_C(63));
    sail_int zgsz3870;
    CREATE(sail_int)(&zgsz3870);
    CONVERT_OF(sail_int, mach_int)(&zgsz3870, INT64_C(0));
    lbits zgsz3871;
    CREATE(lbits)(&zgsz3871);
    sailgen_integer_subrange(&zgsz3871, zgaz3168, zgsz3869, zgsz3870);
    zgsz3113 = CONVERT_OF(fbits, lbits)(zgsz3871, true);
    KILL(lbits)(&zgsz3871);
    KILL(sail_int)(&zgsz3870);
    KILL(sail_int)(&zgsz3869);
  }
  KILL(sail_int)(&zgaz3168);
  zCAP_PERM_LOAD_CAP = zgsz3113;

let_end_207: ;
}
static void kill_letbind_55(void) {
}

static uint64_t zCAP_PERM_STORE_CAP;
static void create_letbind_56(void) {


  uint64_t zgsz3114;
  sail_int zgaz3169;
  CREATE(sail_int)(&zgaz3169);
  {
    sail_int zgsz3877;
    CREATE(sail_int)(&zgsz3877);
    CONVERT_OF(sail_int, mach_int)(&zgsz3877, INT64_C(1));
    sail_int zgsz3878;
    CREATE(sail_int)(&zgsz3878);
    CONVERT_OF(sail_int, mach_int)(&zgsz3878, INT64_C(13));
    shl_int(&zgaz3169, zgsz3877, zgsz3878);
    KILL(sail_int)(&zgsz3878);
    KILL(sail_int)(&zgsz3877);
  }
  {
    sail_int zgsz3874;
    CREATE(sail_int)(&zgsz3874);
    CONVERT_OF(sail_int, mach_int)(&zgsz3874, INT64_C(63));
    sail_int zgsz3875;
    CREATE(sail_int)(&zgsz3875);
    CONVERT_OF(sail_int, mach_int)(&zgsz3875, INT64_C(0));
    lbits zgsz3876;
    CREATE(lbits)(&zgsz3876);
    sailgen_integer_subrange(&zgsz3876, zgaz3169, zgsz3874, zgsz3875);
    zgsz3114 = CONVERT_OF(fbits, lbits)(zgsz3876, true);
    KILL(lbits)(&zgsz3876);
    KILL(sail_int)(&zgsz3875);
    KILL(sail_int)(&zgsz3874);
  }
  KILL(sail_int)(&zgaz3169);
  zCAP_PERM_STORE_CAP = zgsz3114;

let_end_208: ;
}
static void kill_letbind_56(void) {
}

static uint64_t zCAP_PERM_STORE_LOCAL;
static void create_letbind_57(void) {


  uint64_t zgsz3115;
  sail_int zgaz3170;
  CREATE(sail_int)(&zgaz3170);
  {
    sail_int zgsz3882;
    CREATE(sail_int)(&zgsz3882);
    CONVERT_OF(sail_int, mach_int)(&zgsz3882, INT64_C(1));
    sail_int zgsz3883;
    CREATE(sail_int)(&zgsz3883);
    CONVERT_OF(sail_int, mach_int)(&zgsz3883, INT64_C(12));
    shl_int(&zgaz3170, zgsz3882, zgsz3883);
    KILL(sail_int)(&zgsz3883);
    KILL(sail_int)(&zgsz3882);
  }
  {
    sail_int zgsz3879;
    CREATE(sail_int)(&zgsz3879);
    CONVERT_OF(sail_int, mach_int)(&zgsz3879, INT64_C(63));
    sail_int zgsz3880;
    CREATE(sail_int)(&zgsz3880);
    CONVERT_OF(sail_int, mach_int)(&zgsz3880, INT64_C(0));
    lbits zgsz3881;
    CREATE(lbits)(&zgsz3881);
    sailgen_integer_subrange(&zgsz3881, zgaz3170, zgsz3879, zgsz3880);
    zgsz3115 = CONVERT_OF(fbits, lbits)(zgsz3881, true);
    KILL(lbits)(&zgsz3881);
    KILL(sail_int)(&zgsz3880);
    KILL(sail_int)(&zgsz3879);
  }
  KILL(sail_int)(&zgaz3170);
  zCAP_PERM_STORE_LOCAL = zgsz3115;

let_end_209: ;
}
static void kill_letbind_57(void) {
}

static uint64_t zCAP_PERM_SEAL;
static void create_letbind_58(void) {


  uint64_t zgsz3116;
  sail_int zgaz3171;
  CREATE(sail_int)(&zgaz3171);
  {
    sail_int zgsz3887;
    CREATE(sail_int)(&zgsz3887);
    CONVERT_OF(sail_int, mach_int)(&zgsz3887, INT64_C(1));
    sail_int zgsz3888;
    CREATE(sail_int)(&zgsz3888);
    CONVERT_OF(sail_int, mach_int)(&zgsz3888, INT64_C(11));
    shl_int(&zgaz3171, zgsz3887, zgsz3888);
    KILL(sail_int)(&zgsz3888);
    KILL(sail_int)(&zgsz3887);
  }
  {
    sail_int zgsz3884;
    CREATE(sail_int)(&zgsz3884);
    CONVERT_OF(sail_int, mach_int)(&zgsz3884, INT64_C(63));
    sail_int zgsz3885;
    CREATE(sail_int)(&zgsz3885);
    CONVERT_OF(sail_int, mach_int)(&zgsz3885, INT64_C(0));
    lbits zgsz3886;
    CREATE(lbits)(&zgsz3886);
    sailgen_integer_subrange(&zgsz3886, zgaz3171, zgsz3884, zgsz3885);
    zgsz3116 = CONVERT_OF(fbits, lbits)(zgsz3886, true);
    KILL(lbits)(&zgsz3886);
    KILL(sail_int)(&zgsz3885);
    KILL(sail_int)(&zgsz3884);
  }
  KILL(sail_int)(&zgaz3171);
  zCAP_PERM_SEAL = zgsz3116;

let_end_210: ;
}
static void kill_letbind_58(void) {
}

static uint64_t zCAP_PERM_UNSEAL;
static void create_letbind_59(void) {


  uint64_t zgsz3117;
  sail_int zgaz3172;
  CREATE(sail_int)(&zgaz3172);
  {
    sail_int zgsz3892;
    CREATE(sail_int)(&zgsz3892);
    CONVERT_OF(sail_int, mach_int)(&zgsz3892, INT64_C(1));
    sail_int zgsz3893;
    CREATE(sail_int)(&zgsz3893);
    CONVERT_OF(sail_int, mach_int)(&zgsz3893, INT64_C(10));
    shl_int(&zgaz3172, zgsz3892, zgsz3893);
    KILL(sail_int)(&zgsz3893);
    KILL(sail_int)(&zgsz3892);
  }
  {
    sail_int zgsz3889;
    CREATE(sail_int)(&zgsz3889);
    CONVERT_OF(sail_int, mach_int)(&zgsz3889, INT64_C(63));
    sail_int zgsz3890;
    CREATE(sail_int)(&zgsz3890);
    CONVERT_OF(sail_int, mach_int)(&zgsz3890, INT64_C(0));
    lbits zgsz3891;
    CREATE(lbits)(&zgsz3891);
    sailgen_integer_subrange(&zgsz3891, zgaz3172, zgsz3889, zgsz3890);
    zgsz3117 = CONVERT_OF(fbits, lbits)(zgsz3891, true);
    KILL(lbits)(&zgsz3891);
    KILL(sail_int)(&zgsz3890);
    KILL(sail_int)(&zgsz3889);
  }
  KILL(sail_int)(&zgaz3172);
  zCAP_PERM_UNSEAL = zgsz3117;

let_end_211: ;
}
static void kill_letbind_59(void) {
}

static uint64_t zCAP_PERM_BRANCH_SEALED_PAIR;
static void create_letbind_60(void) {


  uint64_t zgsz3118;
  sail_int zgaz3173;
  CREATE(sail_int)(&zgaz3173);
  {
    sail_int zgsz3897;
    CREATE(sail_int)(&zgsz3897);
    CONVERT_OF(sail_int, mach_int)(&zgsz3897, INT64_C(1));
    sail_int zgsz3898;
    CREATE(sail_int)(&zgsz3898);
    CONVERT_OF(sail_int, mach_int)(&zgsz3898, INT64_C(8));
    shl_int(&zgaz3173, zgsz3897, zgsz3898);
    KILL(sail_int)(&zgsz3898);
    KILL(sail_int)(&zgsz3897);
  }
  {
    sail_int zgsz3894;
    CREATE(sail_int)(&zgsz3894);
    CONVERT_OF(sail_int, mach_int)(&zgsz3894, INT64_C(63));
    sail_int zgsz3895;
    CREATE(sail_int)(&zgsz3895);
    CONVERT_OF(sail_int, mach_int)(&zgsz3895, INT64_C(0));
    lbits zgsz3896;
    CREATE(lbits)(&zgsz3896);
    sailgen_integer_subrange(&zgsz3896, zgaz3173, zgsz3894, zgsz3895);
    zgsz3118 = CONVERT_OF(fbits, lbits)(zgsz3896, true);
    KILL(lbits)(&zgsz3896);
    KILL(sail_int)(&zgsz3895);
    KILL(sail_int)(&zgsz3894);
  }
  KILL(sail_int)(&zgaz3173);
  zCAP_PERM_BRANCH_SEALED_PAIR = zgsz3118;

let_end_212: ;
}
static void kill_letbind_60(void) {
}

static uint64_t zCAP_PERM_MUTABLE_LOAD;
static void create_letbind_61(void) {


  uint64_t zgsz3119;
  sail_int zgaz3174;
  CREATE(sail_int)(&zgaz3174);
  {
    sail_int zgsz3902;
    CREATE(sail_int)(&zgsz3902);
    CONVERT_OF(sail_int, mach_int)(&zgsz3902, INT64_C(1));
    sail_int zgsz3903;
    CREATE(sail_int)(&zgsz3903);
    CONVERT_OF(sail_int, mach_int)(&zgsz3903, INT64_C(6));
    shl_int(&zgaz3174, zgsz3902, zgsz3903);
    KILL(sail_int)(&zgsz3903);
    KILL(sail_int)(&zgsz3902);
  }
  {
    sail_int zgsz3899;
    CREATE(sail_int)(&zgsz3899);
    CONVERT_OF(sail_int, mach_int)(&zgsz3899, INT64_C(63));
    sail_int zgsz3900;
    CREATE(sail_int)(&zgsz3900);
    CONVERT_OF(sail_int, mach_int)(&zgsz3900, INT64_C(0));
    lbits zgsz3901;
    CREATE(lbits)(&zgsz3901);
    sailgen_integer_subrange(&zgsz3901, zgaz3174, zgsz3899, zgsz3900);
    zgsz3119 = CONVERT_OF(fbits, lbits)(zgsz3901, true);
    KILL(lbits)(&zgsz3901);
    KILL(sail_int)(&zgsz3900);
    KILL(sail_int)(&zgsz3899);
  }
  KILL(sail_int)(&zgaz3174);
  zCAP_PERM_MUTABLE_LOAD = zgsz3119;

let_end_213: ;
}
static void kill_letbind_61(void) {
}

static uint64_t zCAP_PERM_GLOBAL;
static void create_letbind_62(void) {


  uint64_t zgsz3120;
  {
    sail_int zgsz3904;
    CREATE(sail_int)(&zgsz3904);
    CONVERT_OF(sail_int, mach_int)(&zgsz3904, INT64_C(1));
    sail_int zgsz3905;
    CREATE(sail_int)(&zgsz3905);
    CONVERT_OF(sail_int, mach_int)(&zgsz3905, INT64_C(63));
    sail_int zgsz3906;
    CREATE(sail_int)(&zgsz3906);
    CONVERT_OF(sail_int, mach_int)(&zgsz3906, INT64_C(0));
    lbits zgsz3907;
    CREATE(lbits)(&zgsz3907);
    sailgen_integer_subrange(&zgsz3907, zgsz3904, zgsz3905, zgsz3906);
    zgsz3120 = CONVERT_OF(fbits, lbits)(zgsz3907, true);
    KILL(lbits)(&zgsz3907);
    KILL(sail_int)(&zgsz3906);
    KILL(sail_int)(&zgsz3905);
    KILL(sail_int)(&zgsz3904);
  }
  zCAP_PERM_GLOBAL = zgsz3120;

let_end_214: ;
}
static void kill_letbind_62(void) {
}

static uint64_t zCAP_PERM_NONE;
static void create_letbind_63(void) {


  uint64_t zgsz3121;
  {
    sail_int zgsz3908;
    CREATE(sail_int)(&zgsz3908);
    CONVERT_OF(sail_int, mach_int)(&zgsz3908, INT64_C(0));
    sail_int zgsz3909;
    CREATE(sail_int)(&zgsz3909);
    CONVERT_OF(sail_int, mach_int)(&zgsz3909, INT64_C(63));
    sail_int zgsz3910;
    CREATE(sail_int)(&zgsz3910);
    CONVERT_OF(sail_int, mach_int)(&zgsz3910, INT64_C(0));
    lbits zgsz3911;
    CREATE(lbits)(&zgsz3911);
    sailgen_integer_subrange(&zgsz3911, zgsz3908, zgsz3909, zgsz3910);
    zgsz3121 = CONVERT_OF(fbits, lbits)(zgsz3911, true);
    KILL(lbits)(&zgsz3911);
    KILL(sail_int)(&zgsz3910);
    KILL(sail_int)(&zgsz3909);
    KILL(sail_int)(&zgsz3908);
  }
  zCAP_PERM_NONE = zgsz3121;

let_end_215: ;
}
static void kill_letbind_63(void) {
}

static int64_t zCAP_OTYPE_NUM_BITS;
static void create_letbind_64(void) {


  int64_t zgsz3122;
  int64_t zgaz3175;
  {
    sail_int zgsz3912;
    CREATE(sail_int)(&zgsz3912);
    CONVERT_OF(sail_int, mach_int)(&zgsz3912, zCAP_OTYPE_HI_BIT);
    sail_int zgsz3913;
    CREATE(sail_int)(&zgsz3913);
    CONVERT_OF(sail_int, mach_int)(&zgsz3913, zCAP_OTYPE_LO_BIT);
    sail_int zgsz3914;
    CREATE(sail_int)(&zgsz3914);
    sub_int(&zgsz3914, zgsz3912, zgsz3913);
    zgaz3175 = CONVERT_OF(mach_int, sail_int)(zgsz3914);
    KILL(sail_int)(&zgsz3914);
    KILL(sail_int)(&zgsz3913);
    KILL(sail_int)(&zgsz3912);
  }
  zgsz3122 = (zgaz3175 + INT64_C(1));

  zCAP_OTYPE_NUM_BITS = zgsz3122;

let_end_216: ;
}
static void kill_letbind_64(void) {
}

static int64_t zCAP_LENGTH_NUM_BITS;
static void create_letbind_65(void) {


  int64_t zgsz3123;
  zgsz3123 = (zCAP_VALUE_NUM_BITS + INT64_C(1));
  zCAP_LENGTH_NUM_BITS = zgsz3123;

let_end_217: ;
}
static void kill_letbind_65(void) {
}

static sail_int zCAP_MAX_OBJECT_TYPE;
static void create_letbind_66(void) {    CREATE(sail_int)(&zCAP_MAX_OBJECT_TYPE);


  sail_int zgsz3124;
  CREATE(sail_int)(&zgsz3124);
  sail_int zgaz3176;
  CREATE(sail_int)(&zgaz3176);
  {
    sail_int zgsz3916;
    CREATE(sail_int)(&zgsz3916);
    CONVERT_OF(sail_int, mach_int)(&zgsz3916, INT64_C(1));
    sail_int zgsz3917;
    CREATE(sail_int)(&zgsz3917);
    CONVERT_OF(sail_int, mach_int)(&zgsz3917, zCAP_OTYPE_NUM_BITS);
    shl_int(&zgaz3176, zgsz3916, zgsz3917);
    KILL(sail_int)(&zgsz3917);
    KILL(sail_int)(&zgsz3916);
  }
  {
    sail_int zgsz3915;
    CREATE(sail_int)(&zgsz3915);
    CONVERT_OF(sail_int, mach_int)(&zgsz3915, INT64_C(1));
    sub_int(&zgsz3124, zgaz3176, zgsz3915);
    KILL(sail_int)(&zgsz3915);
  }
  KILL(sail_int)(&zgaz3176);
  COPY(sail_int)(&zCAP_MAX_OBJECT_TYPE, zgsz3124);
  KILL(sail_int)(&zgsz3124);
let_end_218: ;
}
static void kill_letbind_66(void) {    KILL(sail_int)(&zCAP_MAX_OBJECT_TYPE);
}

// register __saved_exception_level
static uint64_t z__saved_exception_level;

// register __CNTControlBase
static uint64_t z__CNTControlBase;

static enum z__InstrEnc sailgen_undefined___InstrEnc(unit);

static enum z__InstrEnc sailgen_undefined___InstrEnc(unit zgsz3125)
{
  __label__ end_function_220, end_block_exception_221;

  enum z__InstrEnc zcbz330;
  zcbz330 = z__A64;
end_function_220: ;
  return zcbz330;
end_block_exception_221: ;

  return z__A32;
}

// register __ThisInstr
static uint64_t z__ThisInstr;

// register __ThisInstrEnc
static enum z__InstrEnc z__ThisInstrEnc;














































static void sailgen_undefined_instr_ast(struct zinstr_ast *rop, unit);

sail_int zghz3274;
lbits zghz3275;
sail_int zghz3276;
lbits zghz3277;

static void startup_sailgen_undefined_instr_ast(void)
{
  CREATE(sail_int)(&zghz3274);
  CREATE(lbits)(&zghz3275);
  CREATE(sail_int)(&zghz3276);
  CREATE(lbits)(&zghz3277);
}















































static void sailgen_undefined_instr_ast(struct zinstr_ast *zcbz331, unit zgsz3126)
{
  __label__ end_function_223, end_block_exception_224, end_function_285;

  struct ztuple_z8z5bv2zCz0z5bv5z9 zgaz3179;
  {
    uint64_t zgaz3177;
    {
      RECREATE(sail_int)(&zghz3276);
      CONVERT_OF(sail_int, mach_int)(&zghz3276, INT64_C(2));
      RECREATE(lbits)(&zghz3277);
      UNDEFINED(lbits)(&zghz3277, zghz3276);
      zgaz3177 = CONVERT_OF(fbits, lbits)(zghz3277, true);
    }
    uint64_t zgaz3178;
    {
      RECREATE(sail_int)(&zghz3274);
      CONVERT_OF(sail_int, mach_int)(&zghz3274, INT64_C(5));
      RECREATE(lbits)(&zghz3275);
      UNDEFINED(lbits)(&zghz3275, zghz3274);
      zgaz3178 = CONVERT_OF(fbits, lbits)(zghz3275, true);
    }
    zgaz3179.ztup0 = zgaz3177;
    zgaz3179.ztup1 = zgaz3178;
  }
  sailgen_Instr_BLRR_C_C((*(&zcbz331)), zgaz3179);

end_function_223: ;
  goto end_function_285;
end_block_exception_224: ;
  goto end_function_285;
end_function_285: ;
}



static void finish_sailgen_undefined_instr_ast(void)
{
  KILL(lbits)(&zghz3277);
  KILL(sail_int)(&zghz3276);
  KILL(lbits)(&zghz3275);
  KILL(sail_int)(&zghz3274);
}














































// register __ThisInstrAbstract
static struct zinstr_ast z__ThisInstrAbstract;

// register __highest_el_aarch32
static bool z__highest_el_aarch32;

static uint64_t z__exclusive_granule_sizze;
static void create_letbind_67(void) {


  uint64_t zgsz3128;
  zgsz3128 = UINT64_C(0x4);
  z__exclusive_granule_sizze = zgsz3128;

let_end_225: ;
}
static void kill_letbind_67(void) {
}

static uint64_t zCFG_ID_AA64PFR0_EL1_MPAM;
static void create_letbind_68(void) {


  uint64_t zgsz3129;
  zgsz3129 = UINT64_C(0x1);
  zCFG_ID_AA64PFR0_EL1_MPAM = zgsz3129;

let_end_226: ;
}
static void kill_letbind_68(void) {
}

static bool sailgen_CapUnsignedGreaterThanOrEqual(lbits, lbits);

sail_int zghz3278;
sail_int zghz3279;

static void startup_sailgen_CapUnsignedGreaterThanOrEqual(void)
{
  CREATE(sail_int)(&zghz3278);
  CREATE(sail_int)(&zghz3279);
}

static bool sailgen_CapUnsignedGreaterThanOrEqual(lbits za, lbits zb)
{
  __label__ cleanup_229, end_cleanup_230, end_function_228, end_block_exception_231;

  bool zcbz332;
  {
    RECREATE(sail_int)(&zghz3278);
    sail_unsigned(&zghz3278, za);
    RECREATE(sail_int)(&zghz3279);
    sail_unsigned(&zghz3279, zb);
    zcbz332 = gteq(zghz3278, zghz3279);
  }
  goto cleanup_229;
  /* unreachable after return */
  goto end_cleanup_230;
cleanup_229: ;
  goto end_function_228;
end_cleanup_230: ;
end_function_228: ;
  return zcbz332;
end_block_exception_231: ;

  return false;
}



static void finish_sailgen_CapUnsignedGreaterThanOrEqual(void)
{
  KILL(sail_int)(&zghz3279);
  KILL(sail_int)(&zghz3278);
}

static bool sailgen_CapIsRepresentableFast(lbits, uint64_t);

lbits zghz3280;
lbits zghz3281;
sail_int zghz3282;
lbits zghz3283;
lbits zghz3284;
lbits zghz3285;
lbits zghz3286;
sail_int zghz3287;
lbits zghz3288;
sail_int zghz3289;
lbits zghz3290;
sail_int zghz3291;
lbits zghz3292;
sail_int zghz3293;
sail_int zghz3294;
sail_int zghz3295;
sail_int zghz3296;
sail_int zghz3297;
sail_int zghz3298;
sail_int zghz3299;
sail_int zghz3300;
sail_int zghz3301;
sail_int zghz3302;
sail_int zghz3303;
sail_int zghz3304;
lbits zghz3305;
sail_int zghz3306;
lbits zghz3307;
sail_int zghz3308;
sail_int zghz3309;
sail_int zghz3310;
lbits zghz3311;
sail_int zghz3312;
lbits zghz3313;
lbits zghz3314;
sail_int zghz3315;
lbits zghz3316;
sail_int zghz3317;
sail_int zghz3318;
sail_int zghz3319;
sail_int zghz3320;
lbits zghz3321;
sail_int zghz3322;
lbits zghz3323;
sail_int zghz3324;
sail_int zghz3325;
sail_int zghz3326;
sail_int zghz3327;
sail_int zghz3328;
sail_int zghz3329;
sail_int zghz3330;
sail_int zghz3331;
sail_int zghz3332;
sail_int zghz3333;
lbits zghz3334;
sail_int zghz3335;
lbits zghz3336;
sail_int zghz3337;
lbits zghz3338;
sail_int zghz3339;
sail_int zghz3340;
sail_int zghz3341;
sail_int zghz3342;
sail_int zghz3343;
sail_int zghz3344;
sail_int zghz3345;
sail_int zghz3346;
sail_int zghz3347;
sail_int zghz3348;
lbits zghz3349;
sail_int zghz3350;
lbits zghz3351;
sail_int zghz3352;
lbits zghz3353;
sail_int zghz3354;
sail_int zghz3355;
sail_int zghz3356;
sail_int zghz3357;
sail_int zghz3358;
sail_int zghz3359;
sail_int zghz3360;
sail_int zghz3361;
sail_int zghz3362;
sail_int zghz3363;
sail_int zghz3364;
sail_int zghz3365;
sail_int zghz3366;
sail_int zghz3367;
sail_int zghz3368;

static void startup_sailgen_CapIsRepresentableFast(void)
{
  CREATE(lbits)(&zghz3280);
  CREATE(lbits)(&zghz3281);
  CREATE(sail_int)(&zghz3282);
  CREATE(lbits)(&zghz3283);
  CREATE(lbits)(&zghz3284);
  CREATE(lbits)(&zghz3285);
  CREATE(lbits)(&zghz3286);
  CREATE(sail_int)(&zghz3287);
  CREATE(lbits)(&zghz3288);
  CREATE(sail_int)(&zghz3289);
  CREATE(lbits)(&zghz3290);
  CREATE(sail_int)(&zghz3291);
  CREATE(lbits)(&zghz3292);
  CREATE(sail_int)(&zghz3293);
  CREATE(sail_int)(&zghz3294);
  CREATE(sail_int)(&zghz3295);
  CREATE(sail_int)(&zghz3296);
  CREATE(sail_int)(&zghz3297);
  CREATE(sail_int)(&zghz3298);
  CREATE(sail_int)(&zghz3299);
  CREATE(sail_int)(&zghz3300);
  CREATE(sail_int)(&zghz3301);
  CREATE(sail_int)(&zghz3302);
  CREATE(sail_int)(&zghz3303);
  CREATE(sail_int)(&zghz3304);
  CREATE(lbits)(&zghz3305);
  CREATE(sail_int)(&zghz3306);
  CREATE(lbits)(&zghz3307);
  CREATE(sail_int)(&zghz3308);
  CREATE(sail_int)(&zghz3309);
  CREATE(sail_int)(&zghz3310);
  CREATE(lbits)(&zghz3311);
  CREATE(sail_int)(&zghz3312);
  CREATE(lbits)(&zghz3313);
  CREATE(lbits)(&zghz3314);
  CREATE(sail_int)(&zghz3315);
  CREATE(lbits)(&zghz3316);
  CREATE(sail_int)(&zghz3317);
  CREATE(sail_int)(&zghz3318);
  CREATE(sail_int)(&zghz3319);
  CREATE(sail_int)(&zghz3320);
  CREATE(lbits)(&zghz3321);
  CREATE(sail_int)(&zghz3322);
  CREATE(lbits)(&zghz3323);
  CREATE(sail_int)(&zghz3324);
  CREATE(sail_int)(&zghz3325);
  CREATE(sail_int)(&zghz3326);
  CREATE(sail_int)(&zghz3327);
  CREATE(sail_int)(&zghz3328);
  CREATE(sail_int)(&zghz3329);
  CREATE(sail_int)(&zghz3330);
  CREATE(sail_int)(&zghz3331);
  CREATE(sail_int)(&zghz3332);
  CREATE(sail_int)(&zghz3333);
  CREATE(lbits)(&zghz3334);
  CREATE(sail_int)(&zghz3335);
  CREATE(lbits)(&zghz3336);
  CREATE(sail_int)(&zghz3337);
  CREATE(lbits)(&zghz3338);
  CREATE(sail_int)(&zghz3339);
  CREATE(sail_int)(&zghz3340);
  CREATE(sail_int)(&zghz3341);
  CREATE(sail_int)(&zghz3342);
  CREATE(sail_int)(&zghz3343);
  CREATE(sail_int)(&zghz3344);
  CREATE(sail_int)(&zghz3345);
  CREATE(sail_int)(&zghz3346);
  CREATE(sail_int)(&zghz3347);
  CREATE(sail_int)(&zghz3348);
  CREATE(lbits)(&zghz3349);
  CREATE(sail_int)(&zghz3350);
  CREATE(lbits)(&zghz3351);
  CREATE(sail_int)(&zghz3352);
  CREATE(lbits)(&zghz3353);
  CREATE(sail_int)(&zghz3354);
  CREATE(sail_int)(&zghz3355);
  CREATE(sail_int)(&zghz3356);
  CREATE(sail_int)(&zghz3357);
  CREATE(sail_int)(&zghz3358);
  CREATE(sail_int)(&zghz3359);
  CREATE(sail_int)(&zghz3360);
  CREATE(sail_int)(&zghz3361);
  CREATE(sail_int)(&zghz3362);
  CREATE(sail_int)(&zghz3363);
  CREATE(sail_int)(&zghz3364);
  CREATE(sail_int)(&zghz3365);
  CREATE(sail_int)(&zghz3366);
  CREATE(sail_int)(&zghz3367);
  CREATE(sail_int)(&zghz3368);
}

static bool sailgen_CapIsRepresentableFast(lbits zc, uint64_t zincrement_name__arg)
{
  __label__ case_234, finish_match_233, end_function_235, end_block_exception_244;

  bool zcbz333;
  uint64_t zincrement_name;
  zincrement_name = zincrement_name__arg;
  uint64_t zB3;
  {
    int64_t zgaz3222;
    {
      int64_t zgaz3221;
      {
        int64_t zgaz3220;
        {
          int64_t zgaz3218;
          {
            int64_t zgaz3215;
            {
              int64_t zgaz3214;
              {
                RECREATE(sail_int)(&zghz3366);
                CONVERT_OF(sail_int, mach_int)(&zghz3366, INT64_C(79));
                RECREATE(sail_int)(&zghz3367);
                CONVERT_OF(sail_int, mach_int)(&zghz3367, INT64_C(64));
                RECREATE(sail_int)(&zghz3368);
                sub_int(&zghz3368, zghz3366, zghz3367);
                zgaz3214 = CONVERT_OF(mach_int, sail_int)(zghz3368);
              }
              zgaz3215 = (zgaz3214 + INT64_C(1));
            }
            {
              RECREATE(sail_int)(&zghz3363);
              CONVERT_OF(sail_int, mach_int)(&zghz3363, zgaz3215);
              RECREATE(sail_int)(&zghz3364);
              CONVERT_OF(sail_int, mach_int)(&zghz3364, INT64_C(1));
              RECREATE(sail_int)(&zghz3365);
              sub_int(&zghz3365, zghz3363, zghz3364);
              zgaz3218 = CONVERT_OF(mach_int, sail_int)(zghz3365);
            }
          }
          int64_t zgaz3219;
          {
            int64_t zgaz3217;
            {
              int64_t zgaz3216;
              {
                RECREATE(sail_int)(&zghz3360);
                CONVERT_OF(sail_int, mach_int)(&zghz3360, INT64_C(79));
                RECREATE(sail_int)(&zghz3361);
                CONVERT_OF(sail_int, mach_int)(&zghz3361, INT64_C(64));
                RECREATE(sail_int)(&zghz3362);
                sub_int(&zghz3362, zghz3360, zghz3361);
                zgaz3216 = CONVERT_OF(mach_int, sail_int)(zghz3362);
              }
              zgaz3217 = (zgaz3216 + INT64_C(1));
            }
            {
              RECREATE(sail_int)(&zghz3357);
              CONVERT_OF(sail_int, mach_int)(&zghz3357, zgaz3217);
              RECREATE(sail_int)(&zghz3358);
              CONVERT_OF(sail_int, mach_int)(&zghz3358, INT64_C(3));
              RECREATE(sail_int)(&zghz3359);
              sub_int(&zghz3359, zghz3357, zghz3358);
              zgaz3219 = CONVERT_OF(mach_int, sail_int)(zghz3359);
            }
          }
          {
            RECREATE(sail_int)(&zghz3354);
            CONVERT_OF(sail_int, mach_int)(&zghz3354, zgaz3218);
            RECREATE(sail_int)(&zghz3355);
            CONVERT_OF(sail_int, mach_int)(&zghz3355, zgaz3219);
            RECREATE(sail_int)(&zghz3356);
            sub_int(&zghz3356, zghz3354, zghz3355);
            zgaz3220 = CONVERT_OF(mach_int, sail_int)(zghz3356);
          }
        }
        zgaz3221 = (zgaz3220 + INT64_C(1));
      }
      zgaz3222 = (INT64_C(0) + zgaz3221);
    }
    {
      RECREATE(sail_int)(&zghz3352);
      CONVERT_OF(sail_int, mach_int)(&zghz3352, zgaz3222);
      RECREATE(lbits)(&zghz3353);
      UNDEFINED(lbits)(&zghz3353, zghz3352);
      zB3 = CONVERT_OF(fbits, lbits)(zghz3353, true);
    }
  }
  uint64_t zR;
  {
    RECREATE(sail_int)(&zghz3350);
    CONVERT_OF(sail_int, mach_int)(&zghz3350, INT64_C(16));
    RECREATE(lbits)(&zghz3351);
    UNDEFINED(lbits)(&zghz3351, zghz3350);
    zR = CONVERT_OF(fbits, lbits)(zghz3351, true);
  }
  uint64_t zR3;
  {
    RECREATE(sail_int)(&zghz3348);
    CONVERT_OF(sail_int, mach_int)(&zghz3348, INT64_C(3));
    RECREATE(lbits)(&zghz3349);
    UNDEFINED(lbits)(&zghz3349, zghz3348);
    zR3 = CONVERT_OF(fbits, lbits)(zghz3349, true);
  }
  uint64_t za_mid;
  {
    int64_t zgaz3213;
    {
      int64_t zgaz3212;
      {
        int64_t zgaz3211;
        {
          int64_t zgaz3210;
          {
            int64_t zgaz3209;
            {
              int64_t zgaz3208;
              {
                RECREATE(sail_int)(&zghz3345);
                CONVERT_OF(sail_int, mach_int)(&zghz3345, INT64_C(79));
                RECREATE(sail_int)(&zghz3346);
                CONVERT_OF(sail_int, mach_int)(&zghz3346, INT64_C(64));
                RECREATE(sail_int)(&zghz3347);
                sub_int(&zghz3347, zghz3345, zghz3346);
                zgaz3208 = CONVERT_OF(mach_int, sail_int)(zghz3347);
              }
              zgaz3209 = (zgaz3208 + INT64_C(1));
            }
            {
              RECREATE(sail_int)(&zghz3342);
              CONVERT_OF(sail_int, mach_int)(&zghz3342, zgaz3209);
              RECREATE(sail_int)(&zghz3343);
              CONVERT_OF(sail_int, mach_int)(&zghz3343, INT64_C(1));
              RECREATE(sail_int)(&zghz3344);
              sub_int(&zghz3344, zghz3342, zghz3343);
              zgaz3210 = CONVERT_OF(mach_int, sail_int)(zghz3344);
            }
          }
          {
            RECREATE(sail_int)(&zghz3339);
            CONVERT_OF(sail_int, mach_int)(&zghz3339, zgaz3210);
            RECREATE(sail_int)(&zghz3340);
            CONVERT_OF(sail_int, mach_int)(&zghz3340, INT64_C(0));
            RECREATE(sail_int)(&zghz3341);
            sub_int(&zghz3341, zghz3339, zghz3340);
            zgaz3211 = CONVERT_OF(mach_int, sail_int)(zghz3341);
          }
        }
        zgaz3212 = (zgaz3211 + INT64_C(1));
      }
      zgaz3213 = (INT64_C(0) + zgaz3212);
    }
    {
      RECREATE(sail_int)(&zghz3337);
      CONVERT_OF(sail_int, mach_int)(&zghz3337, zgaz3213);
      RECREATE(lbits)(&zghz3338);
      UNDEFINED(lbits)(&zghz3338, zghz3337);
      za_mid = CONVERT_OF(fbits, lbits)(zghz3338, true);
    }
  }
  uint64_t zdiff;
  {
    RECREATE(sail_int)(&zghz3335);
    CONVERT_OF(sail_int, mach_int)(&zghz3335, INT64_C(16));
    RECREATE(lbits)(&zghz3336);
    UNDEFINED(lbits)(&zghz3336, zghz3335);
    zdiff = CONVERT_OF(fbits, lbits)(zghz3336, true);
  }
  uint64_t zdiff1;
  {
    RECREATE(sail_int)(&zghz3333);
    CONVERT_OF(sail_int, mach_int)(&zghz3333, INT64_C(16));
    RECREATE(lbits)(&zghz3334);
    UNDEFINED(lbits)(&zghz3334, zghz3333);
    zdiff1 = CONVERT_OF(fbits, lbits)(zghz3334, true);
  }
  uint64_t zi_mid;
  {
    int64_t zgaz3207;
    {
      int64_t zgaz3206;
      {
        int64_t zgaz3205;
        {
          int64_t zgaz3204;
          {
            int64_t zgaz3203;
            {
              int64_t zgaz3202;
              {
                RECREATE(sail_int)(&zghz3330);
                CONVERT_OF(sail_int, mach_int)(&zghz3330, INT64_C(79));
                RECREATE(sail_int)(&zghz3331);
                CONVERT_OF(sail_int, mach_int)(&zghz3331, INT64_C(64));
                RECREATE(sail_int)(&zghz3332);
                sub_int(&zghz3332, zghz3330, zghz3331);
                zgaz3202 = CONVERT_OF(mach_int, sail_int)(zghz3332);
              }
              zgaz3203 = (zgaz3202 + INT64_C(1));
            }
            {
              RECREATE(sail_int)(&zghz3327);
              CONVERT_OF(sail_int, mach_int)(&zghz3327, zgaz3203);
              RECREATE(sail_int)(&zghz3328);
              CONVERT_OF(sail_int, mach_int)(&zghz3328, INT64_C(1));
              RECREATE(sail_int)(&zghz3329);
              sub_int(&zghz3329, zghz3327, zghz3328);
              zgaz3204 = CONVERT_OF(mach_int, sail_int)(zghz3329);
            }
          }
          {
            RECREATE(sail_int)(&zghz3324);
            CONVERT_OF(sail_int, mach_int)(&zghz3324, zgaz3204);
            RECREATE(sail_int)(&zghz3325);
            CONVERT_OF(sail_int, mach_int)(&zghz3325, INT64_C(0));
            RECREATE(sail_int)(&zghz3326);
            sub_int(&zghz3326, zghz3324, zghz3325);
            zgaz3205 = CONVERT_OF(mach_int, sail_int)(zghz3326);
          }
        }
        zgaz3206 = (zgaz3205 + INT64_C(1));
      }
      zgaz3207 = (INT64_C(0) + zgaz3206);
    }
    {
      RECREATE(sail_int)(&zghz3322);
      CONVERT_OF(sail_int, mach_int)(&zghz3322, zgaz3207);
      RECREATE(lbits)(&zghz3323);
      UNDEFINED(lbits)(&zghz3323, zghz3322);
      zi_mid = CONVERT_OF(fbits, lbits)(zghz3323, true);
    }
  }
  uint64_t zi_top;
  {
    RECREATE(sail_int)(&zghz3320);
    CONVERT_OF(sail_int, mach_int)(&zghz3320, INT64_C(64));
    RECREATE(lbits)(&zghz3321);
    UNDEFINED(lbits)(&zghz3321, zghz3320);
    zi_top = CONVERT_OF(fbits, lbits)(zghz3321, true);
  }
  int64_t zgaz3183;
  zgaz3183 = sailgen_CapGetExponent(zc);
  {
    int64_t zexp;
    zexp = zgaz3183;
    bool zgaz3185;
    {
      int64_t zgaz3184;
      {
        RECREATE(sail_int)(&zghz3317);
        CONVERT_OF(sail_int, mach_int)(&zghz3317, zCAP_MAX_EXPONENT);
        RECREATE(sail_int)(&zghz3318);
        CONVERT_OF(sail_int, mach_int)(&zghz3318, INT64_C(2));
        RECREATE(sail_int)(&zghz3319);
        sub_int(&zghz3319, zghz3317, zghz3318);
        zgaz3184 = CONVERT_OF(mach_int, sail_int)(zghz3319);
      }
      zgaz3185 = (zexp >= zgaz3184);
    }
    if (zgaz3185) {
      __label__ cleanup_242, end_cleanup_243;

      zcbz333 = true;
      goto cleanup_242;
      /* unreachable after return */
      goto end_cleanup_243;
    cleanup_242: ;
      goto end_function_235;
    end_cleanup_243: ;
    } else {
      uint64_t za;
      za = sailgen_CapGetValue(zc);
      uint64_t zashadowz33;
      zashadowz33 = sailgen_CapBoundsAddress(za);
      uint64_t zincrement_nameshadowz34;
      zincrement_nameshadowz34 = sailgen_CapBoundsAddress(zincrement_name);
      uint64_t zi_topshadowz35;
      {
        int64_t zgaz3201;
        zgaz3201 = (zexp + zCAP_MW);
        {
          RECREATE(lbits)(&zghz3314);
          CONVERT_OF(lbits, fbits)(&zghz3314, zincrement_nameshadowz34, UINT64_C(64) , true);
          RECREATE(sail_int)(&zghz3315);
          CONVERT_OF(sail_int, mach_int)(&zghz3315, zgaz3201);
          RECREATE(lbits)(&zghz3316);
          arith_shiftr(&zghz3316, zghz3314, zghz3315);
          zi_topshadowz35 = CONVERT_OF(fbits, lbits)(zghz3316, true);
        }
      }
      uint64_t zi_midshadowz36;
      {
        uint64_t zgaz3199;
        {
          RECREATE(lbits)(&zghz3311);
          CONVERT_OF(lbits, fbits)(&zghz3311, zincrement_nameshadowz34, UINT64_C(64) , true);
          RECREATE(sail_int)(&zghz3312);
          CONVERT_OF(sail_int, mach_int)(&zghz3312, zexp);
          RECREATE(lbits)(&zghz3313);
          shiftr(&zghz3313, zghz3311, zghz3312);
          zgaz3199 = CONVERT_OF(fbits, lbits)(zghz3313, true);
        }
        int64_t zgaz3200;
        {
          RECREATE(sail_int)(&zghz3308);
          CONVERT_OF(sail_int, mach_int)(&zghz3308, zCAP_MW);
          RECREATE(sail_int)(&zghz3309);
          CONVERT_OF(sail_int, mach_int)(&zghz3309, INT64_C(1));
          RECREATE(sail_int)(&zghz3310);
          sub_int(&zghz3310, zghz3308, zghz3309);
          zgaz3200 = CONVERT_OF(mach_int, sail_int)(zghz3310);
        }
        zi_midshadowz36 = (safe_rshift(UINT64_MAX, 64 - 16) & (zgaz3199 >> INT64_C(0)));
      }
      uint64_t za_midshadowz37;
      {
        uint64_t zgaz3197;
        {
          RECREATE(lbits)(&zghz3305);
          CONVERT_OF(lbits, fbits)(&zghz3305, zashadowz33, UINT64_C(64) , true);
          RECREATE(sail_int)(&zghz3306);
          CONVERT_OF(sail_int, mach_int)(&zghz3306, zexp);
          RECREATE(lbits)(&zghz3307);
          shiftr(&zghz3307, zghz3305, zghz3306);
          zgaz3197 = CONVERT_OF(fbits, lbits)(zghz3307, true);
        }
        int64_t zgaz3198;
        {
          RECREATE(sail_int)(&zghz3302);
          CONVERT_OF(sail_int, mach_int)(&zghz3302, zCAP_MW);
          RECREATE(sail_int)(&zghz3303);
          CONVERT_OF(sail_int, mach_int)(&zghz3303, INT64_C(1));
          RECREATE(sail_int)(&zghz3304);
          sub_int(&zghz3304, zghz3302, zghz3303);
          zgaz3198 = CONVERT_OF(mach_int, sail_int)(zghz3304);
        }
        za_midshadowz37 = (safe_rshift(UINT64_MAX, 64 - 16) & (zgaz3197 >> INT64_C(0)));
      }
      uint64_t zB3shadowz38;
      {
        uint64_t zgaz3194;
        zgaz3194 = sailgen_CapGetBottom(zc);
        int64_t zgaz3195;
        {
          RECREATE(sail_int)(&zghz3299);
          CONVERT_OF(sail_int, mach_int)(&zghz3299, zCAP_MW);
          RECREATE(sail_int)(&zghz3300);
          CONVERT_OF(sail_int, mach_int)(&zghz3300, INT64_C(1));
          RECREATE(sail_int)(&zghz3301);
          sub_int(&zghz3301, zghz3299, zghz3300);
          zgaz3195 = CONVERT_OF(mach_int, sail_int)(zghz3301);
        }
        int64_t zgaz3196;
        {
          RECREATE(sail_int)(&zghz3296);
          CONVERT_OF(sail_int, mach_int)(&zghz3296, zCAP_MW);
          RECREATE(sail_int)(&zghz3297);
          CONVERT_OF(sail_int, mach_int)(&zghz3297, INT64_C(3));
          RECREATE(sail_int)(&zghz3298);
          sub_int(&zghz3298, zghz3296, zghz3297);
          zgaz3196 = CONVERT_OF(mach_int, sail_int)(zghz3298);
        }
        zB3shadowz38 = (safe_rshift(UINT64_MAX, 64 - 3) & (zgaz3194 >> zgaz3196));
      }
      uint64_t zR3shadowz39;
      zR3shadowz39 = ((zB3shadowz38 - UINT64_C(0b001)) & UINT64_C(0b111));
      uint64_t zRshadowz310;
      {
        uint64_t zgaz3193;
        {
          int64_t zgaz3192;
          {
            RECREATE(sail_int)(&zghz3293);
            CONVERT_OF(sail_int, mach_int)(&zghz3293, zCAP_MW);
            RECREATE(sail_int)(&zghz3294);
            CONVERT_OF(sail_int, mach_int)(&zghz3294, INT64_C(3));
            RECREATE(sail_int)(&zghz3295);
            sub_int(&zghz3295, zghz3293, zghz3294);
            zgaz3192 = CONVERT_OF(mach_int, sail_int)(zghz3295);
          }
          {
            RECREATE(sail_int)(&zghz3291);
            CONVERT_OF(sail_int, mach_int)(&zghz3291, zgaz3192);
            RECREATE(lbits)(&zghz3292);
            sailgen_Zeros(&zghz3292, zghz3291);
            zgaz3193 = CONVERT_OF(fbits, lbits)(zghz3292, true);
          }
        }
        zRshadowz310 = (zR3shadowz39 << 13) | zgaz3193;
      }
      uint64_t zdiffshadowz311;
      zdiffshadowz311 = ((zRshadowz310 - za_midshadowz37) & UINT64_C(0xFFFF));
      uint64_t zdiff1shadowz312;
      {
        RECREATE(lbits)(&zghz3288);
        CONVERT_OF(lbits, fbits)(&zghz3288, zdiffshadowz311, UINT64_C(16) , true);
        RECREATE(sail_int)(&zghz3289);
        CONVERT_OF(sail_int, mach_int)(&zghz3289, INT64_C(1));
        RECREATE(lbits)(&zghz3290);
        sub_bits_int(&zghz3290, zghz3288, zghz3289);
        zdiff1shadowz312 = CONVERT_OF(fbits, lbits)(zghz3290, true);
      }
      bool zgaz3186;
      {
        RECREATE(lbits)(&zghz3286);
        CONVERT_OF(lbits, fbits)(&zghz3286, zi_topshadowz35, UINT64_C(64) , true);
        RECREATE(sail_int)(&zghz3287);
        CONVERT_OF(sail_int, mach_int)(&zghz3287, INT64_C(0));
        zgaz3186 = sailgen_eq_bits_int(zghz3286, zghz3287);
      }
      if (zgaz3186) {
        __label__ cleanup_240, end_cleanup_241;

        bool zgaz3187;
        {
          RECREATE(lbits)(&zghz3284);
          CONVERT_OF(lbits, fbits)(&zghz3284, zi_midshadowz36, UINT64_C(16) , true);
          RECREATE(lbits)(&zghz3285);
          CONVERT_OF(lbits, fbits)(&zghz3285, zdiff1shadowz312, UINT64_C(16) , true);
          zgaz3187 = sailgen_CapUnsignedLessThan(zghz3284, zghz3285);
        }
        zcbz333 = zgaz3187;
        goto cleanup_240;
        /* unreachable after return */
        goto end_cleanup_241;
      cleanup_240: ;
        goto end_function_235;
      end_cleanup_241: ;
      } else {
        bool zgaz3189;
        {
          uint64_t zgaz3188;
          {
            RECREATE(sail_int)(&zghz3282);
            CONVERT_OF(sail_int, mach_int)(&zghz3282, zCAP_VALUE_NUM_BITS);
            RECREATE(lbits)(&zghz3283);
            sailgen_Ones(&zghz3283, zghz3282);
            zgaz3188 = CONVERT_OF(fbits, lbits)(zghz3283, true);
          }
          zgaz3189 = (zi_topshadowz35 == zgaz3188);
        }
        if (zgaz3189) {
          __label__ cleanup_238, end_cleanup_239;

          bool zgaz3191;
          {
            bool zgaz3190;
            {
              RECREATE(lbits)(&zghz3280);
              CONVERT_OF(lbits, fbits)(&zghz3280, zi_midshadowz36, UINT64_C(16) , true);
              RECREATE(lbits)(&zghz3281);
              CONVERT_OF(lbits, fbits)(&zghz3281, zdiffshadowz311, UINT64_C(16) , true);
              zgaz3190 = sailgen_CapUnsignedGreaterThanOrEqual(zghz3280, zghz3281);
            }
            bool zgsz3132;
            if (zgaz3190) {  zgsz3132 = (zRshadowz310 != za_midshadowz37);  } else {  zgsz3132 = false;  }
            zgaz3191 = zgsz3132;
          }
          zcbz333 = zgaz3191;
          goto cleanup_238;
          /* unreachable after return */
          goto end_cleanup_239;
        cleanup_238: ;
          goto end_function_235;
        end_cleanup_239: ;
        } else {
          __label__ cleanup_236, end_cleanup_237;

          zcbz333 = false;
          goto cleanup_236;
          /* unreachable after return */
          goto end_cleanup_237;
        cleanup_236: ;
          goto end_function_235;
        end_cleanup_237: ;
        }
      }
    }
    goto finish_match_233;
  }
case_234: ;
  sail_match_failure("CapIsRepresentableFast");
finish_match_233: ;










end_function_235: ;
  return zcbz333;
end_block_exception_244: ;

  return false;
}



static void finish_sailgen_CapIsRepresentableFast(void)
{
  KILL(sail_int)(&zghz3368);
  KILL(sail_int)(&zghz3367);
  KILL(sail_int)(&zghz3366);
  KILL(sail_int)(&zghz3365);
  KILL(sail_int)(&zghz3364);
  KILL(sail_int)(&zghz3363);
  KILL(sail_int)(&zghz3362);
  KILL(sail_int)(&zghz3361);
  KILL(sail_int)(&zghz3360);
  KILL(sail_int)(&zghz3359);
  KILL(sail_int)(&zghz3358);
  KILL(sail_int)(&zghz3357);
  KILL(sail_int)(&zghz3356);
  KILL(sail_int)(&zghz3355);
  KILL(sail_int)(&zghz3354);
  KILL(lbits)(&zghz3353);
  KILL(sail_int)(&zghz3352);
  KILL(lbits)(&zghz3351);
  KILL(sail_int)(&zghz3350);
  KILL(lbits)(&zghz3349);
  KILL(sail_int)(&zghz3348);
  KILL(sail_int)(&zghz3347);
  KILL(sail_int)(&zghz3346);
  KILL(sail_int)(&zghz3345);
  KILL(sail_int)(&zghz3344);
  KILL(sail_int)(&zghz3343);
  KILL(sail_int)(&zghz3342);
  KILL(sail_int)(&zghz3341);
  KILL(sail_int)(&zghz3340);
  KILL(sail_int)(&zghz3339);
  KILL(lbits)(&zghz3338);
  KILL(sail_int)(&zghz3337);
  KILL(lbits)(&zghz3336);
  KILL(sail_int)(&zghz3335);
  KILL(lbits)(&zghz3334);
  KILL(sail_int)(&zghz3333);
  KILL(sail_int)(&zghz3332);
  KILL(sail_int)(&zghz3331);
  KILL(sail_int)(&zghz3330);
  KILL(sail_int)(&zghz3329);
  KILL(sail_int)(&zghz3328);
  KILL(sail_int)(&zghz3327);
  KILL(sail_int)(&zghz3326);
  KILL(sail_int)(&zghz3325);
  KILL(sail_int)(&zghz3324);
  KILL(lbits)(&zghz3323);
  KILL(sail_int)(&zghz3322);
  KILL(lbits)(&zghz3321);
  KILL(sail_int)(&zghz3320);
  KILL(sail_int)(&zghz3319);
  KILL(sail_int)(&zghz3318);
  KILL(sail_int)(&zghz3317);
  KILL(lbits)(&zghz3316);
  KILL(sail_int)(&zghz3315);
  KILL(lbits)(&zghz3314);
  KILL(lbits)(&zghz3313);
  KILL(sail_int)(&zghz3312);
  KILL(lbits)(&zghz3311);
  KILL(sail_int)(&zghz3310);
  KILL(sail_int)(&zghz3309);
  KILL(sail_int)(&zghz3308);
  KILL(lbits)(&zghz3307);
  KILL(sail_int)(&zghz3306);
  KILL(lbits)(&zghz3305);
  KILL(sail_int)(&zghz3304);
  KILL(sail_int)(&zghz3303);
  KILL(sail_int)(&zghz3302);
  KILL(sail_int)(&zghz3301);
  KILL(sail_int)(&zghz3300);
  KILL(sail_int)(&zghz3299);
  KILL(sail_int)(&zghz3298);
  KILL(sail_int)(&zghz3297);
  KILL(sail_int)(&zghz3296);
  KILL(sail_int)(&zghz3295);
  KILL(sail_int)(&zghz3294);
  KILL(sail_int)(&zghz3293);
  KILL(lbits)(&zghz3292);
  KILL(sail_int)(&zghz3291);
  KILL(lbits)(&zghz3290);
  KILL(sail_int)(&zghz3289);
  KILL(lbits)(&zghz3288);
  KILL(sail_int)(&zghz3287);
  KILL(lbits)(&zghz3286);
  KILL(lbits)(&zghz3285);
  KILL(lbits)(&zghz3284);
  KILL(lbits)(&zghz3283);
  KILL(sail_int)(&zghz3282);
  KILL(lbits)(&zghz3281);
  KILL(lbits)(&zghz3280);
}

// register SP_EL3
static lbits zSP_EL3;

static int64_t zCAPABILITY_DBITS;
static void create_letbind_69(void) {


  int64_t zgsz3133;
  {
    sail_int zgsz31011;
    CREATE(sail_int)(&zgsz31011);
    CONVERT_OF(sail_int, mach_int)(&zgsz31011, zCAPABILITY_DBYTES);
    sail_int zgsz31012;
    CREATE(sail_int)(&zgsz31012);
    CONVERT_OF(sail_int, mach_int)(&zgsz31012, INT64_C(8));
    sail_int zgsz31013;
    CREATE(sail_int)(&zgsz31013);
    mult_int(&zgsz31013, zgsz31011, zgsz31012);
    zgsz3133 = CONVERT_OF(mach_int, sail_int)(zgsz31013);
    KILL(sail_int)(&zgsz31013);
    KILL(sail_int)(&zgsz31012);
    KILL(sail_int)(&zgsz31011);
  }
  zCAPABILITY_DBITS = zgsz3133;

let_end_245: ;
}
static void kill_letbind_69(void) {
}

static bool sailgen_CapUnsignedLessThanOrEqual(lbits, lbits);

sail_int zghz3369;
sail_int zghz3370;

static void startup_sailgen_CapUnsignedLessThanOrEqual(void)
{
  CREATE(sail_int)(&zghz3369);
  CREATE(sail_int)(&zghz3370);
}

static bool sailgen_CapUnsignedLessThanOrEqual(lbits za, lbits zb)
{
  __label__ cleanup_248, end_cleanup_249, end_function_247, end_block_exception_250;

  bool zcbz334;
  {
    RECREATE(sail_int)(&zghz3369);
    sail_unsigned(&zghz3369, za);
    RECREATE(sail_int)(&zghz3370);
    sail_unsigned(&zghz3370, zb);
    zcbz334 = lteq(zghz3369, zghz3370);
  }
  goto cleanup_248;
  /* unreachable after return */
  goto end_cleanup_249;
cleanup_248: ;
  goto end_function_247;
end_cleanup_249: ;
end_function_247: ;
  return zcbz334;
end_block_exception_250: ;

  return false;
}



static void finish_sailgen_CapUnsignedLessThanOrEqual(void)
{
  KILL(sail_int)(&zghz3370);
  KILL(sail_int)(&zghz3369);
}

static uint64_t sailgen_CapGetTag(lbits);

lbits zghz3371;
sail_int zghz3372;
lbits zghz3373;
sail_int zghz3374;

static void startup_sailgen_CapGetTag(void)
{
  CREATE(lbits)(&zghz3371);
  CREATE(sail_int)(&zghz3372);
  CREATE(lbits)(&zghz3373);
  CREATE(sail_int)(&zghz3374);
}

static uint64_t sailgen_CapGetTag(lbits zc)
{
  __label__ cleanup_253, end_cleanup_254, end_function_252, end_block_exception_255;

  uint64_t zcbz335;
  {
    uint64_t zgaz3227;
    {
      fbits zgaz3226;
      {
        RECREATE(sail_int)(&zghz3374);
        CONVERT_OF(sail_int, mach_int)(&zghz3374, zCAP_TAG_BIT);
        zgaz3226 = bitvector_access(zc, zghz3374);
      }
      zgaz3227 = UINT64_C(0b0);
      zgaz3227 = update_fbits(zgaz3227, INT64_C(0), zgaz3226);
    }
    {
      RECREATE(lbits)(&zghz3371);
      CONVERT_OF(lbits, fbits)(&zghz3371, zgaz3227, UINT64_C(1) , true);
      RECREATE(sail_int)(&zghz3372);
      CONVERT_OF(sail_int, mach_int)(&zghz3372, INT64_C(64));
      RECREATE(lbits)(&zghz3373);
      zero_extend(&zghz3373, zghz3371, zghz3372);
      zcbz335 = CONVERT_OF(fbits, lbits)(zghz3373, true);
    }
  }
  goto cleanup_253;
  /* unreachable after return */
  goto end_cleanup_254;
cleanup_253: ;
  goto end_function_252;
end_cleanup_254: ;
end_function_252: ;
  return zcbz335;
end_block_exception_255: ;

  return UINT64_C(0xdeadc0de);
}



static void finish_sailgen_CapGetTag(void)
{
  KILL(sail_int)(&zghz3374);
  KILL(lbits)(&zghz3373);
  KILL(sail_int)(&zghz3372);
  KILL(lbits)(&zghz3371);
}

static void sailgen_CapSetBounds(lbits *rop, lbits, lbits, bool);

lbits zghz3375;
lbits zghz3376;
sail_int zghz3377;
lbits zghz3378;
lbits zghz3379;
lbits zghz3380;
sail_int zghz3381;
lbits zghz3382;
sail_int zghz3383;
sail_int zghz3384;
lbits zghz3385;
sail_int zghz3386;
lbits zghz3387;
sail_int zghz3388;
sail_int zghz3389;
sail_int zghz3390;
sail_int zghz3391;
sail_int zghz3392;
sail_int zghz3393;
lbits zghz3394;
sail_int zghz3395;
sail_int zghz3396;
sail_int zghz3397;
sail_int zghz3398;
sail_int zghz3399;
lbits zghz3400;
sail_int zghz3401;
sail_int zghz3402;
sail_int zghz3403;
sail_int zghz3404;
sail_int zghz3405;
lbits zghz3406;
sail_int zghz3407;
sail_int zghz3408;
lbits zghz3409;
sail_int zghz3410;
sail_int zghz3411;
sail_int zghz3412;
lbits zghz3413;
sail_int zghz3414;
sail_int zghz3415;
lbits zghz3416;
sail_int zghz3417;
sail_int zghz3418;
lbits zghz3419;
sail_int zghz3420;
sail_int zghz3421;
lbits zghz3422;
sail_int zghz3423;
sail_int zghz3424;
lbits zghz3425;
sail_int zghz3426;
sail_int zghz3427;
lbits zghz3428;
sail_int zghz3429;
sail_int zghz3430;
lbits zghz3431;
lbits zghz3432;
lbits zghz3433;
sail_int zghz3434;
sail_int zghz3435;
lbits zghz3436;
sail_int zghz3437;
lbits zghz3438;
sail_int zghz3439;
sail_int zghz3440;
lbits zghz3441;
sail_int zghz3442;
sail_int zghz3443;
sail_int zghz3444;
sail_int zghz3445;
sail_int zghz3446;
sail_int zghz3447;
lbits zghz3448;
sail_int zghz3449;
sail_int zghz3450;
sail_int zghz3451;
sail_int zghz3452;
sail_int zghz3453;
sail_int zghz3454;
sail_int zghz3455;
sail_int zghz3456;
sail_int zghz3457;
sail_int zghz3458;
sail_int zghz3459;
sail_int zghz3460;
lbits zghz3461;
sail_int zghz3462;
lbits zghz3463;
lbits zghz3464;
lbits zghz3465;
sail_int zghz3466;
lbits zghz3467;
lbits zghz3468;
sail_int zghz3469;
sail_int zghz3470;
lbits zghz3471;
sail_int zghz3472;
sail_int zghz3473;
sail_int zghz3474;
sail_int zghz3475;
sail_int zghz3476;
sail_int zghz3477;
sail_int zghz3478;
sail_int zghz3479;
sail_int zghz3480;
sail_int zghz3481;
sail_int zghz3482;
sail_int zghz3483;
sail_int zghz3484;
sail_int zghz3485;
sail_int zghz3486;
lbits zghz3487;
sail_int zghz3488;
sail_int zghz3489;
sail_int zghz3490;
sail_int zghz3491;
sail_int zghz3492;
sail_int zghz3493;
sail_int zghz3494;
sail_int zghz3495;
sail_int zghz3496;
sail_int zghz3497;
sail_int zghz3498;
sail_int zghz3499;
sail_int zghz3500;
sail_int zghz3501;
sail_int zghz3502;
sail_int zghz3503;
sail_int zghz3504;
sail_int zghz3505;
sail_int zghz3506;
sail_int zghz3507;
lbits zghz3508;
sail_int zghz3509;
sail_int zghz3510;
sail_int zghz3511;
sail_int zghz3512;
sail_int zghz3513;
sail_int zghz3514;
sail_int zghz3515;
sail_int zghz3516;
sail_int zghz3517;
sail_int zghz3518;
sail_int zghz3519;
sail_int zghz3520;
sail_int zghz3521;
sail_int zghz3522;
sail_int zghz3523;
sail_int zghz3524;
sail_int zghz3525;
sail_int zghz3526;
sail_int zghz3527;
lbits zghz3528;
sail_int zghz3529;
sail_int zghz3530;
sail_int zghz3531;
sail_int zghz3532;
sail_int zghz3533;
lbits zghz3534;
sail_int zghz3535;
sail_int zghz3536;
sail_int zghz3537;
lbits zghz3538;
lbits zghz3539;
lbits zghz3540;
lbits zghz3541;
sail_int zghz3542;
sail_int zghz3543;
sail_int zghz3544;
sail_int zghz3545;
sail_int zghz3546;
sail_int zghz3547;
sail_int zghz3548;
sail_int zghz3549;
lbits zghz3550;
sail_int zghz3551;
sail_int zghz3552;
sail_int zghz3553;
lbits zghz3554;
sail_int zghz3555;
sail_int zghz3556;
sail_int zghz3557;
sail_int zghz3558;
sail_int zghz3559;
sail_int zghz3560;
sail_int zghz3561;
sail_int zghz3562;
sail_int zghz3563;
sail_int zghz3564;
sail_int zghz3565;
sail_int zghz3566;
lbits zghz3567;

static void startup_sailgen_CapSetBounds(void)
{
  CREATE(lbits)(&zghz3375);
  CREATE(lbits)(&zghz3376);
  CREATE(sail_int)(&zghz3377);
  CREATE(lbits)(&zghz3378);
  CREATE(lbits)(&zghz3379);
  CREATE(lbits)(&zghz3380);
  CREATE(sail_int)(&zghz3381);
  CREATE(lbits)(&zghz3382);
  CREATE(sail_int)(&zghz3383);
  CREATE(sail_int)(&zghz3384);
  CREATE(lbits)(&zghz3385);
  CREATE(sail_int)(&zghz3386);
  CREATE(lbits)(&zghz3387);
  CREATE(sail_int)(&zghz3388);
  CREATE(sail_int)(&zghz3389);
  CREATE(sail_int)(&zghz3390);
  CREATE(sail_int)(&zghz3391);
  CREATE(sail_int)(&zghz3392);
  CREATE(sail_int)(&zghz3393);
  CREATE(lbits)(&zghz3394);
  CREATE(sail_int)(&zghz3395);
  CREATE(sail_int)(&zghz3396);
  CREATE(sail_int)(&zghz3397);
  CREATE(sail_int)(&zghz3398);
  CREATE(sail_int)(&zghz3399);
  CREATE(lbits)(&zghz3400);
  CREATE(sail_int)(&zghz3401);
  CREATE(sail_int)(&zghz3402);
  CREATE(sail_int)(&zghz3403);
  CREATE(sail_int)(&zghz3404);
  CREATE(sail_int)(&zghz3405);
  CREATE(lbits)(&zghz3406);
  CREATE(sail_int)(&zghz3407);
  CREATE(sail_int)(&zghz3408);
  CREATE(lbits)(&zghz3409);
  CREATE(sail_int)(&zghz3410);
  CREATE(sail_int)(&zghz3411);
  CREATE(sail_int)(&zghz3412);
  CREATE(lbits)(&zghz3413);
  CREATE(sail_int)(&zghz3414);
  CREATE(sail_int)(&zghz3415);
  CREATE(lbits)(&zghz3416);
  CREATE(sail_int)(&zghz3417);
  CREATE(sail_int)(&zghz3418);
  CREATE(lbits)(&zghz3419);
  CREATE(sail_int)(&zghz3420);
  CREATE(sail_int)(&zghz3421);
  CREATE(lbits)(&zghz3422);
  CREATE(sail_int)(&zghz3423);
  CREATE(sail_int)(&zghz3424);
  CREATE(lbits)(&zghz3425);
  CREATE(sail_int)(&zghz3426);
  CREATE(sail_int)(&zghz3427);
  CREATE(lbits)(&zghz3428);
  CREATE(sail_int)(&zghz3429);
  CREATE(sail_int)(&zghz3430);
  CREATE(lbits)(&zghz3431);
  CREATE(lbits)(&zghz3432);
  CREATE(lbits)(&zghz3433);
  CREATE(sail_int)(&zghz3434);
  CREATE(sail_int)(&zghz3435);
  CREATE(lbits)(&zghz3436);
  CREATE(sail_int)(&zghz3437);
  CREATE(lbits)(&zghz3438);
  CREATE(sail_int)(&zghz3439);
  CREATE(sail_int)(&zghz3440);
  CREATE(lbits)(&zghz3441);
  CREATE(sail_int)(&zghz3442);
  CREATE(sail_int)(&zghz3443);
  CREATE(sail_int)(&zghz3444);
  CREATE(sail_int)(&zghz3445);
  CREATE(sail_int)(&zghz3446);
  CREATE(sail_int)(&zghz3447);
  CREATE(lbits)(&zghz3448);
  CREATE(sail_int)(&zghz3449);
  CREATE(sail_int)(&zghz3450);
  CREATE(sail_int)(&zghz3451);
  CREATE(sail_int)(&zghz3452);
  CREATE(sail_int)(&zghz3453);
  CREATE(sail_int)(&zghz3454);
  CREATE(sail_int)(&zghz3455);
  CREATE(sail_int)(&zghz3456);
  CREATE(sail_int)(&zghz3457);
  CREATE(sail_int)(&zghz3458);
  CREATE(sail_int)(&zghz3459);
  CREATE(sail_int)(&zghz3460);
  CREATE(lbits)(&zghz3461);
  CREATE(sail_int)(&zghz3462);
  CREATE(lbits)(&zghz3463);
  CREATE(lbits)(&zghz3464);
  CREATE(lbits)(&zghz3465);
  CREATE(sail_int)(&zghz3466);
  CREATE(lbits)(&zghz3467);
  CREATE(lbits)(&zghz3468);
  CREATE(sail_int)(&zghz3469);
  CREATE(sail_int)(&zghz3470);
  CREATE(lbits)(&zghz3471);
  CREATE(sail_int)(&zghz3472);
  CREATE(sail_int)(&zghz3473);
  CREATE(sail_int)(&zghz3474);
  CREATE(sail_int)(&zghz3475);
  CREATE(sail_int)(&zghz3476);
  CREATE(sail_int)(&zghz3477);
  CREATE(sail_int)(&zghz3478);
  CREATE(sail_int)(&zghz3479);
  CREATE(sail_int)(&zghz3480);
  CREATE(sail_int)(&zghz3481);
  CREATE(sail_int)(&zghz3482);
  CREATE(sail_int)(&zghz3483);
  CREATE(sail_int)(&zghz3484);
  CREATE(sail_int)(&zghz3485);
  CREATE(sail_int)(&zghz3486);
  CREATE(lbits)(&zghz3487);
  CREATE(sail_int)(&zghz3488);
  CREATE(sail_int)(&zghz3489);
  CREATE(sail_int)(&zghz3490);
  CREATE(sail_int)(&zghz3491);
  CREATE(sail_int)(&zghz3492);
  CREATE(sail_int)(&zghz3493);
  CREATE(sail_int)(&zghz3494);
  CREATE(sail_int)(&zghz3495);
  CREATE(sail_int)(&zghz3496);
  CREATE(sail_int)(&zghz3497);
  CREATE(sail_int)(&zghz3498);
  CREATE(sail_int)(&zghz3499);
  CREATE(sail_int)(&zghz3500);
  CREATE(sail_int)(&zghz3501);
  CREATE(sail_int)(&zghz3502);
  CREATE(sail_int)(&zghz3503);
  CREATE(sail_int)(&zghz3504);
  CREATE(sail_int)(&zghz3505);
  CREATE(sail_int)(&zghz3506);
  CREATE(sail_int)(&zghz3507);
  CREATE(lbits)(&zghz3508);
  CREATE(sail_int)(&zghz3509);
  CREATE(sail_int)(&zghz3510);
  CREATE(sail_int)(&zghz3511);
  CREATE(sail_int)(&zghz3512);
  CREATE(sail_int)(&zghz3513);
  CREATE(sail_int)(&zghz3514);
  CREATE(sail_int)(&zghz3515);
  CREATE(sail_int)(&zghz3516);
  CREATE(sail_int)(&zghz3517);
  CREATE(sail_int)(&zghz3518);
  CREATE(sail_int)(&zghz3519);
  CREATE(sail_int)(&zghz3520);
  CREATE(sail_int)(&zghz3521);
  CREATE(sail_int)(&zghz3522);
  CREATE(sail_int)(&zghz3523);
  CREATE(sail_int)(&zghz3524);
  CREATE(sail_int)(&zghz3525);
  CREATE(sail_int)(&zghz3526);
  CREATE(sail_int)(&zghz3527);
  CREATE(lbits)(&zghz3528);
  CREATE(sail_int)(&zghz3529);
  CREATE(sail_int)(&zghz3530);
  CREATE(sail_int)(&zghz3531);
  CREATE(sail_int)(&zghz3532);
  CREATE(sail_int)(&zghz3533);
  CREATE(lbits)(&zghz3534);
  CREATE(sail_int)(&zghz3535);
  CREATE(sail_int)(&zghz3536);
  CREATE(sail_int)(&zghz3537);
  CREATE(lbits)(&zghz3538);
  CREATE(lbits)(&zghz3539);
  CREATE(lbits)(&zghz3540);
  CREATE(lbits)(&zghz3541);
  CREATE(sail_int)(&zghz3542);
  CREATE(sail_int)(&zghz3543);
  CREATE(sail_int)(&zghz3544);
  CREATE(sail_int)(&zghz3545);
  CREATE(sail_int)(&zghz3546);
  CREATE(sail_int)(&zghz3547);
  CREATE(sail_int)(&zghz3548);
  CREATE(sail_int)(&zghz3549);
  CREATE(lbits)(&zghz3550);
  CREATE(sail_int)(&zghz3551);
  CREATE(sail_int)(&zghz3552);
  CREATE(sail_int)(&zghz3553);
  CREATE(lbits)(&zghz3554);
  CREATE(sail_int)(&zghz3555);
  CREATE(sail_int)(&zghz3556);
  CREATE(sail_int)(&zghz3557);
  CREATE(sail_int)(&zghz3558);
  CREATE(sail_int)(&zghz3559);
  CREATE(sail_int)(&zghz3560);
  CREATE(sail_int)(&zghz3561);
  CREATE(sail_int)(&zghz3562);
  CREATE(sail_int)(&zghz3563);
  CREATE(sail_int)(&zghz3564);
  CREATE(sail_int)(&zghz3565);
  CREATE(sail_int)(&zghz3566);
  CREATE(lbits)(&zghz3567);
}

static void sailgen_CapSetBounds(lbits *zcbz336, lbits zc, lbits zreq_len, bool zexact)
{
  __label__ case_266, finish_match_265, end_function_269, end_block_exception_272, end_function_284;

  uint64_t zL_ie;
  {
    RECREATE(sail_int)(&zghz3566);
    CONVERT_OF(sail_int, mach_int)(&zghz3566, INT64_C(13));
    RECREATE(lbits)(&zghz3567);
    UNDEFINED(lbits)(&zghz3567, zghz3566);
    zL_ie = CONVERT_OF(fbits, lbits)(zghz3567, true);
  }
  RECREATE(lbits)(&zghz3375);
  {
    int64_t zgaz3330;
    {
      int64_t zgaz3329;
      {
        int64_t zgaz3328;
        {
          RECREATE(sail_int)(&zghz3563);
          CONVERT_OF(sail_int, mach_int)(&zghz3563, INT64_C(63));
          RECREATE(sail_int)(&zghz3564);
          CONVERT_OF(sail_int, mach_int)(&zghz3564, INT64_C(0));
          RECREATE(sail_int)(&zghz3565);
          sub_int(&zghz3565, zghz3563, zghz3564);
          zgaz3328 = CONVERT_OF(mach_int, sail_int)(zghz3565);
        }
        zgaz3329 = (zgaz3328 + INT64_C(1));
      }
      zgaz3330 = (zgaz3329 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3562);
      CONVERT_OF(sail_int, mach_int)(&zghz3562, zgaz3330);
      UNDEFINED(lbits)(&zghz3375, zghz3562);
    }
  }
  RECREATE(lbits)(&zghz3376);
  {
    int64_t zgaz3327;
    {
      int64_t zgaz3326;
      {
        int64_t zgaz3325;
        {
          RECREATE(sail_int)(&zghz3559);
          CONVERT_OF(sail_int, mach_int)(&zghz3559, INT64_C(63));
          RECREATE(sail_int)(&zghz3560);
          CONVERT_OF(sail_int, mach_int)(&zghz3560, INT64_C(0));
          RECREATE(sail_int)(&zghz3561);
          sub_int(&zghz3561, zghz3559, zghz3560);
          zgaz3325 = CONVERT_OF(mach_int, sail_int)(zghz3561);
        }
        zgaz3326 = (zgaz3325 + INT64_C(1));
      }
      zgaz3327 = (zgaz3326 + INT64_C(1));
    }
    {
      RECREATE(sail_int)(&zghz3558);
      CONVERT_OF(sail_int, mach_int)(&zghz3558, zgaz3327);
      UNDEFINED(lbits)(&zghz3376, zghz3558);
    }
  }
  bool zovalid;
  zovalid = false;
  {
    bool zgaz3229;
    zgaz3229 = sailgen_CapUnsignedLessThanOrEqual(zreq_len, zCAP_BOUND_MAX);
    unit zgsz3135;
    zgsz3135 = sail_assert(zgaz3229, "v8_base.sail:45083.61-45083.62");
  }
  RECREATE(sail_int)(&zghz3377);
  {
    int64_t zgaz3324;
    {
      uint64_t zgaz3323;
      {
        int64_t zgaz3322;
        {
          RECREATE(sail_int)(&zghz3555);
          CONVERT_OF(sail_int, mach_int)(&zghz3555, zCAP_MW);
          RECREATE(sail_int)(&zghz3556);
          CONVERT_OF(sail_int, mach_int)(&zghz3556, INT64_C(1));
          RECREATE(sail_int)(&zghz3557);
          sub_int(&zghz3557, zghz3555, zghz3556);
          zgaz3322 = CONVERT_OF(mach_int, sail_int)(zghz3557);
        }
        {
          RECREATE(sail_int)(&zghz3552);
          CONVERT_OF(sail_int, mach_int)(&zghz3552, zCAP_VALUE_NUM_BITS);
          RECREATE(sail_int)(&zghz3553);
          CONVERT_OF(sail_int, mach_int)(&zghz3553, zgaz3322);
          RECREATE(lbits)(&zghz3554);
          vector_subrange_lbits(&zghz3554, zreq_len, zghz3552, zghz3553);
          zgaz3323 = CONVERT_OF(fbits, lbits)(zghz3554, true);
        }
      }
      {
        RECREATE(lbits)(&zghz3550);
        CONVERT_OF(lbits, fbits)(&zghz3550, zgaz3323, UINT64_C(50) , true);
        RECREATE(sail_int)(&zghz3551);
        count_leading_zeros(&zghz3551, zghz3550);
        zgaz3324 = CONVERT_OF(mach_int, sail_int)(zghz3551);
      }
    }
    {
      RECREATE(sail_int)(&zghz3548);
      CONVERT_OF(sail_int, mach_int)(&zghz3548, zCAP_MAX_EXPONENT);
      RECREATE(sail_int)(&zghz3549);
      CONVERT_OF(sail_int, mach_int)(&zghz3549, zgaz3324);
      sub_int(&zghz3377, zghz3548, zghz3549);
    }
  }
  bool zie;
  {
    bool zgaz3321;
    {
      RECREATE(sail_int)(&zghz3547);
      CONVERT_OF(sail_int, mach_int)(&zghz3547, INT64_C(0));
      zgaz3321 = sailgen_neq_int(zghz3377, zghz3547);
    }
    bool zgsz3137;
    if (zgaz3321) {  zgsz3137 = true;  } else {
      uint64_t zgaz3320;
      {
        fbits zgaz3319;
        {
          int64_t zgaz3318;
          {
            RECREATE(sail_int)(&zghz3544);
            CONVERT_OF(sail_int, mach_int)(&zghz3544, zCAP_MW);
            RECREATE(sail_int)(&zghz3545);
            CONVERT_OF(sail_int, mach_int)(&zghz3545, INT64_C(2));
            RECREATE(sail_int)(&zghz3546);
            sub_int(&zghz3546, zghz3544, zghz3545);
            zgaz3318 = CONVERT_OF(mach_int, sail_int)(zghz3546);
          }
          {
            RECREATE(sail_int)(&zghz3543);
            CONVERT_OF(sail_int, mach_int)(&zghz3543, zgaz3318);
            zgaz3319 = bitvector_access(zreq_len, zghz3543);
          }
        }
        zgaz3320 = UINT64_C(0b0);
        zgaz3320 = update_fbits(zgaz3320, INT64_C(0), zgaz3319);
      }
      zgsz3137 = (zgaz3320 == UINT64_C(0b1));
    }
    zie = zgsz3137;
  }
  uint64_t zbase;
  zbase = sailgen_CapGetValue(zc);
  uint64_t zabase;
  {
    bool zgaz3317;
    {
      int64_t zgaz3316;
      zgaz3316 = sailgen_CapGetExponent(zc);
      {
        RECREATE(sail_int)(&zghz3542);
        CONVERT_OF(sail_int, mach_int)(&zghz3542, zgaz3316);
        zgaz3317 = sailgen_CapBoundsUsesValue(zghz3542);
      }
    }
    if (zgaz3317) {  zabase = sailgen_CapBoundsAddress(zbase);  } else {  zabase = zbase;  }
  }
  RECREATE(lbits)(&zghz3378);
  {
    RECREATE(lbits)(&zghz3540);
    CONVERT_OF(lbits, fbits)(&zghz3540, UINT64_C(0b00), UINT64_C(2) , true);
    RECREATE(lbits)(&zghz3541);
    CONVERT_OF(lbits, fbits)(&zghz3541, zabase, UINT64_C(64) , true);
    append(&zghz3378, zghz3540, zghz3541);
  }
  RECREATE(lbits)(&zghz3379);
  {
    RECREATE(lbits)(&zghz3538);
    {
      RECREATE(lbits)(&zghz3539);
      CONVERT_OF(lbits, fbits)(&zghz3539, UINT64_C(0b0), UINT64_C(1) , true);
      append(&zghz3538, zghz3539, zreq_len);
    }
    add_bits(&zghz3379, zghz3378, zghz3538);
  }
  uint64_t zBbits;
  {
    int64_t zgaz3314;
    {
      RECREATE(sail_int)(&zghz3535);
      CONVERT_OF(sail_int, mach_int)(&zghz3535, zCAP_MW);
      RECREATE(sail_int)(&zghz3536);
      CONVERT_OF(sail_int, mach_int)(&zghz3536, INT64_C(1));
      RECREATE(sail_int)(&zghz3537);
      sub_int(&zghz3537, zghz3535, zghz3536);
      zgaz3314 = CONVERT_OF(mach_int, sail_int)(zghz3537);
    }
    {
      RECREATE(sail_int)(&zghz3532);
      CONVERT_OF(sail_int, mach_int)(&zghz3532, zgaz3314);
      RECREATE(sail_int)(&zghz3533);
      CONVERT_OF(sail_int, mach_int)(&zghz3533, INT64_C(0));
      RECREATE(lbits)(&zghz3534);
      vector_subrange_lbits(&zghz3534, zghz3378, zghz3532, zghz3533);
      zBbits = CONVERT_OF(fbits, lbits)(zghz3534, true);
    }
  }
  uint64_t zTBits;
  {
    int64_t zgaz3313;
    {
      RECREATE(sail_int)(&zghz3529);
      CONVERT_OF(sail_int, mach_int)(&zghz3529, zCAP_MW);
      RECREATE(sail_int)(&zghz3530);
      CONVERT_OF(sail_int, mach_int)(&zghz3530, INT64_C(1));
      RECREATE(sail_int)(&zghz3531);
      sub_int(&zghz3531, zghz3529, zghz3530);
      zgaz3313 = CONVERT_OF(mach_int, sail_int)(zghz3531);
    }
    {
      RECREATE(sail_int)(&zghz3526);
      CONVERT_OF(sail_int, mach_int)(&zghz3526, zgaz3313);
      RECREATE(sail_int)(&zghz3527);
      CONVERT_OF(sail_int, mach_int)(&zghz3527, INT64_C(0));
      RECREATE(lbits)(&zghz3528);
      vector_subrange_lbits(&zghz3528, zghz3379, zghz3526, zghz3527);
      zTBits = CONVERT_OF(fbits, lbits)(zghz3528, true);
    }
  }
  bool zlostTop;
  zlostTop = false;
  bool zlostBottom;
  zlostBottom = false;
  bool zincrementE_name;
  zincrementE_name = false;
  {
    unit zgsz3138;
    if (zie) {
      uint64_t zB_ie;
      {
        __label__ case_258, finish_match_257;

        {
          RECREATE(sail_int)(&zghz3505);
          COPY(sail_int)(&zghz3505, zghz3377);
          {
            bool zgaz3278;
            {
              bool zgaz3277;
              {
                RECREATE(sail_int)(&zghz3522);
                {
                  RECREATE(sail_int)(&zghz3524);
                  sailgen___id(&zghz3524, zghz3505);
                  {
                    RECREATE(sail_int)(&zghz3525);
                    CONVERT_OF(sail_int, mach_int)(&zghz3525, INT64_C(3));
                    add_int(&zghz3522, zghz3524, zghz3525);
                  }
                }
                {
                  RECREATE(sail_int)(&zghz3523);
                  CONVERT_OF(sail_int, mach_int)(&zghz3523, INT64_C(0));
                  zgaz3277 = lteq(zghz3523, zghz3522);
                }
              }
              bool zgsz3140;
              if (zgaz3277) {
                RECREATE(sail_int)(&zghz3516);
                {
                  RECREATE(sail_int)(&zghz3518);
                  {
                    RECREATE(sail_int)(&zghz3520);
                    sailgen___id(&zghz3520, zghz3505);
                    {
                      RECREATE(sail_int)(&zghz3521);
                      CONVERT_OF(sail_int, mach_int)(&zghz3521, INT64_C(16));
                      add_int(&zghz3518, zghz3520, zghz3521);
                    }
                  }
                  {
                    RECREATE(sail_int)(&zghz3519);
                    CONVERT_OF(sail_int, mach_int)(&zghz3519, INT64_C(1));
                    sub_int(&zghz3516, zghz3518, zghz3519);
                  }
                }
                {
                  RECREATE(sail_int)(&zghz3517);
                  CONVERT_OF(sail_int, mach_int)(&zghz3517, INT64_C(66));
                  zgsz3140 = lt(zghz3516, zghz3517);
                }
              } else {  zgsz3140 = false;  }
              zgaz3278 = zgsz3140;
            }
            unit zgsz3141;
            zgsz3141 = sail_assert(zgaz3278, "v8_base.sail:45098.69-45098.70");
          }
          int64_t zgaz3280;
          {
            int64_t zgaz3279;
            {
              RECREATE(sail_int)(&zghz3514);
              CONVERT_OF(sail_int, mach_int)(&zghz3514, zCAP_MW);
              RECREATE(sail_int)(&zghz3515);
              add_int(&zghz3515, zghz3505, zghz3514);
              zgaz3279 = CONVERT_OF(mach_int, sail_int)(zghz3515);
            }
            {
              RECREATE(sail_int)(&zghz3511);
              CONVERT_OF(sail_int, mach_int)(&zghz3511, zgaz3279);
              RECREATE(sail_int)(&zghz3512);
              CONVERT_OF(sail_int, mach_int)(&zghz3512, INT64_C(1));
              RECREATE(sail_int)(&zghz3513);
              sub_int(&zghz3513, zghz3511, zghz3512);
              zgaz3280 = CONVERT_OF(mach_int, sail_int)(zghz3513);
            }
          }
          int64_t zgaz3281;
          {
            RECREATE(sail_int)(&zghz3509);
            CONVERT_OF(sail_int, mach_int)(&zghz3509, INT64_C(3));
            RECREATE(sail_int)(&zghz3510);
            add_int(&zghz3510, zghz3505, zghz3509);
            zgaz3281 = CONVERT_OF(mach_int, sail_int)(zghz3510);
          }
          {
            RECREATE(sail_int)(&zghz3506);
            CONVERT_OF(sail_int, mach_int)(&zghz3506, zgaz3280);
            RECREATE(sail_int)(&zghz3507);
            CONVERT_OF(sail_int, mach_int)(&zghz3507, zgaz3281);
            RECREATE(lbits)(&zghz3508);
            vector_subrange_lbits(&zghz3508, zghz3378, zghz3506, zghz3507);
            zB_ie = CONVERT_OF(fbits, lbits)(zghz3508, true);
          }
          goto finish_match_257;
        }
      case_258: ;
        sail_match_failure("CapSetBounds");
      finish_match_257: ;
      }
      uint64_t zT_ie;
      {
        __label__ case_260, finish_match_259;

        {
          RECREATE(sail_int)(&zghz3484);
          COPY(sail_int)(&zghz3484, zghz3377);
          {
            bool zgaz3268;
            {
              bool zgaz3267;
              {
                RECREATE(sail_int)(&zghz3501);
                {
                  RECREATE(sail_int)(&zghz3503);
                  sailgen___id(&zghz3503, zghz3484);
                  {
                    RECREATE(sail_int)(&zghz3504);
                    CONVERT_OF(sail_int, mach_int)(&zghz3504, INT64_C(3));
                    add_int(&zghz3501, zghz3503, zghz3504);
                  }
                }
                {
                  RECREATE(sail_int)(&zghz3502);
                  CONVERT_OF(sail_int, mach_int)(&zghz3502, INT64_C(0));
                  zgaz3267 = lteq(zghz3502, zghz3501);
                }
              }
              bool zgsz3144;
              if (zgaz3267) {
                RECREATE(sail_int)(&zghz3495);
                {
                  RECREATE(sail_int)(&zghz3497);
                  {
                    RECREATE(sail_int)(&zghz3499);
                    sailgen___id(&zghz3499, zghz3484);
                    {
                      RECREATE(sail_int)(&zghz3500);
                      CONVERT_OF(sail_int, mach_int)(&zghz3500, INT64_C(16));
                      add_int(&zghz3497, zghz3499, zghz3500);
                    }
                  }
                  {
                    RECREATE(sail_int)(&zghz3498);
                    CONVERT_OF(sail_int, mach_int)(&zghz3498, INT64_C(1));
                    sub_int(&zghz3495, zghz3497, zghz3498);
                  }
                }
                {
                  RECREATE(sail_int)(&zghz3496);
                  CONVERT_OF(sail_int, mach_int)(&zghz3496, INT64_C(66));
                  zgsz3144 = lt(zghz3495, zghz3496);
                }
              } else {  zgsz3144 = false;  }
              zgaz3268 = zgsz3144;
            }
            unit zgsz3145;
            zgsz3145 = sail_assert(zgaz3268, "v8_base.sail:45103.69-45103.70");
          }
          int64_t zgaz3270;
          {
            int64_t zgaz3269;
            {
              RECREATE(sail_int)(&zghz3493);
              CONVERT_OF(sail_int, mach_int)(&zghz3493, zCAP_MW);
              RECREATE(sail_int)(&zghz3494);
              add_int(&zghz3494, zghz3484, zghz3493);
              zgaz3269 = CONVERT_OF(mach_int, sail_int)(zghz3494);
            }
            {
              RECREATE(sail_int)(&zghz3490);
              CONVERT_OF(sail_int, mach_int)(&zghz3490, zgaz3269);
              RECREATE(sail_int)(&zghz3491);
              CONVERT_OF(sail_int, mach_int)(&zghz3491, INT64_C(1));
              RECREATE(sail_int)(&zghz3492);
              sub_int(&zghz3492, zghz3490, zghz3491);
              zgaz3270 = CONVERT_OF(mach_int, sail_int)(zghz3492);
            }
          }
          int64_t zgaz3271;
          {
            RECREATE(sail_int)(&zghz3488);
            CONVERT_OF(sail_int, mach_int)(&zghz3488, INT64_C(3));
            RECREATE(sail_int)(&zghz3489);
            add_int(&zghz3489, zghz3484, zghz3488);
            zgaz3271 = CONVERT_OF(mach_int, sail_int)(zghz3489);
          }
          {
            RECREATE(sail_int)(&zghz3485);
            CONVERT_OF(sail_int, mach_int)(&zghz3485, zgaz3270);
            RECREATE(sail_int)(&zghz3486);
            CONVERT_OF(sail_int, mach_int)(&zghz3486, zgaz3271);
            RECREATE(lbits)(&zghz3487);
            vector_subrange_lbits(&zghz3487, zghz3379, zghz3485, zghz3486);
            zT_ie = CONVERT_OF(fbits, lbits)(zghz3487, true);
          }
          goto finish_match_259;
        }
      case_260: ;
        sail_match_failure("CapSetBounds");
      finish_match_259: ;
      }
      RECREATE(lbits)(&zghz3433);
      {
        __label__ case_262, finish_match_261;

        {
          RECREATE(sail_int)(&zghz3470);
          COPY(sail_int)(&zghz3470, zghz3377);
          {
            bool zgaz3255;
            {
              RECREATE(sail_int)(&zghz3480);
              {
                RECREATE(sail_int)(&zghz3482);
                sailgen___id(&zghz3482, zghz3470);
                {
                  RECREATE(sail_int)(&zghz3483);
                  CONVERT_OF(sail_int, mach_int)(&zghz3483, INT64_C(3));
                  add_int(&zghz3480, zghz3482, zghz3483);
                }
              }
              {
                RECREATE(sail_int)(&zghz3481);
                CONVERT_OF(sail_int, mach_int)(&zghz3481, INT64_C(0));
                zgaz3255 = gteq(zghz3480, zghz3481);
              }
            }
            unit zgsz3149;
            zgsz3149 = sail_assert(zgaz3255, "v8_base.sail:45108.46-45108.47");
          }
          {
            bool zgaz3258;
            {
              RECREATE(sail_int)(&zghz3476);
              {
                RECREATE(sail_int)(&zghz3478);
                sailgen___id(&zghz3478, zghz3470);
                {
                  RECREATE(sail_int)(&zghz3479);
                  CONVERT_OF(sail_int, mach_int)(&zghz3479, INT64_C(3));
                  add_int(&zghz3476, zghz3478, zghz3479);
                }
              }
              {
                RECREATE(sail_int)(&zghz3477);
                CONVERT_OF(sail_int, mach_int)(&zghz3477, INT64_C(66));
                zgaz3258 = gteq(zghz3477, zghz3476);
              }
            }
            unit zgsz3148;
            zgsz3148 = sail_assert(zgaz3258, "v8_base.sail:45109.47-45109.48");
          }
          RECREATE(lbits)(&zghz3471);
          {
            int64_t zgaz3259;
            {
              RECREATE(sail_int)(&zghz3474);
              CONVERT_OF(sail_int, mach_int)(&zghz3474, INT64_C(3));
              RECREATE(sail_int)(&zghz3475);
              add_int(&zghz3475, zghz3470, zghz3474);
              zgaz3259 = CONVERT_OF(mach_int, sail_int)(zghz3475);
            }
            {
              RECREATE(sail_int)(&zghz3473);
              CONVERT_OF(sail_int, mach_int)(&zghz3473, zgaz3259);
              sailgen_Ones(&zghz3471, zghz3473);
            }
          }
          int64_t zgaz3261;
          zgaz3261 = (zCAP_VALUE_NUM_BITS + INT64_C(2));
          {
            RECREATE(sail_int)(&zghz3472);
            CONVERT_OF(sail_int, mach_int)(&zghz3472, zgaz3261);
            zero_extend(&zghz3433, zghz3471, zghz3472);
          }
          goto finish_match_261;
        }
      case_262: ;
        sail_match_failure("CapSetBounds");
      finish_match_261: ;
      }
      {
        RECREATE(lbits)(&zghz3467);
        and_bits(&zghz3467, zghz3378, zghz3433);
        RECREATE(lbits)(&zghz3468);
        {
          int64_t zgaz3230;
          zgaz3230 = (zCAP_VALUE_NUM_BITS + INT64_C(2));
          {
            RECREATE(sail_int)(&zghz3469);
            CONVERT_OF(sail_int, mach_int)(&zghz3469, zgaz3230);
            sailgen_Zeros(&zghz3468, zghz3469);
          }
        }
        zlostBottom = neq_bits(zghz3467, zghz3468);
        unit zgsz3153;
        zgsz3153 = UNIT;
      }
      {
        RECREATE(lbits)(&zghz3464);
        and_bits(&zghz3464, zghz3379, zghz3433);
        RECREATE(lbits)(&zghz3465);
        {
          int64_t zgaz3233;
          zgaz3233 = (zCAP_VALUE_NUM_BITS + INT64_C(2));
          {
            RECREATE(sail_int)(&zghz3466);
            CONVERT_OF(sail_int, mach_int)(&zghz3466, zgaz3233);
            sailgen_Zeros(&zghz3465, zghz3466);
          }
        }
        zlostTop = neq_bits(zghz3464, zghz3465);
        unit zgsz3152;
        zgsz3152 = UNIT;
      }
      {
        unit zgsz3151;
        if (zlostTop) {
          {
            RECREATE(lbits)(&zghz3461);
            CONVERT_OF(lbits, fbits)(&zghz3461, zT_ie, UINT64_C(13) , true);
            RECREATE(sail_int)(&zghz3462);
            CONVERT_OF(sail_int, mach_int)(&zghz3462, INT64_C(1));
            RECREATE(lbits)(&zghz3463);
            add_bits_int(&zghz3463, zghz3461, zghz3462);
            zT_ie = CONVERT_OF(fbits, lbits)(zghz3463, true);
          }
          zgsz3151 = UNIT;
        } else {  zgsz3151 = UNIT;  }
      }
      uint64_t zL_ieshadowz314;
      zL_ieshadowz314 = ((zT_ie - zB_ie) & UINT64_C(0b1111111111111));
      {
        __label__ case_264, finish_match_263;

        unit zgsz3154;
        {
          RECREATE(sail_int)(&zghz3435);
          COPY(sail_int)(&zghz3435, zghz3377);
          bool zgaz3239;
          {
            uint64_t zgaz3238;
            {
              fbits zgaz3237;
              {
                int64_t zgaz3236;
                {
                  RECREATE(sail_int)(&zghz3458);
                  CONVERT_OF(sail_int, mach_int)(&zghz3458, zCAP_MW);
                  RECREATE(sail_int)(&zghz3459);
                  CONVERT_OF(sail_int, mach_int)(&zghz3459, INT64_C(4));
                  RECREATE(sail_int)(&zghz3460);
                  sub_int(&zghz3460, zghz3458, zghz3459);
                  zgaz3236 = CONVERT_OF(mach_int, sail_int)(zghz3460);
                }
                zgaz3237 = (UINT64_C(1) & (zL_ieshadowz314 >> zgaz3236));
              }
              zgaz3238 = UINT64_C(0b0);
              zgaz3238 = update_fbits(zgaz3238, INT64_C(0), zgaz3237);
            }
            zgaz3239 = (zgaz3238 == UINT64_C(0b1));
          }
          if (zgaz3239) {
            {
              zincrementE_name = true;
              unit zgsz3167;
              zgsz3167 = UNIT;
            }
            {
              bool zgsz3158;
              if (zlostBottom) {  zgsz3158 = true;  } else {
                uint64_t zgaz3241;
                {
                  fbits zgaz3240;
                  zgaz3240 = (UINT64_C(1) & (zB_ie >> INT64_C(0)));
                  zgaz3241 = UINT64_C(0b0);
                  zgaz3241 = update_fbits(zgaz3241, INT64_C(0), zgaz3240);
                }
                zgsz3158 = (zgaz3241 == UINT64_C(0b1));
              }
              zlostBottom = zgsz3158;
              unit zgsz3166;
              zgsz3166 = UNIT;
            }
            {
              bool zgsz3160;
              if (zlostTop) {  zgsz3160 = true;  } else {
                uint64_t zgaz3243;
                {
                  fbits zgaz3242;
                  zgaz3242 = (UINT64_C(1) & (zT_ie >> INT64_C(0)));
                  zgaz3243 = UINT64_C(0b0);
                  zgaz3243 = update_fbits(zgaz3243, INT64_C(0), zgaz3242);
                }
                zgsz3160 = (zgaz3243 == UINT64_C(0b1));
              }
              zlostTop = zgsz3160;
              unit zgsz3165;
              zgsz3165 = UNIT;
            }
            {
              bool zgaz3244;
              {
                RECREATE(sail_int)(&zghz3457);
                CONVERT_OF(sail_int, mach_int)(&zghz3457, zCAP_MAX_EXPONENT);
                zgaz3244 = lt(zghz3435, zghz3457);
              }
              unit zgsz3164;
              zgsz3164 = sail_assert(zgaz3244, "v8_base.sail:45124.47-45124.48");
            }
            {
              bool zgaz3247;
              {
                RECREATE(sail_int)(&zghz3453);
                {
                  RECREATE(sail_int)(&zghz3455);
                  sailgen___id(&zghz3455, zghz3435);
                  {
                    RECREATE(sail_int)(&zghz3456);
                    CONVERT_OF(sail_int, mach_int)(&zghz3456, INT64_C(4));
                    add_int(&zghz3453, zghz3455, zghz3456);
                  }
                }
                {
                  RECREATE(sail_int)(&zghz3454);
                  CONVERT_OF(sail_int, mach_int)(&zghz3454, INT64_C(0));
                  zgaz3247 = lteq(zghz3454, zghz3453);
                }
              }
              unit zgsz3163;
              zgsz3163 = sail_assert(zgaz3247, "v8_base.sail:45125.50-45125.51");
            }
            {
              int64_t zgaz3248;
              {
                RECREATE(sail_int)(&zghz3451);
                CONVERT_OF(sail_int, mach_int)(&zghz3451, zCAP_MW);
                RECREATE(sail_int)(&zghz3452);
                add_int(&zghz3452, zghz3435, zghz3451);
                zgaz3248 = CONVERT_OF(mach_int, sail_int)(zghz3452);
              }
              int64_t zgaz3249;
              {
                RECREATE(sail_int)(&zghz3449);
                CONVERT_OF(sail_int, mach_int)(&zghz3449, INT64_C(4));
                RECREATE(sail_int)(&zghz3450);
                add_int(&zghz3450, zghz3435, zghz3449);
                zgaz3249 = CONVERT_OF(mach_int, sail_int)(zghz3450);
              }
              {
                RECREATE(sail_int)(&zghz3446);
                CONVERT_OF(sail_int, mach_int)(&zghz3446, zgaz3248);
                RECREATE(sail_int)(&zghz3447);
                CONVERT_OF(sail_int, mach_int)(&zghz3447, zgaz3249);
                RECREATE(lbits)(&zghz3448);
                vector_subrange_lbits(&zghz3448, zghz3378, zghz3446, zghz3447);
                zB_ie = CONVERT_OF(fbits, lbits)(zghz3448, true);
              }
              unit zgsz3162;
              zgsz3162 = UNIT;
            }
            {
              int64_t zgaz3250;
              {
                RECREATE(sail_int)(&zghz3444);
                CONVERT_OF(sail_int, mach_int)(&zghz3444, zCAP_MW);
                RECREATE(sail_int)(&zghz3445);
                add_int(&zghz3445, zghz3435, zghz3444);
                zgaz3250 = CONVERT_OF(mach_int, sail_int)(zghz3445);
              }
              int64_t zgaz3251;
              {
                RECREATE(sail_int)(&zghz3442);
                CONVERT_OF(sail_int, mach_int)(&zghz3442, INT64_C(4));
                RECREATE(sail_int)(&zghz3443);
                add_int(&zghz3443, zghz3435, zghz3442);
                zgaz3251 = CONVERT_OF(mach_int, sail_int)(zghz3443);
              }
              {
                RECREATE(sail_int)(&zghz3439);
                CONVERT_OF(sail_int, mach_int)(&zghz3439, zgaz3250);
                RECREATE(sail_int)(&zghz3440);
                CONVERT_OF(sail_int, mach_int)(&zghz3440, zgaz3251);
                RECREATE(lbits)(&zghz3441);
                vector_subrange_lbits(&zghz3441, zghz3379, zghz3439, zghz3440);
                zT_ie = CONVERT_OF(fbits, lbits)(zghz3441, true);
              }
              unit zgsz3161;
              zgsz3161 = UNIT;
            }
            if (zlostTop) {
              {
                RECREATE(lbits)(&zghz3436);
                CONVERT_OF(lbits, fbits)(&zghz3436, zT_ie, UINT64_C(13) , true);
                RECREATE(sail_int)(&zghz3437);
                CONVERT_OF(sail_int, mach_int)(&zghz3437, INT64_C(1));
                RECREATE(lbits)(&zghz3438);
                add_bits_int(&zghz3438, zghz3436, zghz3437);
                zT_ie = CONVERT_OF(fbits, lbits)(zghz3438, true);
              }
              zgsz3154 = UNIT;
            } else {  zgsz3154 = UNIT;  }
          } else {  zgsz3154 = UNIT;  }
          goto finish_match_263;
        }
      case_264: ;
        sail_match_failure("CapSetBounds");
      finish_match_263: ;
        unit zgsz3170;
        zgsz3170 = zgsz3154;
      }
      {
        bool zgaz3252;
        zgaz3252 = eq_bool(zincrementE_name, true);
        unit zgsz3169;
        if (zgaz3252) {
          {
            RECREATE(sail_int)(&zghz3434);
            CONVERT_OF(sail_int, mach_int)(&zghz3434, INT64_C(1));
            add_int(&zghz3377, zghz3377, zghz3434);
          }
          zgsz3169 = UNIT;
        } else {  zgsz3169 = UNIT;  }
      }
      {
        zBbits = (zB_ie << 3) | UINT64_C(0b000);
        unit zgsz3168;
        zgsz3168 = UNIT;
      }
      zTBits = (zT_ie << 3) | UINT64_C(0b000);
      zgsz3138 = UNIT;
    } else {  zgsz3138 = UNIT;  }
  }
  RECREATE(lbits)(&zghz3380);
  {
    __label__ cleanup_270, end_cleanup_271;

    RECREATE(sail_int)(&zghz3381);
    COPY(sail_int)(&zghz3381, zghz3377);
    RECREATE(lbits)(&zghz3382);
    COPY(lbits)(&zghz3382, zc);
    {
      __label__ case_268, finish_match_267;

      struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zgaz3282;
      CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3282);
      sailgen_CapGetBounds(&zgaz3282, zc);
      unit zgsz3172;
      {
        RECREATE(lbits)(&zghz3431);
        COPY(lbits)(&zghz3431, zgaz3282.ztup0);
        RECREATE(lbits)(&zghz3432);
        COPY(lbits)(&zghz3432, zgaz3282.ztup1);
        bool ztup__2;
        ztup__2 = zgaz3282.ztup2;
        {
          COPY(lbits)(&zghz3375, zghz3431);
          unit zgsz3174;
          zgsz3174 = UNIT;
        }
        {
          COPY(lbits)(&zghz3376, zghz3432);
          unit zgsz3173;
          zgsz3173 = UNIT;
        }
        zovalid = ztup__2;
        zgsz3172 = UNIT;
        goto finish_match_267;
      }
    case_268: ;
      sail_match_failure("CapSetBounds");
    finish_match_267: ;
      unit zgsz3186;
      zgsz3186 = zgsz3172;
      KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zgaz3282);
    }
    {
      bool zgaz3289;
      {
        bool zgaz3288;
        {
          bool zgaz3287;
          {
            bool zgaz3284;
            {
              RECREATE(lbits)(&zghz3428);
              {
                RECREATE(sail_int)(&zghz3429);
                CONVERT_OF(sail_int, mach_int)(&zghz3429, INT64_C(0));
                RECREATE(sail_int)(&zghz3430);
                CONVERT_OF(sail_int, mach_int)(&zghz3430, zCAP_BOUND_NUM_BITS);
                slice(&zghz3428, zghz3378, zghz3429, zghz3430);
              }
              zgaz3284 = sailgen_CapUnsignedGreaterThanOrEqual(zghz3428, zghz3375);
            }
            zgaz3287 = not(zgaz3284);
          }
          bool zgsz3176;
          if (zgaz3287) {  zgsz3176 = true;  } else {
            bool zgaz3286;
            {
              RECREATE(lbits)(&zghz3425);
              {
                RECREATE(sail_int)(&zghz3426);
                CONVERT_OF(sail_int, mach_int)(&zghz3426, INT64_C(0));
                RECREATE(sail_int)(&zghz3427);
                CONVERT_OF(sail_int, mach_int)(&zghz3427, zCAP_BOUND_NUM_BITS);
                slice(&zghz3425, zghz3379, zghz3426, zghz3427);
              }
              zgaz3286 = sailgen_CapUnsignedLessThanOrEqual(zghz3425, zghz3376);
            }
            zgsz3176 = not(zgaz3286);
          }
          zgaz3288 = zgsz3176;
        }
        bool zgsz3177;
        if (zgaz3288) {  zgsz3177 = true;  } else {  zgsz3177 = not(zovalid);  }
        zgaz3289 = zgsz3177;
      }
      unit zgsz3185;
      if (zgaz3289) {
        fbits zgaz3290;
        zgaz3290 = sailgen_Bit(UINT64_C(0b0));
        {
          RECREATE(sail_int)(&zghz3424);
          CONVERT_OF(sail_int, mach_int)(&zghz3424, zCAP_TAG_BIT);
          update_lbits(&zghz3382, zghz3382, zghz3424, zgaz3290);
        }
        zgsz3185 = UNIT;
      } else {  zgsz3185 = UNIT;  }
    }
    {
      unit zgsz3180;
      if (zie) {
        {
          fbits zgaz3291;
          zgaz3291 = sailgen_Bit(UINT64_C(0b0));
          {
            RECREATE(sail_int)(&zghz3423);
            CONVERT_OF(sail_int, mach_int)(&zghz3423, zCAP_IE_BIT);
            update_lbits(&zghz3382, zghz3382, zghz3423, zgaz3291);
          }
          unit zgsz3184;
          zgsz3184 = UNIT;
        }
        {
          uint64_t zgaz3293;
          {
            uint64_t zgaz3292;
            {
              RECREATE(sail_int)(&zghz3420);
              CONVERT_OF(sail_int, mach_int)(&zghz3420, INT64_C(2));
              RECREATE(sail_int)(&zghz3421);
              CONVERT_OF(sail_int, mach_int)(&zghz3421, INT64_C(0));
              RECREATE(lbits)(&zghz3422);
              sailgen_integer_subrange(&zghz3422, zghz3381, zghz3420, zghz3421);
              zgaz3292 = CONVERT_OF(fbits, lbits)(zghz3422, true);
            }
            zgaz3293 = (~(zgaz3292) & UINT64_C(0b111));
          }
          {
            RECREATE(sail_int)(&zghz3417);
            CONVERT_OF(sail_int, mach_int)(&zghz3417, zCAP_BASE_EXP_HI_BIT);
            RECREATE(sail_int)(&zghz3418);
            CONVERT_OF(sail_int, mach_int)(&zghz3418, zCAP_BASE_LO_BIT);
            RECREATE(lbits)(&zghz3419);
            CONVERT_OF(lbits, fbits)(&zghz3419, zgaz3293, UINT64_C(3) , true);
            vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3417, zghz3418, zghz3419);
          }
          unit zgsz3183;
          zgsz3183 = UNIT;
        }
        uint64_t zgaz3295;
        {
          uint64_t zgaz3294;
          {
            RECREATE(sail_int)(&zghz3414);
            CONVERT_OF(sail_int, mach_int)(&zghz3414, INT64_C(5));
            RECREATE(sail_int)(&zghz3415);
            CONVERT_OF(sail_int, mach_int)(&zghz3415, INT64_C(3));
            RECREATE(lbits)(&zghz3416);
            sailgen_integer_subrange(&zghz3416, zghz3381, zghz3414, zghz3415);
            zgaz3294 = CONVERT_OF(fbits, lbits)(zghz3416, true);
          }
          zgaz3295 = (~(zgaz3294) & UINT64_C(0b111));
        }
        {
          RECREATE(sail_int)(&zghz3411);
          CONVERT_OF(sail_int, mach_int)(&zghz3411, zCAP_LIMIT_EXP_HI_BIT);
          RECREATE(sail_int)(&zghz3412);
          CONVERT_OF(sail_int, mach_int)(&zghz3412, zCAP_LIMIT_LO_BIT);
          RECREATE(lbits)(&zghz3413);
          CONVERT_OF(lbits, fbits)(&zghz3413, zgaz3295, UINT64_C(3) , true);
          vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3411, zghz3412, zghz3413);
        }
        zgsz3180 = UNIT;
      } else {
        {
          fbits zgaz3296;
          zgaz3296 = sailgen_Bit(UINT64_C(0b1));
          {
            RECREATE(sail_int)(&zghz3410);
            CONVERT_OF(sail_int, mach_int)(&zghz3410, zCAP_IE_BIT);
            update_lbits(&zghz3382, zghz3382, zghz3410, zgaz3296);
          }
          unit zgsz3182;
          zgsz3182 = UNIT;
        }
        {
          uint64_t zgaz3297;
          zgaz3297 = (safe_rshift(UINT64_MAX, 64 - 3) & (zBbits >> INT64_C(0)));
          {
            RECREATE(sail_int)(&zghz3407);
            CONVERT_OF(sail_int, mach_int)(&zghz3407, zCAP_BASE_EXP_HI_BIT);
            RECREATE(sail_int)(&zghz3408);
            CONVERT_OF(sail_int, mach_int)(&zghz3408, zCAP_BASE_LO_BIT);
            RECREATE(lbits)(&zghz3409);
            CONVERT_OF(lbits, fbits)(&zghz3409, zgaz3297, UINT64_C(3) , true);
            vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3407, zghz3408, zghz3409);
          }
          unit zgsz3181;
          zgsz3181 = UNIT;
        }
        uint64_t zgaz3298;
        zgaz3298 = (safe_rshift(UINT64_MAX, 64 - 3) & (zTBits >> INT64_C(0)));
        {
          RECREATE(sail_int)(&zghz3404);
          CONVERT_OF(sail_int, mach_int)(&zghz3404, zCAP_LIMIT_EXP_HI_BIT);
          RECREATE(sail_int)(&zghz3405);
          CONVERT_OF(sail_int, mach_int)(&zghz3405, zCAP_LIMIT_LO_BIT);
          RECREATE(lbits)(&zghz3406);
          CONVERT_OF(lbits, fbits)(&zghz3406, zgaz3298, UINT64_C(3) , true);
          vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3404, zghz3405, zghz3406);
        }
        zgsz3180 = UNIT;
      }
    }
    {
      uint64_t zgaz3300;
      {
        int64_t zgaz3299;
        {
          RECREATE(sail_int)(&zghz3401);
          CONVERT_OF(sail_int, mach_int)(&zghz3401, zCAP_MW);
          RECREATE(sail_int)(&zghz3402);
          CONVERT_OF(sail_int, mach_int)(&zghz3402, INT64_C(1));
          RECREATE(sail_int)(&zghz3403);
          sub_int(&zghz3403, zghz3401, zghz3402);
          zgaz3299 = CONVERT_OF(mach_int, sail_int)(zghz3403);
        }
        zgaz3300 = (safe_rshift(UINT64_MAX, 64 - 13) & (zBbits >> INT64_C(3)));
      }
      {
        RECREATE(sail_int)(&zghz3398);
        CONVERT_OF(sail_int, mach_int)(&zghz3398, zCAP_BASE_HI_BIT);
        RECREATE(sail_int)(&zghz3399);
        CONVERT_OF(sail_int, mach_int)(&zghz3399, zCAP_BASE_MANTISSA_LO_BIT);
        RECREATE(lbits)(&zghz3400);
        CONVERT_OF(lbits, fbits)(&zghz3400, zgaz3300, UINT64_C(13) , true);
        vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3398, zghz3399, zghz3400);
      }
      unit zgsz3179;
      zgsz3179 = UNIT;
    }
    {
      uint64_t zgaz3302;
      {
        int64_t zgaz3301;
        {
          RECREATE(sail_int)(&zghz3395);
          CONVERT_OF(sail_int, mach_int)(&zghz3395, zCAP_MW);
          RECREATE(sail_int)(&zghz3396);
          CONVERT_OF(sail_int, mach_int)(&zghz3396, INT64_C(3));
          RECREATE(sail_int)(&zghz3397);
          sub_int(&zghz3397, zghz3395, zghz3396);
          zgaz3301 = CONVERT_OF(mach_int, sail_int)(zghz3397);
        }
        zgaz3302 = (safe_rshift(UINT64_MAX, 64 - 11) & (zTBits >> INT64_C(3)));
      }
      {
        RECREATE(sail_int)(&zghz3392);
        CONVERT_OF(sail_int, mach_int)(&zghz3392, zCAP_LIMIT_HI_BIT);
        RECREATE(sail_int)(&zghz3393);
        CONVERT_OF(sail_int, mach_int)(&zghz3393, zCAP_LIMIT_MANTISSA_LO_BIT);
        RECREATE(lbits)(&zghz3394);
        CONVERT_OF(lbits, fbits)(&zghz3394, zgaz3302, UINT64_C(11) , true);
        vector_update_subrange_lbits(&zghz3382, zghz3382, zghz3392, zghz3393, zghz3394);
      }
      unit zgsz3178;
      zgsz3178 = UNIT;
    }
    bool zfrom_large;
    {
      bool zgaz3312;
      {
        int64_t zgaz3311;
        zgaz3311 = sailgen_CapGetExponent(zc);
        {
          RECREATE(sail_int)(&zghz3391);
          CONVERT_OF(sail_int, mach_int)(&zghz3391, zgaz3311);
          zgaz3312 = sailgen_CapBoundsUsesValue(zghz3391);
        }
      }
      zfrom_large = not(zgaz3312);
    }
    bool zto_small;
    zto_small = sailgen_CapBoundsUsesValue(zghz3381);
    {
      bool zgaz3307;
      {
        bool zgaz3306;
        {    if (zfrom_large) {  zgaz3306 = zto_small;  } else {  zgaz3306 = false;  }
        }
        bool zgsz3188;
        if (zgaz3306) {
          uint64_t zgaz3305;
          {
            uint64_t zgaz3304;
            {
              int64_t zgaz3303;
              {
                RECREATE(sail_int)(&zghz3388);
                CONVERT_OF(sail_int, mach_int)(&zghz3388, zCAP_FLAGS_LO_BIT);
                RECREATE(sail_int)(&zghz3389);
                CONVERT_OF(sail_int, mach_int)(&zghz3389, INT64_C(1));
                RECREATE(sail_int)(&zghz3390);
                sub_int(&zghz3390, zghz3388, zghz3389);
                zgaz3303 = CONVERT_OF(mach_int, sail_int)(zghz3390);
              }
              zgaz3304 = (safe_rshift(UINT64_MAX, 64 - 56) & (zbase >> INT64_C(0)));
            }
            {
              RECREATE(lbits)(&zghz3385);
              CONVERT_OF(lbits, fbits)(&zghz3385, zgaz3304, UINT64_C(56) , true);
              RECREATE(sail_int)(&zghz3386);
              CONVERT_OF(sail_int, mach_int)(&zghz3386, INT64_C(64));
              RECREATE(lbits)(&zghz3387);
              sign_extend(&zghz3387, zghz3385, zghz3386);
              zgaz3305 = CONVERT_OF(fbits, lbits)(zghz3387, true);
            }
          }
          zgsz3188 = (zgaz3305 != zbase);
        } else {  zgsz3188 = false;  }
        zgaz3307 = zgsz3188;
      }
      unit zgsz3192;
      if (zgaz3307) {
        fbits zgaz3308;
        zgaz3308 = sailgen_Bit(UINT64_C(0b0));
        {
          RECREATE(sail_int)(&zghz3384);
          CONVERT_OF(sail_int, mach_int)(&zghz3384, zCAP_TAG_BIT);
          update_lbits(&zghz3382, zghz3382, zghz3384, zgaz3308);
        }
        zgsz3192 = UNIT;
      } else {  zgsz3192 = UNIT;  }
    }
    {
      bool zgaz3309;
      {
        if (zexact) {
          bool zgsz3189;
          if (zlostBottom) {  zgsz3189 = true;  } else {  zgsz3189 = zlostTop;  }
          zgaz3309 = zgsz3189;
        } else {  zgaz3309 = false;  }
      }
      unit zgsz3191;
      if (zgaz3309) {
        fbits zgaz3310;
        zgaz3310 = sailgen_Bit(UINT64_C(0b0));
        {
          RECREATE(sail_int)(&zghz3383);
          CONVERT_OF(sail_int, mach_int)(&zghz3383, zCAP_TAG_BIT);
          update_lbits(&zghz3382, zghz3382, zghz3383, zgaz3310);
        }
        zgsz3191 = UNIT;
      } else {  zgsz3191 = UNIT;  }
    }
    COPY(lbits)((*(&zcbz336)), zghz3382);
    goto cleanup_270;
    /* unreachable after return */
    goto finish_match_265;
    goto end_cleanup_271;
  cleanup_270: ;
    goto end_function_269;
  end_cleanup_271: ;
  }
case_266: ;
  sail_match_failure("CapSetBounds");
finish_match_265: ;
  COPY(lbits)((*(&zcbz336)), zghz3380);










end_function_269: ;
  goto end_function_284;
end_block_exception_272: ;
  goto end_function_284;
end_function_284: ;
}



static void finish_sailgen_CapSetBounds(void)
{
  KILL(lbits)(&zghz3567);
  KILL(sail_int)(&zghz3566);
  KILL(sail_int)(&zghz3565);
  KILL(sail_int)(&zghz3564);
  KILL(sail_int)(&zghz3563);
  KILL(sail_int)(&zghz3562);
  KILL(sail_int)(&zghz3561);
  KILL(sail_int)(&zghz3560);
  KILL(sail_int)(&zghz3559);
  KILL(sail_int)(&zghz3558);
  KILL(sail_int)(&zghz3557);
  KILL(sail_int)(&zghz3556);
  KILL(sail_int)(&zghz3555);
  KILL(lbits)(&zghz3554);
  KILL(sail_int)(&zghz3553);
  KILL(sail_int)(&zghz3552);
  KILL(sail_int)(&zghz3551);
  KILL(lbits)(&zghz3550);
  KILL(sail_int)(&zghz3549);
  KILL(sail_int)(&zghz3548);
  KILL(sail_int)(&zghz3547);
  KILL(sail_int)(&zghz3546);
  KILL(sail_int)(&zghz3545);
  KILL(sail_int)(&zghz3544);
  KILL(sail_int)(&zghz3543);
  KILL(sail_int)(&zghz3542);
  KILL(lbits)(&zghz3541);
  KILL(lbits)(&zghz3540);
  KILL(lbits)(&zghz3539);
  KILL(lbits)(&zghz3538);
  KILL(sail_int)(&zghz3537);
  KILL(sail_int)(&zghz3536);
  KILL(sail_int)(&zghz3535);
  KILL(lbits)(&zghz3534);
  KILL(sail_int)(&zghz3533);
  KILL(sail_int)(&zghz3532);
  KILL(sail_int)(&zghz3531);
  KILL(sail_int)(&zghz3530);
  KILL(sail_int)(&zghz3529);
  KILL(lbits)(&zghz3528);
  KILL(sail_int)(&zghz3527);
  KILL(sail_int)(&zghz3526);
  KILL(sail_int)(&zghz3525);
  KILL(sail_int)(&zghz3524);
  KILL(sail_int)(&zghz3523);
  KILL(sail_int)(&zghz3522);
  KILL(sail_int)(&zghz3521);
  KILL(sail_int)(&zghz3520);
  KILL(sail_int)(&zghz3519);
  KILL(sail_int)(&zghz3518);
  KILL(sail_int)(&zghz3517);
  KILL(sail_int)(&zghz3516);
  KILL(sail_int)(&zghz3515);
  KILL(sail_int)(&zghz3514);
  KILL(sail_int)(&zghz3513);
  KILL(sail_int)(&zghz3512);
  KILL(sail_int)(&zghz3511);
  KILL(sail_int)(&zghz3510);
  KILL(sail_int)(&zghz3509);
  KILL(lbits)(&zghz3508);
  KILL(sail_int)(&zghz3507);
  KILL(sail_int)(&zghz3506);
  KILL(sail_int)(&zghz3505);
  KILL(sail_int)(&zghz3504);
  KILL(sail_int)(&zghz3503);
  KILL(sail_int)(&zghz3502);
  KILL(sail_int)(&zghz3501);
  KILL(sail_int)(&zghz3500);
  KILL(sail_int)(&zghz3499);
  KILL(sail_int)(&zghz3498);
  KILL(sail_int)(&zghz3497);
  KILL(sail_int)(&zghz3496);
  KILL(sail_int)(&zghz3495);
  KILL(sail_int)(&zghz3494);
  KILL(sail_int)(&zghz3493);
  KILL(sail_int)(&zghz3492);
  KILL(sail_int)(&zghz3491);
  KILL(sail_int)(&zghz3490);
  KILL(sail_int)(&zghz3489);
  KILL(sail_int)(&zghz3488);
  KILL(lbits)(&zghz3487);
  KILL(sail_int)(&zghz3486);
  KILL(sail_int)(&zghz3485);
  KILL(sail_int)(&zghz3484);
  KILL(sail_int)(&zghz3483);
  KILL(sail_int)(&zghz3482);
  KILL(sail_int)(&zghz3481);
  KILL(sail_int)(&zghz3480);
  KILL(sail_int)(&zghz3479);
  KILL(sail_int)(&zghz3478);
  KILL(sail_int)(&zghz3477);
  KILL(sail_int)(&zghz3476);
  KILL(sail_int)(&zghz3475);
  KILL(sail_int)(&zghz3474);
  KILL(sail_int)(&zghz3473);
  KILL(sail_int)(&zghz3472);
  KILL(lbits)(&zghz3471);
  KILL(sail_int)(&zghz3470);
  KILL(sail_int)(&zghz3469);
  KILL(lbits)(&zghz3468);
  KILL(lbits)(&zghz3467);
  KILL(sail_int)(&zghz3466);
  KILL(lbits)(&zghz3465);
  KILL(lbits)(&zghz3464);
  KILL(lbits)(&zghz3463);
  KILL(sail_int)(&zghz3462);
  KILL(lbits)(&zghz3461);
  KILL(sail_int)(&zghz3460);
  KILL(sail_int)(&zghz3459);
  KILL(sail_int)(&zghz3458);
  KILL(sail_int)(&zghz3457);
  KILL(sail_int)(&zghz3456);
  KILL(sail_int)(&zghz3455);
  KILL(sail_int)(&zghz3454);
  KILL(sail_int)(&zghz3453);
  KILL(sail_int)(&zghz3452);
  KILL(sail_int)(&zghz3451);
  KILL(sail_int)(&zghz3450);
  KILL(sail_int)(&zghz3449);
  KILL(lbits)(&zghz3448);
  KILL(sail_int)(&zghz3447);
  KILL(sail_int)(&zghz3446);
  KILL(sail_int)(&zghz3445);
  KILL(sail_int)(&zghz3444);
  KILL(sail_int)(&zghz3443);
  KILL(sail_int)(&zghz3442);
  KILL(lbits)(&zghz3441);
  KILL(sail_int)(&zghz3440);
  KILL(sail_int)(&zghz3439);
  KILL(lbits)(&zghz3438);
  KILL(sail_int)(&zghz3437);
  KILL(lbits)(&zghz3436);
  KILL(sail_int)(&zghz3435);
  KILL(sail_int)(&zghz3434);
  KILL(lbits)(&zghz3433);
  KILL(lbits)(&zghz3432);
  KILL(lbits)(&zghz3431);
  KILL(sail_int)(&zghz3430);
  KILL(sail_int)(&zghz3429);
  KILL(lbits)(&zghz3428);
  KILL(sail_int)(&zghz3427);
  KILL(sail_int)(&zghz3426);
  KILL(lbits)(&zghz3425);
  KILL(sail_int)(&zghz3424);
  KILL(sail_int)(&zghz3423);
  KILL(lbits)(&zghz3422);
  KILL(sail_int)(&zghz3421);
  KILL(sail_int)(&zghz3420);
  KILL(lbits)(&zghz3419);
  KILL(sail_int)(&zghz3418);
  KILL(sail_int)(&zghz3417);
  KILL(lbits)(&zghz3416);
  KILL(sail_int)(&zghz3415);
  KILL(sail_int)(&zghz3414);
  KILL(lbits)(&zghz3413);
  KILL(sail_int)(&zghz3412);
  KILL(sail_int)(&zghz3411);
  KILL(sail_int)(&zghz3410);
  KILL(lbits)(&zghz3409);
  KILL(sail_int)(&zghz3408);
  KILL(sail_int)(&zghz3407);
  KILL(lbits)(&zghz3406);
  KILL(sail_int)(&zghz3405);
  KILL(sail_int)(&zghz3404);
  KILL(sail_int)(&zghz3403);
  KILL(sail_int)(&zghz3402);
  KILL(sail_int)(&zghz3401);
  KILL(lbits)(&zghz3400);
  KILL(sail_int)(&zghz3399);
  KILL(sail_int)(&zghz3398);
  KILL(sail_int)(&zghz3397);
  KILL(sail_int)(&zghz3396);
  KILL(sail_int)(&zghz3395);
  KILL(lbits)(&zghz3394);
  KILL(sail_int)(&zghz3393);
  KILL(sail_int)(&zghz3392);
  KILL(sail_int)(&zghz3391);
  KILL(sail_int)(&zghz3390);
  KILL(sail_int)(&zghz3389);
  KILL(sail_int)(&zghz3388);
  KILL(lbits)(&zghz3387);
  KILL(sail_int)(&zghz3386);
  KILL(lbits)(&zghz3385);
  KILL(sail_int)(&zghz3384);
  KILL(sail_int)(&zghz3383);
  KILL(lbits)(&zghz3382);
  KILL(sail_int)(&zghz3381);
  KILL(lbits)(&zghz3380);
  KILL(lbits)(&zghz3379);
  KILL(lbits)(&zghz3378);
  KILL(sail_int)(&zghz3377);
  KILL(lbits)(&zghz3376);
  KILL(lbits)(&zghz3375);
}

static lbits zCMAX;
static void create_letbind_70(void) {    CREATE(lbits)(&zCMAX);


  lbits zgsz3195;
  CREATE(lbits)(&zgsz3195);
  lbits zgsz3194;
  CREATE(lbits)(&zgsz3194);
  CONVERT_OF(lbits, fbits)(&zgsz3194, UINT64_C(0b1), UINT64_C(1) , true);
  append_64(&zgsz3194, zgsz3194, UINT64_C(0xFFFFC00000010005));
  append_64(&zgsz3194, zgsz3194, UINT64_C(0x0000000000000000));
  COPY(lbits)(&zgsz3195, zgsz3194);
  KILL(lbits)(&zgsz3194);
  COPY(lbits)(&zCMAX, zgsz3195);
  KILL(lbits)(&zgsz3195);
let_end_273: ;
}
static void kill_letbind_70(void) {    KILL(lbits)(&zCMAX);
}

// register TestCap
static lbits zTestCap;

static void sailgen_doCSetBounds(lbits *rop, lbits, lbits);

lbits zghz3568;

static void startup_sailgen_doCSetBounds(void)
{    CREATE(lbits)(&zghz3568);
}

static void sailgen_doCSetBounds(lbits *zcbz337, lbits zinput, lbits zreq_len)
{
  __label__ end_function_275, end_block_exception_276, end_function_283;

  RECREATE(lbits)(&zghz3568);
  sailgen_CapSetBounds(&zghz3568, zinput, zreq_len, false);
  bool zgaz3331;
  zgaz3331 = sailgen_CapIsSealed(zinput);
  if (zgaz3331) {  sailgen_CapWithTagClear((*(&zcbz337)), zghz3568);  } else {  COPY(lbits)((*(&zcbz337)), zghz3568);  }

end_function_275: ;
  goto end_function_283;
end_block_exception_276: ;
  goto end_function_283;
end_function_283: ;
}



static void finish_sailgen_doCSetBounds(void)
{    KILL(lbits)(&zghz3568);
}

static unit sailgen_main(unit);

lbits zghz3569;
lbits zghz3570;
sail_int zghz3571;
lbits zghz3572;
lbits zghz3573;
sail_int zghz3574;
sail_int zghz3575;

static void startup_sailgen_main(void)
{
  CREATE(lbits)(&zghz3569);
  CREATE(lbits)(&zghz3570);
  CREATE(sail_int)(&zghz3571);
  CREATE(lbits)(&zghz3572);
  CREATE(lbits)(&zghz3573);
  CREATE(sail_int)(&zghz3574);
  CREATE(sail_int)(&zghz3575);
}

static unit sailgen_main(unit zgsz3196)
{
  __label__ end_function_278, end_block_exception_279;

  unit zcbz338;
  {
    {
      RECREATE(sail_int)(&zghz3575);
      CONVERT_OF(sail_int, mach_int)(&zghz3575, INT64_C(129));
      sailgen_Zeros(&zTestCap, zghz3575);
    }
    unit zgsz3197;
    zgsz3197 = UNIT;
  }
  RECREATE(lbits)(&zghz3569);
  COPY(lbits)(&zghz3569, zCMAX);
  uint64_t zbits1;
  zbits1 = sailgen_CapGetValue(zTestCap);
  uint64_t zbits2;
  zbits2 = sailgen_CapGetPermissions(zTestCap);
  uint64_t zbits3;
  zbits3 = sailgen_CapGetObjectType(zTestCap);
  uint64_t zbits4;
  zbits4 = sailgen_CapGetTag(zTestCap);
  RECREATE(lbits)(&zghz3570);
  {
    RECREATE(lbits)(&zghz3573);
    {
      RECREATE(sail_int)(&zghz3574);
      CONVERT_OF(sail_int, mach_int)(&zghz3574, INT64_C(65));
      sailgen_Zeros(&zghz3573, zghz3574);
    }
    sailgen_doCSetBounds(&zghz3570, zTestCap, zghz3573);
  }
  struct ztuple_z8z5bvzCz0z5bvzCz0z5boolz9 zbits6;
  CREATE(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zbits6);
  sailgen_CapGetBounds(&zbits6, zTestCap);
  int64_t zexp;
  zexp = sailgen_CapGetExponent(zTestCap);
  bool zrep;
  {
    uint64_t zgaz3332;
    {
      RECREATE(sail_int)(&zghz3571);
      CONVERT_OF(sail_int, mach_int)(&zghz3571, INT64_C(64));
      RECREATE(lbits)(&zghz3572);
      sailgen_Zeros(&zghz3572, zghz3571);
      zgaz3332 = CONVERT_OF(fbits, lbits)(zghz3572, true);
    }
    zrep = sailgen_CapIsRepresentableFast(zTestCap, zgaz3332);
  }
  zcbz338 = UNIT;


  KILL(ztuple_z8z5bvzCz0z5bvzCz0z5boolz9)(&zbits6);




end_function_278: ;
  return zcbz338;
end_block_exception_279: ;

  return UNIT;
}



static void finish_sailgen_main(void)
{
  KILL(sail_int)(&zghz3575);
  KILL(sail_int)(&zghz3574);
  KILL(lbits)(&zghz3573);
  KILL(lbits)(&zghz3572);
  KILL(sail_int)(&zghz3571);
  KILL(lbits)(&zghz3570);
  KILL(lbits)(&zghz3569);
}

static unit sailgen_initializze_registers(unit);

sail_int zghz3576;
sail_int zghz3577;
sail_int zghz3578;
lbits zghz3579;
sail_int zghz3580;
lbits zghz3581;
sail_int zghz3582;
lbits zghz3583;
lbits zghz3584;
sail_int zghz3585;
sail_int zghz3586;
sail_int zghz3587;
lbits zghz3588;
sail_int zghz3589;
lbits zghz3590;
sail_int zghz3591;
lbits zghz3592;
sail_int zghz3593;
lbits zghz3594;
sail_int zghz3595;
lbits zghz3596;
sail_int zghz3597;
lbits zghz3598;
sail_int zghz3599;
lbits zghz3600;
sail_int zghz3601;
lbits zghz3602;
sail_int zghz3603;
lbits zghz3604;
sail_int zghz3605;
lbits zghz3606;
sail_int zghz3607;
lbits zghz3608;
sail_int zghz3609;
lbits zghz3610;
sail_int zghz3611;
lbits zghz3612;
sail_int zghz3613;
sail_int zghz3614;
sail_int zghz3615;
sail_int zghz3616;
sail_int zghz3617;
sail_int zghz3618;
lbits zghz3619;
sail_int zghz3620;
lbits zghz3621;
sail_int zghz3622;
lbits zghz3623;
sail_int zghz3624;
lbits zghz3625;
sail_int zghz3626;
lbits zghz3627;
sail_int zghz3628;
lbits zghz3629;
sail_int zghz3630;
lbits zghz3631;
sail_int zghz3632;
lbits zghz3633;
sail_int zghz3634;
sail_int zghz3635;
sail_int zghz3636;
sail_int zghz3637;
sail_int zghz3638;
sail_int zghz3639;
sail_int zghz3640;
sail_int zghz3641;
sail_int zghz3642;
lbits zghz3643;
sail_int zghz3644;
lbits zghz3645;
sail_int zghz3646;
lbits zghz3647;
sail_int zghz3648;
lbits zghz3649;
sail_int zghz3650;
lbits zghz3651;
sail_int zghz3652;
lbits zghz3653;
sail_int zghz3654;
lbits zghz3655;
sail_int zghz3656;
lbits zghz3657;
sail_int zghz3658;
sail_int zghz3659;
lbits zghz3660;
sail_int zghz3661;
lbits zghz3662;
sail_int zghz3663;
lbits zghz3664;
sail_int zghz3665;
lbits zghz3666;
sail_int zghz3667;
sail_int zghz3668;
sail_int zghz3669;
lbits zghz3670;
sail_int zghz3671;
lbits zghz3672;
sail_int zghz3673;
lbits zghz3674;
sail_int zghz3675;
lbits zghz3676;
sail_int zghz3677;
lbits zghz3678;
sail_int zghz3679;
lbits zghz3680;
sail_int zghz3681;
lbits zghz3682;
sail_int zghz3683;
lbits zghz3684;
sail_int zghz3685;
lbits zghz3686;
sail_int zghz3687;
lbits zghz3688;
sail_int zghz3689;
lbits zghz3690;
sail_int zghz3691;
lbits zghz3692;
sail_int zghz3693;
lbits zghz3694;
sail_int zghz3695;
lbits zghz3696;
sail_int zghz3697;
lbits zghz3698;
sail_int zghz3699;
lbits zghz3700;
sail_int zghz3701;
lbits zghz3702;
sail_int zghz3703;
lbits zghz3704;
sail_int zghz3705;
lbits zghz3706;
sail_int zghz3707;
sail_int zghz3708;
lbits zghz3709;
sail_int zghz3710;
sail_int zghz3711;
lbits zghz3712;
sail_int zghz3713;
lbits zghz3714;
sail_int zghz3715;
lbits zghz3716;
sail_int zghz3717;
lbits zghz3718;
sail_int zghz3719;
lbits zghz3720;
sail_int zghz3721;
lbits zghz3722;
sail_int zghz3723;
lbits zghz3724;
sail_int zghz3725;
lbits zghz3726;
sail_int zghz3727;
lbits zghz3728;
sail_int zghz3729;
lbits zghz3730;
sail_int zghz3731;
lbits zghz3732;
sail_int zghz3733;
lbits zghz3734;
sail_int zghz3735;
lbits zghz3736;
sail_int zghz3737;
lbits zghz3738;
sail_int zghz3739;
lbits zghz3740;
sail_int zghz3741;
lbits zghz3742;
sail_int zghz3743;
lbits zghz3744;
sail_int zghz3745;
lbits zghz3746;
sail_int zghz3747;
lbits zghz3748;
sail_int zghz3749;
lbits zghz3750;
sail_int zghz3751;
lbits zghz3752;
sail_int zghz3753;
lbits zghz3754;
sail_int zghz3755;
lbits zghz3756;
sail_int zghz3757;
lbits zghz3758;
sail_int zghz3759;
lbits zghz3760;
sail_int zghz3761;
lbits zghz3762;
sail_int zghz3763;
lbits zghz3764;
sail_int zghz3765;
lbits zghz3766;
sail_int zghz3767;
lbits zghz3768;
sail_int zghz3769;
lbits zghz3770;
sail_int zghz3771;
lbits zghz3772;
sail_int zghz3773;
lbits zghz3774;
sail_int zghz3775;
lbits zghz3776;
sail_int zghz3777;
lbits zghz3778;
sail_int zghz3779;
lbits zghz3780;
sail_int zghz3781;
lbits zghz3782;
sail_int zghz3783;
lbits zghz3784;
sail_int zghz3785;
lbits zghz3786;
sail_int zghz3787;
lbits zghz3788;
sail_int zghz3789;
lbits zghz3790;
sail_int zghz3791;
lbits zghz3792;
sail_int zghz3793;
lbits zghz3794;
sail_int zghz3795;
lbits zghz3796;
sail_int zghz3797;
lbits zghz3798;
sail_int zghz3799;
lbits zghz3800;
sail_int zghz3801;
lbits zghz3802;
sail_int zghz3803;
lbits zghz3804;
sail_int zghz3805;
lbits zghz3806;
sail_int zghz3807;
lbits zghz3808;
sail_int zghz3809;
lbits zghz3810;
sail_int zghz3811;
lbits zghz3812;
sail_int zghz3813;
lbits zghz3814;
sail_int zghz3815;
lbits zghz3816;
sail_int zghz3817;
lbits zghz3818;
sail_int zghz3819;
lbits zghz3820;
sail_int zghz3821;
lbits zghz3822;
sail_int zghz3823;
lbits zghz3824;
sail_int zghz3825;
lbits zghz3826;
sail_int zghz3827;
lbits zghz3828;
sail_int zghz3829;
lbits zghz3830;
sail_int zghz3831;
lbits zghz3832;
sail_int zghz3833;
lbits zghz3834;
sail_int zghz3835;
lbits zghz3836;
sail_int zghz3837;
lbits zghz3838;
sail_int zghz3839;
lbits zghz3840;
sail_int zghz3841;
lbits zghz3842;
sail_int zghz3843;
lbits zghz3844;
sail_int zghz3845;
lbits zghz3846;
sail_int zghz3847;
lbits zghz3848;
sail_int zghz3849;
lbits zghz3850;
sail_int zghz3851;
lbits zghz3852;
sail_int zghz3853;
lbits zghz3854;
sail_int zghz3855;
lbits zghz3856;
sail_int zghz3857;
lbits zghz3858;
sail_int zghz3859;
lbits zghz3860;
sail_int zghz3861;
lbits zghz3862;
sail_int zghz3863;
lbits zghz3864;
sail_int zghz3865;
lbits zghz3866;
sail_int zghz3867;
lbits zghz3868;
sail_int zghz3869;
lbits zghz3870;
sail_int zghz3871;
lbits zghz3872;
sail_int zghz3873;
lbits zghz3874;
sail_int zghz3875;
lbits zghz3876;
sail_int zghz3877;
lbits zghz3878;
sail_int zghz3879;
lbits zghz3880;
sail_int zghz3881;
sail_int zghz3882;
lbits zghz3883;
sail_int zghz3884;
lbits zghz3885;
sail_int zghz3886;
lbits zghz3887;
sail_int zghz3888;
sail_int zghz3889;
lbits zghz3890;
sail_int zghz3891;
sail_int zghz3892;
lbits zghz3893;
sail_int zghz3894;
lbits zghz3895;
sail_int zghz3896;
lbits zghz3897;
sail_int zghz3898;
lbits zghz3899;
sail_int zghz3900;
lbits zghz3901;
sail_int zghz3902;
lbits zghz3903;
sail_int zghz3904;
lbits zghz3905;
sail_int zghz3906;
lbits zghz3907;
sail_int zghz3908;
lbits zghz3909;
sail_int zghz3910;
lbits zghz3911;
sail_int zghz3912;
lbits zghz3913;
sail_int zghz3914;
lbits zghz3915;
sail_int zghz3916;
lbits zghz3917;
sail_int zghz3918;
lbits zghz3919;
sail_int zghz3920;
lbits zghz3921;
sail_int zghz3922;
lbits zghz3923;
sail_int zghz3924;
lbits zghz3925;
sail_int zghz3926;
lbits zghz3927;
sail_int zghz3928;
lbits zghz3929;
sail_int zghz3930;
lbits zghz3931;
sail_int zghz3932;
lbits zghz3933;
sail_int zghz3934;
lbits zghz3935;
sail_int zghz3936;
lbits zghz3937;
sail_int zghz3938;
lbits zghz3939;
sail_int zghz3940;
lbits zghz3941;
sail_int zghz3942;
lbits zghz3943;
sail_int zghz3944;
lbits zghz3945;
sail_int zghz3946;
lbits zghz3947;
sail_int zghz3948;
sail_int zghz3949;
lbits zghz3950;
sail_int zghz3951;
sail_int zghz3952;
lbits zghz3953;
sail_int zghz3954;
sail_int zghz3955;
lbits zghz3956;
sail_int zghz3957;
sail_int zghz3958;
lbits zghz3959;
sail_int zghz3960;
sail_int zghz3961;
lbits zghz3962;
sail_int zghz3963;
lbits zghz3964;
sail_int zghz3965;
lbits zghz3966;
sail_int zghz3967;
lbits zghz3968;
sail_int zghz3969;
lbits zghz3970;
sail_int zghz3971;
lbits zghz3972;
sail_int zghz3973;
sail_int zghz3974;
lbits zghz3975;
sail_int zghz3976;
lbits zghz3977;
sail_int zghz3978;
lbits zghz3979;
sail_int zghz3980;
lbits zghz3981;
sail_int zghz3982;
lbits zghz3983;
sail_int zghz3984;
lbits zghz3985;
sail_int zghz3986;
lbits zghz3987;
sail_int zghz3988;
lbits zghz3989;
sail_int zghz3990;
lbits zghz3991;
sail_int zghz3992;
lbits zghz3993;
sail_int zghz3994;
lbits zghz3995;
sail_int zghz3996;
lbits zghz3997;
sail_int zghz3998;
lbits zghz3999;
sail_int zghz31000;
lbits zghz31001;
sail_int zghz31002;
lbits zghz31003;
sail_int zghz31004;
lbits zghz31005;
sail_int zghz31006;
lbits zghz31007;
sail_int zghz31008;
sail_int zghz31009;
lbits zghz31010;
sail_int zghz31011;
sail_int zghz31012;
lbits zghz31013;
sail_int zghz31014;
lbits zghz31015;
sail_int zghz31016;
sail_int zghz31017;
lbits zghz31018;
sail_int zghz31019;
lbits zghz31020;
sail_int zghz31021;
lbits zghz31022;
sail_int zghz31023;
lbits zghz31024;
sail_int zghz31025;
sail_int zghz31026;
lbits zghz31027;
sail_int zghz31028;
lbits zghz31029;
sail_int zghz31030;
lbits zghz31031;
sail_int zghz31032;
lbits zghz31033;
sail_int zghz31034;
lbits zghz31035;
sail_int zghz31036;
sail_int zghz31037;
sail_int zghz31038;
lbits zghz31039;
sail_int zghz31040;
lbits zghz31041;
sail_int zghz31042;
lbits zghz31043;
sail_int zghz31044;
lbits zghz31045;
sail_int zghz31046;
lbits zghz31047;
sail_int zghz31048;
lbits zghz31049;
sail_int zghz31050;
lbits zghz31051;
sail_int zghz31052;
lbits zghz31053;
sail_int zghz31054;
lbits zghz31055;
sail_int zghz31056;
lbits zghz31057;
sail_int zghz31058;
lbits zghz31059;
sail_int zghz31060;
lbits zghz31061;
sail_int zghz31062;
lbits zghz31063;
sail_int zghz31064;
lbits zghz31065;
sail_int zghz31066;
lbits zghz31067;
sail_int zghz31068;
lbits zghz31069;
sail_int zghz31070;
lbits zghz31071;
sail_int zghz31072;
lbits zghz31073;
sail_int zghz31074;
lbits zghz31075;
sail_int zghz31076;
lbits zghz31077;
sail_int zghz31078;
lbits zghz31079;
sail_int zghz31080;
lbits zghz31081;
sail_int zghz31082;
lbits zghz31083;
sail_int zghz31084;
lbits zghz31085;
sail_int zghz31086;
lbits zghz31087;
sail_int zghz31088;
lbits zghz31089;
sail_int zghz31090;
lbits zghz31091;
sail_int zghz31092;
lbits zghz31093;
sail_int zghz31094;
lbits zghz31095;
sail_int zghz31096;
lbits zghz31097;
sail_int zghz31098;
lbits zghz31099;
sail_int zghz31100;
lbits zghz31101;
sail_int zghz31102;
lbits zghz31103;
sail_int zghz31104;
lbits zghz31105;
sail_int zghz31106;
lbits zghz31107;
sail_int zghz31108;
lbits zghz31109;
sail_int zghz31110;
lbits zghz31111;
sail_int zghz31112;
lbits zghz31113;
sail_int zghz31114;
lbits zghz31115;
sail_int zghz31116;
lbits zghz31117;
sail_int zghz31118;
lbits zghz31119;
sail_int zghz31120;
lbits zghz31121;
sail_int zghz31122;
lbits zghz31123;
sail_int zghz31124;
lbits zghz31125;
sail_int zghz31126;
lbits zghz31127;
sail_int zghz31128;
lbits zghz31129;
sail_int zghz31130;
lbits zghz31131;
sail_int zghz31132;
lbits zghz31133;
sail_int zghz31134;
lbits zghz31135;
sail_int zghz31136;
lbits zghz31137;
sail_int zghz31138;
lbits zghz31139;
sail_int zghz31140;
lbits zghz31141;
sail_int zghz31142;
lbits zghz31143;
sail_int zghz31144;
lbits zghz31145;
sail_int zghz31146;
lbits zghz31147;
sail_int zghz31148;
lbits zghz31149;
sail_int zghz31150;
lbits zghz31151;
sail_int zghz31152;
sail_int zghz31153;
sail_int zghz31154;
sail_int zghz31155;
sail_int zghz31156;
sail_int zghz31157;
sail_int zghz31158;
lbits zghz31159;
sail_int zghz31160;
lbits zghz31161;
sail_int zghz31162;
lbits zghz31163;
sail_int zghz31164;
lbits zghz31165;
sail_int zghz31166;
lbits zghz31167;
sail_int zghz31168;
lbits zghz31169;
sail_int zghz31170;
lbits zghz31171;
sail_int zghz31172;
lbits zghz31173;
sail_int zghz31174;
lbits zghz31175;
sail_int zghz31176;
lbits zghz31177;
sail_int zghz31178;
lbits zghz31179;
sail_int zghz31180;
lbits zghz31181;
sail_int zghz31182;
lbits zghz31183;
sail_int zghz31184;
lbits zghz31185;
sail_int zghz31186;
lbits zghz31187;
sail_int zghz31188;
sail_int zghz31189;
sail_int zghz31190;
sail_int zghz31191;
sail_int zghz31192;
sail_int zghz31193;
sail_int zghz31194;
sail_int zghz31195;
sail_int zghz31196;
sail_int zghz31197;
sail_int zghz31198;
sail_int zghz31199;
sail_int zghz31200;
sail_int zghz31201;
sail_int zghz31202;
sail_int zghz31203;
sail_int zghz31204;
sail_int zghz31205;
sail_int zghz31206;
sail_int zghz31207;
sail_int zghz31208;
sail_int zghz31209;
sail_int zghz31210;
sail_int zghz31211;
sail_int zghz31212;
sail_int zghz31213;
sail_int zghz31214;
sail_int zghz31215;
sail_int zghz31216;
sail_int zghz31217;
sail_int zghz31218;
sail_int zghz31219;
lbits zghz31220;
sail_int zghz31221;
lbits zghz31222;
sail_int zghz31223;
lbits zghz31224;
sail_int zghz31225;
lbits zghz31226;
sail_int zghz31227;
lbits zghz31228;
sail_int zghz31229;
lbits zghz31230;
sail_int zghz31231;
lbits zghz31232;
sail_int zghz31233;
lbits zghz31234;
sail_int zghz31235;

static void startup_sailgen_initializze_registers(void)
{
  CREATE(sail_int)(&zghz3576);
  CREATE(sail_int)(&zghz3577);
  CREATE(sail_int)(&zghz3578);
  CREATE(lbits)(&zghz3579);
  CREATE(sail_int)(&zghz3580);
  CREATE(lbits)(&zghz3581);
  CREATE(sail_int)(&zghz3582);
  CREATE(lbits)(&zghz3583);
  CREATE(lbits)(&zghz3584);
  CREATE(sail_int)(&zghz3585);
  CREATE(sail_int)(&zghz3586);
  CREATE(sail_int)(&zghz3587);
  CREATE(lbits)(&zghz3588);
  CREATE(sail_int)(&zghz3589);
  CREATE(lbits)(&zghz3590);
  CREATE(sail_int)(&zghz3591);
  CREATE(lbits)(&zghz3592);
  CREATE(sail_int)(&zghz3593);
  CREATE(lbits)(&zghz3594);
  CREATE(sail_int)(&zghz3595);
  CREATE(lbits)(&zghz3596);
  CREATE(sail_int)(&zghz3597);
  CREATE(lbits)(&zghz3598);
  CREATE(sail_int)(&zghz3599);
  CREATE(lbits)(&zghz3600);
  CREATE(sail_int)(&zghz3601);
  CREATE(lbits)(&zghz3602);
  CREATE(sail_int)(&zghz3603);
  CREATE(lbits)(&zghz3604);
  CREATE(sail_int)(&zghz3605);
  CREATE(lbits)(&zghz3606);
  CREATE(sail_int)(&zghz3607);
  CREATE(lbits)(&zghz3608);
  CREATE(sail_int)(&zghz3609);
  CREATE(lbits)(&zghz3610);
  CREATE(sail_int)(&zghz3611);
  CREATE(lbits)(&zghz3612);
  CREATE(sail_int)(&zghz3613);
  CREATE(sail_int)(&zghz3614);
  CREATE(sail_int)(&zghz3615);
  CREATE(sail_int)(&zghz3616);
  CREATE(sail_int)(&zghz3617);
  CREATE(sail_int)(&zghz3618);
  CREATE(lbits)(&zghz3619);
  CREATE(sail_int)(&zghz3620);
  CREATE(lbits)(&zghz3621);
  CREATE(sail_int)(&zghz3622);
  CREATE(lbits)(&zghz3623);
  CREATE(sail_int)(&zghz3624);
  CREATE(lbits)(&zghz3625);
  CREATE(sail_int)(&zghz3626);
  CREATE(lbits)(&zghz3627);
  CREATE(sail_int)(&zghz3628);
  CREATE(lbits)(&zghz3629);
  CREATE(sail_int)(&zghz3630);
  CREATE(lbits)(&zghz3631);
  CREATE(sail_int)(&zghz3632);
  CREATE(lbits)(&zghz3633);
  CREATE(sail_int)(&zghz3634);
  CREATE(sail_int)(&zghz3635);
  CREATE(sail_int)(&zghz3636);
  CREATE(sail_int)(&zghz3637);
  CREATE(sail_int)(&zghz3638);
  CREATE(sail_int)(&zghz3639);
  CREATE(sail_int)(&zghz3640);
  CREATE(sail_int)(&zghz3641);
  CREATE(sail_int)(&zghz3642);
  CREATE(lbits)(&zghz3643);
  CREATE(sail_int)(&zghz3644);
  CREATE(lbits)(&zghz3645);
  CREATE(sail_int)(&zghz3646);
  CREATE(lbits)(&zghz3647);
  CREATE(sail_int)(&zghz3648);
  CREATE(lbits)(&zghz3649);
  CREATE(sail_int)(&zghz3650);
  CREATE(lbits)(&zghz3651);
  CREATE(sail_int)(&zghz3652);
  CREATE(lbits)(&zghz3653);
  CREATE(sail_int)(&zghz3654);
  CREATE(lbits)(&zghz3655);
  CREATE(sail_int)(&zghz3656);
  CREATE(lbits)(&zghz3657);
  CREATE(sail_int)(&zghz3658);
  CREATE(sail_int)(&zghz3659);
  CREATE(lbits)(&zghz3660);
  CREATE(sail_int)(&zghz3661);
  CREATE(lbits)(&zghz3662);
  CREATE(sail_int)(&zghz3663);
  CREATE(lbits)(&zghz3664);
  CREATE(sail_int)(&zghz3665);
  CREATE(lbits)(&zghz3666);
  CREATE(sail_int)(&zghz3667);
  CREATE(sail_int)(&zghz3668);
  CREATE(sail_int)(&zghz3669);
  CREATE(lbits)(&zghz3670);
  CREATE(sail_int)(&zghz3671);
  CREATE(lbits)(&zghz3672);
  CREATE(sail_int)(&zghz3673);
  CREATE(lbits)(&zghz3674);
  CREATE(sail_int)(&zghz3675);
  CREATE(lbits)(&zghz3676);
  CREATE(sail_int)(&zghz3677);
  CREATE(lbits)(&zghz3678);
  CREATE(sail_int)(&zghz3679);
  CREATE(lbits)(&zghz3680);
  CREATE(sail_int)(&zghz3681);
  CREATE(lbits)(&zghz3682);
  CREATE(sail_int)(&zghz3683);
  CREATE(lbits)(&zghz3684);
  CREATE(sail_int)(&zghz3685);
  CREATE(lbits)(&zghz3686);
  CREATE(sail_int)(&zghz3687);
  CREATE(lbits)(&zghz3688);
  CREATE(sail_int)(&zghz3689);
  CREATE(lbits)(&zghz3690);
  CREATE(sail_int)(&zghz3691);
  CREATE(lbits)(&zghz3692);
  CREATE(sail_int)(&zghz3693);
  CREATE(lbits)(&zghz3694);
  CREATE(sail_int)(&zghz3695);
  CREATE(lbits)(&zghz3696);
  CREATE(sail_int)(&zghz3697);
  CREATE(lbits)(&zghz3698);
  CREATE(sail_int)(&zghz3699);
  CREATE(lbits)(&zghz3700);
  CREATE(sail_int)(&zghz3701);
  CREATE(lbits)(&zghz3702);
  CREATE(sail_int)(&zghz3703);
  CREATE(lbits)(&zghz3704);
  CREATE(sail_int)(&zghz3705);
  CREATE(lbits)(&zghz3706);
  CREATE(sail_int)(&zghz3707);
  CREATE(sail_int)(&zghz3708);
  CREATE(lbits)(&zghz3709);
  CREATE(sail_int)(&zghz3710);
  CREATE(sail_int)(&zghz3711);
  CREATE(lbits)(&zghz3712);
  CREATE(sail_int)(&zghz3713);
  CREATE(lbits)(&zghz3714);
  CREATE(sail_int)(&zghz3715);
  CREATE(lbits)(&zghz3716);
  CREATE(sail_int)(&zghz3717);
  CREATE(lbits)(&zghz3718);
  CREATE(sail_int)(&zghz3719);
  CREATE(lbits)(&zghz3720);
  CREATE(sail_int)(&zghz3721);
  CREATE(lbits)(&zghz3722);
  CREATE(sail_int)(&zghz3723);
  CREATE(lbits)(&zghz3724);
  CREATE(sail_int)(&zghz3725);
  CREATE(lbits)(&zghz3726);
  CREATE(sail_int)(&zghz3727);
  CREATE(lbits)(&zghz3728);
  CREATE(sail_int)(&zghz3729);
  CREATE(lbits)(&zghz3730);
  CREATE(sail_int)(&zghz3731);
  CREATE(lbits)(&zghz3732);
  CREATE(sail_int)(&zghz3733);
  CREATE(lbits)(&zghz3734);
  CREATE(sail_int)(&zghz3735);
  CREATE(lbits)(&zghz3736);
  CREATE(sail_int)(&zghz3737);
  CREATE(lbits)(&zghz3738);
  CREATE(sail_int)(&zghz3739);
  CREATE(lbits)(&zghz3740);
  CREATE(sail_int)(&zghz3741);
  CREATE(lbits)(&zghz3742);
  CREATE(sail_int)(&zghz3743);
  CREATE(lbits)(&zghz3744);
  CREATE(sail_int)(&zghz3745);
  CREATE(lbits)(&zghz3746);
  CREATE(sail_int)(&zghz3747);
  CREATE(lbits)(&zghz3748);
  CREATE(sail_int)(&zghz3749);
  CREATE(lbits)(&zghz3750);
  CREATE(sail_int)(&zghz3751);
  CREATE(lbits)(&zghz3752);
  CREATE(sail_int)(&zghz3753);
  CREATE(lbits)(&zghz3754);
  CREATE(sail_int)(&zghz3755);
  CREATE(lbits)(&zghz3756);
  CREATE(sail_int)(&zghz3757);
  CREATE(lbits)(&zghz3758);
  CREATE(sail_int)(&zghz3759);
  CREATE(lbits)(&zghz3760);
  CREATE(sail_int)(&zghz3761);
  CREATE(lbits)(&zghz3762);
  CREATE(sail_int)(&zghz3763);
  CREATE(lbits)(&zghz3764);
  CREATE(sail_int)(&zghz3765);
  CREATE(lbits)(&zghz3766);
  CREATE(sail_int)(&zghz3767);
  CREATE(lbits)(&zghz3768);
  CREATE(sail_int)(&zghz3769);
  CREATE(lbits)(&zghz3770);
  CREATE(sail_int)(&zghz3771);
  CREATE(lbits)(&zghz3772);
  CREATE(sail_int)(&zghz3773);
  CREATE(lbits)(&zghz3774);
  CREATE(sail_int)(&zghz3775);
  CREATE(lbits)(&zghz3776);
  CREATE(sail_int)(&zghz3777);
  CREATE(lbits)(&zghz3778);
  CREATE(sail_int)(&zghz3779);
  CREATE(lbits)(&zghz3780);
  CREATE(sail_int)(&zghz3781);
  CREATE(lbits)(&zghz3782);
  CREATE(sail_int)(&zghz3783);
  CREATE(lbits)(&zghz3784);
  CREATE(sail_int)(&zghz3785);
  CREATE(lbits)(&zghz3786);
  CREATE(sail_int)(&zghz3787);
  CREATE(lbits)(&zghz3788);
  CREATE(sail_int)(&zghz3789);
  CREATE(lbits)(&zghz3790);
  CREATE(sail_int)(&zghz3791);
  CREATE(lbits)(&zghz3792);
  CREATE(sail_int)(&zghz3793);
  CREATE(lbits)(&zghz3794);
  CREATE(sail_int)(&zghz3795);
  CREATE(lbits)(&zghz3796);
  CREATE(sail_int)(&zghz3797);
  CREATE(lbits)(&zghz3798);
  CREATE(sail_int)(&zghz3799);
  CREATE(lbits)(&zghz3800);
  CREATE(sail_int)(&zghz3801);
  CREATE(lbits)(&zghz3802);
  CREATE(sail_int)(&zghz3803);
  CREATE(lbits)(&zghz3804);
  CREATE(sail_int)(&zghz3805);
  CREATE(lbits)(&zghz3806);
  CREATE(sail_int)(&zghz3807);
  CREATE(lbits)(&zghz3808);
  CREATE(sail_int)(&zghz3809);
  CREATE(lbits)(&zghz3810);
  CREATE(sail_int)(&zghz3811);
  CREATE(lbits)(&zghz3812);
  CREATE(sail_int)(&zghz3813);
  CREATE(lbits)(&zghz3814);
  CREATE(sail_int)(&zghz3815);
  CREATE(lbits)(&zghz3816);
  CREATE(sail_int)(&zghz3817);
  CREATE(lbits)(&zghz3818);
  CREATE(sail_int)(&zghz3819);
  CREATE(lbits)(&zghz3820);
  CREATE(sail_int)(&zghz3821);
  CREATE(lbits)(&zghz3822);
  CREATE(sail_int)(&zghz3823);
  CREATE(lbits)(&zghz3824);
  CREATE(sail_int)(&zghz3825);
  CREATE(lbits)(&zghz3826);
  CREATE(sail_int)(&zghz3827);
  CREATE(lbits)(&zghz3828);
  CREATE(sail_int)(&zghz3829);
  CREATE(lbits)(&zghz3830);
  CREATE(sail_int)(&zghz3831);
  CREATE(lbits)(&zghz3832);
  CREATE(sail_int)(&zghz3833);
  CREATE(lbits)(&zghz3834);
  CREATE(sail_int)(&zghz3835);
  CREATE(lbits)(&zghz3836);
  CREATE(sail_int)(&zghz3837);
  CREATE(lbits)(&zghz3838);
  CREATE(sail_int)(&zghz3839);
  CREATE(lbits)(&zghz3840);
  CREATE(sail_int)(&zghz3841);
  CREATE(lbits)(&zghz3842);
  CREATE(sail_int)(&zghz3843);
  CREATE(lbits)(&zghz3844);
  CREATE(sail_int)(&zghz3845);
  CREATE(lbits)(&zghz3846);
  CREATE(sail_int)(&zghz3847);
  CREATE(lbits)(&zghz3848);
  CREATE(sail_int)(&zghz3849);
  CREATE(lbits)(&zghz3850);
  CREATE(sail_int)(&zghz3851);
  CREATE(lbits)(&zghz3852);
  CREATE(sail_int)(&zghz3853);
  CREATE(lbits)(&zghz3854);
  CREATE(sail_int)(&zghz3855);
  CREATE(lbits)(&zghz3856);
  CREATE(sail_int)(&zghz3857);
  CREATE(lbits)(&zghz3858);
  CREATE(sail_int)(&zghz3859);
  CREATE(lbits)(&zghz3860);
  CREATE(sail_int)(&zghz3861);
  CREATE(lbits)(&zghz3862);
  CREATE(sail_int)(&zghz3863);
  CREATE(lbits)(&zghz3864);
  CREATE(sail_int)(&zghz3865);
  CREATE(lbits)(&zghz3866);
  CREATE(sail_int)(&zghz3867);
  CREATE(lbits)(&zghz3868);
  CREATE(sail_int)(&zghz3869);
  CREATE(lbits)(&zghz3870);
  CREATE(sail_int)(&zghz3871);
  CREATE(lbits)(&zghz3872);
  CREATE(sail_int)(&zghz3873);
  CREATE(lbits)(&zghz3874);
  CREATE(sail_int)(&zghz3875);
  CREATE(lbits)(&zghz3876);
  CREATE(sail_int)(&zghz3877);
  CREATE(lbits)(&zghz3878);
  CREATE(sail_int)(&zghz3879);
  CREATE(lbits)(&zghz3880);
  CREATE(sail_int)(&zghz3881);
  CREATE(sail_int)(&zghz3882);
  CREATE(lbits)(&zghz3883);
  CREATE(sail_int)(&zghz3884);
  CREATE(lbits)(&zghz3885);
  CREATE(sail_int)(&zghz3886);
  CREATE(lbits)(&zghz3887);
  CREATE(sail_int)(&zghz3888);
  CREATE(sail_int)(&zghz3889);
  CREATE(lbits)(&zghz3890);
  CREATE(sail_int)(&zghz3891);
  CREATE(sail_int)(&zghz3892);
  CREATE(lbits)(&zghz3893);
  CREATE(sail_int)(&zghz3894);
  CREATE(lbits)(&zghz3895);
  CREATE(sail_int)(&zghz3896);
  CREATE(lbits)(&zghz3897);
  CREATE(sail_int)(&zghz3898);
  CREATE(lbits)(&zghz3899);
  CREATE(sail_int)(&zghz3900);
  CREATE(lbits)(&zghz3901);
  CREATE(sail_int)(&zghz3902);
  CREATE(lbits)(&zghz3903);
  CREATE(sail_int)(&zghz3904);
  CREATE(lbits)(&zghz3905);
  CREATE(sail_int)(&zghz3906);
  CREATE(lbits)(&zghz3907);
  CREATE(sail_int)(&zghz3908);
  CREATE(lbits)(&zghz3909);
  CREATE(sail_int)(&zghz3910);
  CREATE(lbits)(&zghz3911);
  CREATE(sail_int)(&zghz3912);
  CREATE(lbits)(&zghz3913);
  CREATE(sail_int)(&zghz3914);
  CREATE(lbits)(&zghz3915);
  CREATE(sail_int)(&zghz3916);
  CREATE(lbits)(&zghz3917);
  CREATE(sail_int)(&zghz3918);
  CREATE(lbits)(&zghz3919);
  CREATE(sail_int)(&zghz3920);
  CREATE(lbits)(&zghz3921);
  CREATE(sail_int)(&zghz3922);
  CREATE(lbits)(&zghz3923);
  CREATE(sail_int)(&zghz3924);
  CREATE(lbits)(&zghz3925);
  CREATE(sail_int)(&zghz3926);
  CREATE(lbits)(&zghz3927);
  CREATE(sail_int)(&zghz3928);
  CREATE(lbits)(&zghz3929);
  CREATE(sail_int)(&zghz3930);
  CREATE(lbits)(&zghz3931);
  CREATE(sail_int)(&zghz3932);
  CREATE(lbits)(&zghz3933);
  CREATE(sail_int)(&zghz3934);
  CREATE(lbits)(&zghz3935);
  CREATE(sail_int)(&zghz3936);
  CREATE(lbits)(&zghz3937);
  CREATE(sail_int)(&zghz3938);
  CREATE(lbits)(&zghz3939);
  CREATE(sail_int)(&zghz3940);
  CREATE(lbits)(&zghz3941);
  CREATE(sail_int)(&zghz3942);
  CREATE(lbits)(&zghz3943);
  CREATE(sail_int)(&zghz3944);
  CREATE(lbits)(&zghz3945);
  CREATE(sail_int)(&zghz3946);
  CREATE(lbits)(&zghz3947);
  CREATE(sail_int)(&zghz3948);
  CREATE(sail_int)(&zghz3949);
  CREATE(lbits)(&zghz3950);
  CREATE(sail_int)(&zghz3951);
  CREATE(sail_int)(&zghz3952);
  CREATE(lbits)(&zghz3953);
  CREATE(sail_int)(&zghz3954);
  CREATE(sail_int)(&zghz3955);
  CREATE(lbits)(&zghz3956);
  CREATE(sail_int)(&zghz3957);
  CREATE(sail_int)(&zghz3958);
  CREATE(lbits)(&zghz3959);
  CREATE(sail_int)(&zghz3960);
  CREATE(sail_int)(&zghz3961);
  CREATE(lbits)(&zghz3962);
  CREATE(sail_int)(&zghz3963);
  CREATE(lbits)(&zghz3964);
  CREATE(sail_int)(&zghz3965);
  CREATE(lbits)(&zghz3966);
  CREATE(sail_int)(&zghz3967);
  CREATE(lbits)(&zghz3968);
  CREATE(sail_int)(&zghz3969);
  CREATE(lbits)(&zghz3970);
  CREATE(sail_int)(&zghz3971);
  CREATE(lbits)(&zghz3972);
  CREATE(sail_int)(&zghz3973);
  CREATE(sail_int)(&zghz3974);
  CREATE(lbits)(&zghz3975);
  CREATE(sail_int)(&zghz3976);
  CREATE(lbits)(&zghz3977);
  CREATE(sail_int)(&zghz3978);
  CREATE(lbits)(&zghz3979);
  CREATE(sail_int)(&zghz3980);
  CREATE(lbits)(&zghz3981);
  CREATE(sail_int)(&zghz3982);
  CREATE(lbits)(&zghz3983);
  CREATE(sail_int)(&zghz3984);
  CREATE(lbits)(&zghz3985);
  CREATE(sail_int)(&zghz3986);
  CREATE(lbits)(&zghz3987);
  CREATE(sail_int)(&zghz3988);
  CREATE(lbits)(&zghz3989);
  CREATE(sail_int)(&zghz3990);
  CREATE(lbits)(&zghz3991);
  CREATE(sail_int)(&zghz3992);
  CREATE(lbits)(&zghz3993);
  CREATE(sail_int)(&zghz3994);
  CREATE(lbits)(&zghz3995);
  CREATE(sail_int)(&zghz3996);
  CREATE(lbits)(&zghz3997);
  CREATE(sail_int)(&zghz3998);
  CREATE(lbits)(&zghz3999);
  CREATE(sail_int)(&zghz31000);
  CREATE(lbits)(&zghz31001);
  CREATE(sail_int)(&zghz31002);
  CREATE(lbits)(&zghz31003);
  CREATE(sail_int)(&zghz31004);
  CREATE(lbits)(&zghz31005);
  CREATE(sail_int)(&zghz31006);
  CREATE(lbits)(&zghz31007);
  CREATE(sail_int)(&zghz31008);
  CREATE(sail_int)(&zghz31009);
  CREATE(lbits)(&zghz31010);
  CREATE(sail_int)(&zghz31011);
  CREATE(sail_int)(&zghz31012);
  CREATE(lbits)(&zghz31013);
  CREATE(sail_int)(&zghz31014);
  CREATE(lbits)(&zghz31015);
  CREATE(sail_int)(&zghz31016);
  CREATE(sail_int)(&zghz31017);
  CREATE(lbits)(&zghz31018);
  CREATE(sail_int)(&zghz31019);
  CREATE(lbits)(&zghz31020);
  CREATE(sail_int)(&zghz31021);
  CREATE(lbits)(&zghz31022);
  CREATE(sail_int)(&zghz31023);
  CREATE(lbits)(&zghz31024);
  CREATE(sail_int)(&zghz31025);
  CREATE(sail_int)(&zghz31026);
  CREATE(lbits)(&zghz31027);
  CREATE(sail_int)(&zghz31028);
  CREATE(lbits)(&zghz31029);
  CREATE(sail_int)(&zghz31030);
  CREATE(lbits)(&zghz31031);
  CREATE(sail_int)(&zghz31032);
  CREATE(lbits)(&zghz31033);
  CREATE(sail_int)(&zghz31034);
  CREATE(lbits)(&zghz31035);
  CREATE(sail_int)(&zghz31036);
  CREATE(sail_int)(&zghz31037);
  CREATE(sail_int)(&zghz31038);
  CREATE(lbits)(&zghz31039);
  CREATE(sail_int)(&zghz31040);
  CREATE(lbits)(&zghz31041);
  CREATE(sail_int)(&zghz31042);
  CREATE(lbits)(&zghz31043);
  CREATE(sail_int)(&zghz31044);
  CREATE(lbits)(&zghz31045);
  CREATE(sail_int)(&zghz31046);
  CREATE(lbits)(&zghz31047);
  CREATE(sail_int)(&zghz31048);
  CREATE(lbits)(&zghz31049);
  CREATE(sail_int)(&zghz31050);
  CREATE(lbits)(&zghz31051);
  CREATE(sail_int)(&zghz31052);
  CREATE(lbits)(&zghz31053);
  CREATE(sail_int)(&zghz31054);
  CREATE(lbits)(&zghz31055);
  CREATE(sail_int)(&zghz31056);
  CREATE(lbits)(&zghz31057);
  CREATE(sail_int)(&zghz31058);
  CREATE(lbits)(&zghz31059);
  CREATE(sail_int)(&zghz31060);
  CREATE(lbits)(&zghz31061);
  CREATE(sail_int)(&zghz31062);
  CREATE(lbits)(&zghz31063);
  CREATE(sail_int)(&zghz31064);
  CREATE(lbits)(&zghz31065);
  CREATE(sail_int)(&zghz31066);
  CREATE(lbits)(&zghz31067);
  CREATE(sail_int)(&zghz31068);
  CREATE(lbits)(&zghz31069);
  CREATE(sail_int)(&zghz31070);
  CREATE(lbits)(&zghz31071);
  CREATE(sail_int)(&zghz31072);
  CREATE(lbits)(&zghz31073);
  CREATE(sail_int)(&zghz31074);
  CREATE(lbits)(&zghz31075);
  CREATE(sail_int)(&zghz31076);
  CREATE(lbits)(&zghz31077);
  CREATE(sail_int)(&zghz31078);
  CREATE(lbits)(&zghz31079);
  CREATE(sail_int)(&zghz31080);
  CREATE(lbits)(&zghz31081);
  CREATE(sail_int)(&zghz31082);
  CREATE(lbits)(&zghz31083);
  CREATE(sail_int)(&zghz31084);
  CREATE(lbits)(&zghz31085);
  CREATE(sail_int)(&zghz31086);
  CREATE(lbits)(&zghz31087);
  CREATE(sail_int)(&zghz31088);
  CREATE(lbits)(&zghz31089);
  CREATE(sail_int)(&zghz31090);
  CREATE(lbits)(&zghz31091);
  CREATE(sail_int)(&zghz31092);
  CREATE(lbits)(&zghz31093);
  CREATE(sail_int)(&zghz31094);
  CREATE(lbits)(&zghz31095);
  CREATE(sail_int)(&zghz31096);
  CREATE(lbits)(&zghz31097);
  CREATE(sail_int)(&zghz31098);
  CREATE(lbits)(&zghz31099);
  CREATE(sail_int)(&zghz31100);
  CREATE(lbits)(&zghz31101);
  CREATE(sail_int)(&zghz31102);
  CREATE(lbits)(&zghz31103);
  CREATE(sail_int)(&zghz31104);
  CREATE(lbits)(&zghz31105);
  CREATE(sail_int)(&zghz31106);
  CREATE(lbits)(&zghz31107);
  CREATE(sail_int)(&zghz31108);
  CREATE(lbits)(&zghz31109);
  CREATE(sail_int)(&zghz31110);
  CREATE(lbits)(&zghz31111);
  CREATE(sail_int)(&zghz31112);
  CREATE(lbits)(&zghz31113);
  CREATE(sail_int)(&zghz31114);
  CREATE(lbits)(&zghz31115);
  CREATE(sail_int)(&zghz31116);
  CREATE(lbits)(&zghz31117);
  CREATE(sail_int)(&zghz31118);
  CREATE(lbits)(&zghz31119);
  CREATE(sail_int)(&zghz31120);
  CREATE(lbits)(&zghz31121);
  CREATE(sail_int)(&zghz31122);
  CREATE(lbits)(&zghz31123);
  CREATE(sail_int)(&zghz31124);
  CREATE(lbits)(&zghz31125);
  CREATE(sail_int)(&zghz31126);
  CREATE(lbits)(&zghz31127);
  CREATE(sail_int)(&zghz31128);
  CREATE(lbits)(&zghz31129);
  CREATE(sail_int)(&zghz31130);
  CREATE(lbits)(&zghz31131);
  CREATE(sail_int)(&zghz31132);
  CREATE(lbits)(&zghz31133);
  CREATE(sail_int)(&zghz31134);
  CREATE(lbits)(&zghz31135);
  CREATE(sail_int)(&zghz31136);
  CREATE(lbits)(&zghz31137);
  CREATE(sail_int)(&zghz31138);
  CREATE(lbits)(&zghz31139);
  CREATE(sail_int)(&zghz31140);
  CREATE(lbits)(&zghz31141);
  CREATE(sail_int)(&zghz31142);
  CREATE(lbits)(&zghz31143);
  CREATE(sail_int)(&zghz31144);
  CREATE(lbits)(&zghz31145);
  CREATE(sail_int)(&zghz31146);
  CREATE(lbits)(&zghz31147);
  CREATE(sail_int)(&zghz31148);
  CREATE(lbits)(&zghz31149);
  CREATE(sail_int)(&zghz31150);
  CREATE(lbits)(&zghz31151);
  CREATE(sail_int)(&zghz31152);
  CREATE(sail_int)(&zghz31153);
  CREATE(sail_int)(&zghz31154);
  CREATE(sail_int)(&zghz31155);
  CREATE(sail_int)(&zghz31156);
  CREATE(sail_int)(&zghz31157);
  CREATE(sail_int)(&zghz31158);
  CREATE(lbits)(&zghz31159);
  CREATE(sail_int)(&zghz31160);
  CREATE(lbits)(&zghz31161);
  CREATE(sail_int)(&zghz31162);
  CREATE(lbits)(&zghz31163);
  CREATE(sail_int)(&zghz31164);
  CREATE(lbits)(&zghz31165);
  CREATE(sail_int)(&zghz31166);
  CREATE(lbits)(&zghz31167);
  CREATE(sail_int)(&zghz31168);
  CREATE(lbits)(&zghz31169);
  CREATE(sail_int)(&zghz31170);
  CREATE(lbits)(&zghz31171);
  CREATE(sail_int)(&zghz31172);
  CREATE(lbits)(&zghz31173);
  CREATE(sail_int)(&zghz31174);
  CREATE(lbits)(&zghz31175);
  CREATE(sail_int)(&zghz31176);
  CREATE(lbits)(&zghz31177);
  CREATE(sail_int)(&zghz31178);
  CREATE(lbits)(&zghz31179);
  CREATE(sail_int)(&zghz31180);
  CREATE(lbits)(&zghz31181);
  CREATE(sail_int)(&zghz31182);
  CREATE(lbits)(&zghz31183);
  CREATE(sail_int)(&zghz31184);
  CREATE(lbits)(&zghz31185);
  CREATE(sail_int)(&zghz31186);
  CREATE(lbits)(&zghz31187);
  CREATE(sail_int)(&zghz31188);
  CREATE(sail_int)(&zghz31189);
  CREATE(sail_int)(&zghz31190);
  CREATE(sail_int)(&zghz31191);
  CREATE(sail_int)(&zghz31192);
  CREATE(sail_int)(&zghz31193);
  CREATE(sail_int)(&zghz31194);
  CREATE(sail_int)(&zghz31195);
  CREATE(sail_int)(&zghz31196);
  CREATE(sail_int)(&zghz31197);
  CREATE(sail_int)(&zghz31198);
  CREATE(sail_int)(&zghz31199);
  CREATE(sail_int)(&zghz31200);
  CREATE(sail_int)(&zghz31201);
  CREATE(sail_int)(&zghz31202);
  CREATE(sail_int)(&zghz31203);
  CREATE(sail_int)(&zghz31204);
  CREATE(sail_int)(&zghz31205);
  CREATE(sail_int)(&zghz31206);
  CREATE(sail_int)(&zghz31207);
  CREATE(sail_int)(&zghz31208);
  CREATE(sail_int)(&zghz31209);
  CREATE(sail_int)(&zghz31210);
  CREATE(sail_int)(&zghz31211);
  CREATE(sail_int)(&zghz31212);
  CREATE(sail_int)(&zghz31213);
  CREATE(sail_int)(&zghz31214);
  CREATE(sail_int)(&zghz31215);
  CREATE(sail_int)(&zghz31216);
  CREATE(sail_int)(&zghz31217);
  CREATE(sail_int)(&zghz31218);
  CREATE(sail_int)(&zghz31219);
  CREATE(lbits)(&zghz31220);
  CREATE(sail_int)(&zghz31221);
  CREATE(lbits)(&zghz31222);
  CREATE(sail_int)(&zghz31223);
  CREATE(lbits)(&zghz31224);
  CREATE(sail_int)(&zghz31225);
  CREATE(lbits)(&zghz31226);
  CREATE(sail_int)(&zghz31227);
  CREATE(lbits)(&zghz31228);
  CREATE(sail_int)(&zghz31229);
  CREATE(lbits)(&zghz31230);
  CREATE(sail_int)(&zghz31231);
  CREATE(lbits)(&zghz31232);
  CREATE(sail_int)(&zghz31233);
  CREATE(lbits)(&zghz31234);
  CREATE(sail_int)(&zghz31235);
}

















































static unit sailgen_initializze_registers(unit zgsz3198)
{
  __label__ end_function_281, end_block_exception_282;

  unit zcbz339;
  {
    undefined_int(&zSEE, UNIT);
    unit zgsz3556;
    zgsz3556 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31235);
      CONVERT_OF(sail_int, mach_int)(&zghz31235, INT64_C(129));
      UNDEFINED(lbits)(&zPCC, zghz31235);
    }
    unit zgsz3555;
    zgsz3555 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31233);
      CONVERT_OF(sail_int, mach_int)(&zghz31233, INT64_C(32));
      RECREATE(lbits)(&zghz31234);
      UNDEFINED(lbits)(&zghz31234, zghz31233);
      zCNTHPS_CTL_EL2 = CONVERT_OF(fbits, lbits)(zghz31234, true);
    }
    unit zgsz3554;
    zgsz3554 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31231);
      CONVERT_OF(sail_int, mach_int)(&zghz31231, INT64_C(64));
      RECREATE(lbits)(&zghz31232);
      UNDEFINED(lbits)(&zghz31232, zghz31231);
      zCNTHPS_CVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31232, true);
    }
    unit zgsz3553;
    zgsz3553 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31229);
      CONVERT_OF(sail_int, mach_int)(&zghz31229, INT64_C(32));
      RECREATE(lbits)(&zghz31230);
      UNDEFINED(lbits)(&zghz31230, zghz31229);
      zCNTHPS_TVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31230, true);
    }
    unit zgsz3552;
    zgsz3552 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31227);
      CONVERT_OF(sail_int, mach_int)(&zghz31227, INT64_C(32));
      RECREATE(lbits)(&zghz31228);
      UNDEFINED(lbits)(&zghz31228, zghz31227);
      zCNTHVS_CTL_EL2 = CONVERT_OF(fbits, lbits)(zghz31228, true);
    }
    unit zgsz3551;
    zgsz3551 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31225);
      CONVERT_OF(sail_int, mach_int)(&zghz31225, INT64_C(64));
      RECREATE(lbits)(&zghz31226);
      UNDEFINED(lbits)(&zghz31226, zghz31225);
      zCNTHVS_CVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31226, true);
    }
    unit zgsz3550;
    zgsz3550 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31223);
      CONVERT_OF(sail_int, mach_int)(&zghz31223, INT64_C(32));
      RECREATE(lbits)(&zghz31224);
      UNDEFINED(lbits)(&zghz31224, zghz31223);
      zCNTHVS_TVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31224, true);
    }
    unit zgsz3549;
    zgsz3549 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31221);
      CONVERT_OF(sail_int, mach_int)(&zghz31221, INT64_C(32));
      RECREATE(lbits)(&zghz31222);
      UNDEFINED(lbits)(&zghz31222, zghz31221);
      zSCR_EL3 = CONVERT_OF(fbits, lbits)(zghz31222, true);
    }
    unit zgsz3548;
    zgsz3548 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31219);
      CONVERT_OF(sail_int, mach_int)(&zghz31219, INT64_C(64));
      RECREATE(lbits)(&zghz31220);
      UNDEFINED(lbits)(&zghz31220, zghz31219);
      zHCR_EL2 = CONVERT_OF(fbits, lbits)(zghz31220, true);
    }
    unit zgsz3547;
    zgsz3547 = UNIT;
  }
  {
    zPSTATE = sailgen_undefined_ProcState(UNIT);
    unit zgsz3546;
    zgsz3546 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31218);
      CONVERT_OF(sail_int, mach_int)(&zghz31218, INT64_C(129));
      UNDEFINED(lbits)(&z_R00, zghz31218);
    }
    unit zgsz3545;
    zgsz3545 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31217);
      CONVERT_OF(sail_int, mach_int)(&zghz31217, INT64_C(129));
      UNDEFINED(lbits)(&z_R01, zghz31217);
    }
    unit zgsz3544;
    zgsz3544 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31216);
      CONVERT_OF(sail_int, mach_int)(&zghz31216, INT64_C(129));
      UNDEFINED(lbits)(&z_R02, zghz31216);
    }
    unit zgsz3543;
    zgsz3543 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31215);
      CONVERT_OF(sail_int, mach_int)(&zghz31215, INT64_C(129));
      UNDEFINED(lbits)(&z_R03, zghz31215);
    }
    unit zgsz3542;
    zgsz3542 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31214);
      CONVERT_OF(sail_int, mach_int)(&zghz31214, INT64_C(129));
      UNDEFINED(lbits)(&z_R04, zghz31214);
    }
    unit zgsz3541;
    zgsz3541 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31213);
      CONVERT_OF(sail_int, mach_int)(&zghz31213, INT64_C(129));
      UNDEFINED(lbits)(&z_R05, zghz31213);
    }
    unit zgsz3540;
    zgsz3540 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31212);
      CONVERT_OF(sail_int, mach_int)(&zghz31212, INT64_C(129));
      UNDEFINED(lbits)(&z_R06, zghz31212);
    }
    unit zgsz3539;
    zgsz3539 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31211);
      CONVERT_OF(sail_int, mach_int)(&zghz31211, INT64_C(129));
      UNDEFINED(lbits)(&z_R07, zghz31211);
    }
    unit zgsz3538;
    zgsz3538 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31210);
      CONVERT_OF(sail_int, mach_int)(&zghz31210, INT64_C(129));
      UNDEFINED(lbits)(&z_R08, zghz31210);
    }
    unit zgsz3537;
    zgsz3537 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31209);
      CONVERT_OF(sail_int, mach_int)(&zghz31209, INT64_C(129));
      UNDEFINED(lbits)(&z_R09, zghz31209);
    }
    unit zgsz3536;
    zgsz3536 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31208);
      CONVERT_OF(sail_int, mach_int)(&zghz31208, INT64_C(129));
      UNDEFINED(lbits)(&z_R10, zghz31208);
    }
    unit zgsz3535;
    zgsz3535 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31207);
      CONVERT_OF(sail_int, mach_int)(&zghz31207, INT64_C(129));
      UNDEFINED(lbits)(&z_R11, zghz31207);
    }
    unit zgsz3534;
    zgsz3534 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31206);
      CONVERT_OF(sail_int, mach_int)(&zghz31206, INT64_C(129));
      UNDEFINED(lbits)(&z_R12, zghz31206);
    }
    unit zgsz3533;
    zgsz3533 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31205);
      CONVERT_OF(sail_int, mach_int)(&zghz31205, INT64_C(129));
      UNDEFINED(lbits)(&z_R13, zghz31205);
    }
    unit zgsz3532;
    zgsz3532 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31204);
      CONVERT_OF(sail_int, mach_int)(&zghz31204, INT64_C(129));
      UNDEFINED(lbits)(&z_R14, zghz31204);
    }
    unit zgsz3531;
    zgsz3531 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31203);
      CONVERT_OF(sail_int, mach_int)(&zghz31203, INT64_C(129));
      UNDEFINED(lbits)(&z_R15, zghz31203);
    }
    unit zgsz3530;
    zgsz3530 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31202);
      CONVERT_OF(sail_int, mach_int)(&zghz31202, INT64_C(129));
      UNDEFINED(lbits)(&z_R16, zghz31202);
    }
    unit zgsz3529;
    zgsz3529 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31201);
      CONVERT_OF(sail_int, mach_int)(&zghz31201, INT64_C(129));
      UNDEFINED(lbits)(&z_R17, zghz31201);
    }
    unit zgsz3528;
    zgsz3528 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31200);
      CONVERT_OF(sail_int, mach_int)(&zghz31200, INT64_C(129));
      UNDEFINED(lbits)(&z_R18, zghz31200);
    }
    unit zgsz3527;
    zgsz3527 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31199);
      CONVERT_OF(sail_int, mach_int)(&zghz31199, INT64_C(129));
      UNDEFINED(lbits)(&z_R19, zghz31199);
    }
    unit zgsz3526;
    zgsz3526 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31198);
      CONVERT_OF(sail_int, mach_int)(&zghz31198, INT64_C(129));
      UNDEFINED(lbits)(&z_R20, zghz31198);
    }
    unit zgsz3525;
    zgsz3525 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31197);
      CONVERT_OF(sail_int, mach_int)(&zghz31197, INT64_C(129));
      UNDEFINED(lbits)(&z_R21, zghz31197);
    }
    unit zgsz3524;
    zgsz3524 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31196);
      CONVERT_OF(sail_int, mach_int)(&zghz31196, INT64_C(129));
      UNDEFINED(lbits)(&z_R22, zghz31196);
    }
    unit zgsz3523;
    zgsz3523 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31195);
      CONVERT_OF(sail_int, mach_int)(&zghz31195, INT64_C(129));
      UNDEFINED(lbits)(&z_R23, zghz31195);
    }
    unit zgsz3522;
    zgsz3522 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31194);
      CONVERT_OF(sail_int, mach_int)(&zghz31194, INT64_C(129));
      UNDEFINED(lbits)(&z_R24, zghz31194);
    }
    unit zgsz3521;
    zgsz3521 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31193);
      CONVERT_OF(sail_int, mach_int)(&zghz31193, INT64_C(129));
      UNDEFINED(lbits)(&z_R25, zghz31193);
    }
    unit zgsz3520;
    zgsz3520 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31192);
      CONVERT_OF(sail_int, mach_int)(&zghz31192, INT64_C(129));
      UNDEFINED(lbits)(&z_R26, zghz31192);
    }
    unit zgsz3519;
    zgsz3519 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31191);
      CONVERT_OF(sail_int, mach_int)(&zghz31191, INT64_C(129));
      UNDEFINED(lbits)(&z_R27, zghz31191);
    }
    unit zgsz3518;
    zgsz3518 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31190);
      CONVERT_OF(sail_int, mach_int)(&zghz31190, INT64_C(129));
      UNDEFINED(lbits)(&z_R28, zghz31190);
    }
    unit zgsz3517;
    zgsz3517 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31189);
      CONVERT_OF(sail_int, mach_int)(&zghz31189, INT64_C(129));
      UNDEFINED(lbits)(&z_R29, zghz31189);
    }
    unit zgsz3516;
    zgsz3516 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31188);
      CONVERT_OF(sail_int, mach_int)(&zghz31188, INT64_C(129));
      UNDEFINED(lbits)(&z_R30, zghz31188);
    }
    unit zgsz3515;
    zgsz3515 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31186);
      CONVERT_OF(sail_int, mach_int)(&zghz31186, INT64_C(32));
      RECREATE(lbits)(&zghz31187);
      UNDEFINED(lbits)(&zghz31187, zghz31186);
      zESR_EL1 = CONVERT_OF(fbits, lbits)(zghz31187, true);
    }
    unit zgsz3514;
    zgsz3514 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31184);
      CONVERT_OF(sail_int, mach_int)(&zghz31184, INT64_C(32));
      RECREATE(lbits)(&zghz31185);
      UNDEFINED(lbits)(&zghz31185, zghz31184);
      zESR_EL2 = CONVERT_OF(fbits, lbits)(zghz31185, true);
    }
    unit zgsz3513;
    zgsz3513 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31182);
      CONVERT_OF(sail_int, mach_int)(&zghz31182, INT64_C(32));
      RECREATE(lbits)(&zghz31183);
      UNDEFINED(lbits)(&zghz31183, zghz31182);
      zESR_EL3 = CONVERT_OF(fbits, lbits)(zghz31183, true);
    }
    unit zgsz3512;
    zgsz3512 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31180);
      CONVERT_OF(sail_int, mach_int)(&zghz31180, INT64_C(64));
      RECREATE(lbits)(&zghz31181);
      UNDEFINED(lbits)(&zghz31181, zghz31180);
      zFAR_EL1 = CONVERT_OF(fbits, lbits)(zghz31181, true);
    }
    unit zgsz3511;
    zgsz3511 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31178);
      CONVERT_OF(sail_int, mach_int)(&zghz31178, INT64_C(64));
      RECREATE(lbits)(&zghz31179);
      UNDEFINED(lbits)(&zghz31179, zghz31178);
      zFAR_EL2 = CONVERT_OF(fbits, lbits)(zghz31179, true);
    }
    unit zgsz3510;
    zgsz3510 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31176);
      CONVERT_OF(sail_int, mach_int)(&zghz31176, INT64_C(64));
      RECREATE(lbits)(&zghz31177);
      UNDEFINED(lbits)(&zghz31177, zghz31176);
      zFAR_EL3 = CONVERT_OF(fbits, lbits)(zghz31177, true);
    }
    unit zgsz3509;
    zgsz3509 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31174);
      CONVERT_OF(sail_int, mach_int)(&zghz31174, INT64_C(64));
      RECREATE(lbits)(&zghz31175);
      UNDEFINED(lbits)(&zghz31175, zghz31174);
      zHPFAR_EL2 = CONVERT_OF(fbits, lbits)(zghz31175, true);
    }
    unit zgsz3508;
    zgsz3508 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31172);
      CONVERT_OF(sail_int, mach_int)(&zghz31172, INT64_C(64));
      RECREATE(lbits)(&zghz31173);
      UNDEFINED(lbits)(&zghz31173, zghz31172);
      zTCR_EL1 = CONVERT_OF(fbits, lbits)(zghz31173, true);
    }
    unit zgsz3507;
    zgsz3507 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31170);
      CONVERT_OF(sail_int, mach_int)(&zghz31170, INT64_C(64));
      RECREATE(lbits)(&zghz31171);
      UNDEFINED(lbits)(&zghz31171, zghz31170);
      zTCR_EL2 = CONVERT_OF(fbits, lbits)(zghz31171, true);
    }
    unit zgsz3506;
    zgsz3506 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31168);
      CONVERT_OF(sail_int, mach_int)(&zghz31168, INT64_C(32));
      RECREATE(lbits)(&zghz31169);
      UNDEFINED(lbits)(&zghz31169, zghz31168);
      zTCR_EL3 = CONVERT_OF(fbits, lbits)(zghz31169, true);
    }
    unit zgsz3505;
    zgsz3505 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31166);
      CONVERT_OF(sail_int, mach_int)(&zghz31166, INT64_C(64));
      RECREATE(lbits)(&zghz31167);
      UNDEFINED(lbits)(&zghz31167, zghz31166);
      z_PC = CONVERT_OF(fbits, lbits)(zghz31167, true);
    }
    unit zgsz3504;
    zgsz3504 = UNIT;
  }
  {
    z__BranchTaken = false;
    unit zgsz3503;
    zgsz3503 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31164);
      CONVERT_OF(sail_int, mach_int)(&zghz31164, INT64_C(32));
      RECREATE(lbits)(&zghz31165);
      UNDEFINED(lbits)(&zghz31165, zghz31164);
      zCCTLR_EL0 = CONVERT_OF(fbits, lbits)(zghz31165, true);
    }
    unit zgsz3502;
    zgsz3502 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31162);
      CONVERT_OF(sail_int, mach_int)(&zghz31162, INT64_C(32));
      RECREATE(lbits)(&zghz31163);
      UNDEFINED(lbits)(&zghz31163, zghz31162);
      zCCTLR_EL1 = CONVERT_OF(fbits, lbits)(zghz31163, true);
    }
    unit zgsz3501;
    zgsz3501 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31160);
      CONVERT_OF(sail_int, mach_int)(&zghz31160, INT64_C(32));
      RECREATE(lbits)(&zghz31161);
      UNDEFINED(lbits)(&zghz31161, zghz31160);
      zCCTLR_EL2 = CONVERT_OF(fbits, lbits)(zghz31161, true);
    }
    unit zgsz3500;
    zgsz3500 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31158);
      CONVERT_OF(sail_int, mach_int)(&zghz31158, INT64_C(32));
      RECREATE(lbits)(&zghz31159);
      UNDEFINED(lbits)(&zghz31159, zghz31158);
      zCCTLR_EL3 = CONVERT_OF(fbits, lbits)(zghz31159, true);
    }
    unit zgsz3499;
    zgsz3499 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31157);
      CONVERT_OF(sail_int, mach_int)(&zghz31157, INT64_C(129));
      UNDEFINED(lbits)(&zELR_EL1, zghz31157);
    }
    unit zgsz3498;
    zgsz3498 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31156);
      CONVERT_OF(sail_int, mach_int)(&zghz31156, INT64_C(129));
      UNDEFINED(lbits)(&zELR_EL2, zghz31156);
    }
    unit zgsz3497;
    zgsz3497 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31155);
      CONVERT_OF(sail_int, mach_int)(&zghz31155, INT64_C(129));
      UNDEFINED(lbits)(&zELR_EL3, zghz31155);
    }
    unit zgsz3496;
    zgsz3496 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31154);
      CONVERT_OF(sail_int, mach_int)(&zghz31154, INT64_C(129));
      UNDEFINED(lbits)(&zVBAR_EL1, zghz31154);
    }
    unit zgsz3495;
    zgsz3495 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31153);
      CONVERT_OF(sail_int, mach_int)(&zghz31153, INT64_C(129));
      UNDEFINED(lbits)(&zVBAR_EL2, zghz31153);
    }
    unit zgsz3494;
    zgsz3494 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31152);
      CONVERT_OF(sail_int, mach_int)(&zghz31152, INT64_C(129));
      UNDEFINED(lbits)(&zVBAR_EL3, zghz31152);
    }
    unit zgsz3493;
    zgsz3493 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31150);
      CONVERT_OF(sail_int, mach_int)(&zghz31150, INT64_C(32));
      RECREATE(lbits)(&zghz31151);
      UNDEFINED(lbits)(&zghz31151, zghz31150);
      zCPACR_EL1 = CONVERT_OF(fbits, lbits)(zghz31151, true);
    }
    unit zgsz3492;
    zgsz3492 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31148);
      CONVERT_OF(sail_int, mach_int)(&zghz31148, INT64_C(32));
      RECREATE(lbits)(&zghz31149);
      UNDEFINED(lbits)(&zghz31149, zghz31148);
      zCPTR_EL2 = CONVERT_OF(fbits, lbits)(zghz31149, true);
    }
    unit zgsz3491;
    zgsz3491 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31146);
      CONVERT_OF(sail_int, mach_int)(&zghz31146, INT64_C(32));
      RECREATE(lbits)(&zghz31147);
      UNDEFINED(lbits)(&zghz31147, zghz31146);
      zCPTR_EL3 = CONVERT_OF(fbits, lbits)(zghz31147, true);
    }
    unit zgsz3490;
    zgsz3490 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31144);
      CONVERT_OF(sail_int, mach_int)(&zghz31144, INT64_C(32));
      RECREATE(lbits)(&zghz31145);
      UNDEFINED(lbits)(&zghz31145, zghz31144);
      zEDSCR = CONVERT_OF(fbits, lbits)(zghz31145, true);
    }
    unit zgsz3489;
    zgsz3489 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31142);
      CONVERT_OF(sail_int, mach_int)(&zghz31142, INT64_C(64));
      RECREATE(lbits)(&zghz31143);
      UNDEFINED(lbits)(&zghz31143, zghz31142);
      zSCTLR_EL1 = CONVERT_OF(fbits, lbits)(zghz31143, true);
    }
    unit zgsz3488;
    zgsz3488 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31140);
      CONVERT_OF(sail_int, mach_int)(&zghz31140, INT64_C(64));
      RECREATE(lbits)(&zghz31141);
      UNDEFINED(lbits)(&zghz31141, zghz31140);
      zSCTLR_EL2 = CONVERT_OF(fbits, lbits)(zghz31141, true);
    }
    unit zgsz3487;
    zgsz3487 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31138);
      CONVERT_OF(sail_int, mach_int)(&zghz31138, INT64_C(64));
      RECREATE(lbits)(&zghz31139);
      UNDEFINED(lbits)(&zghz31139, zghz31138);
      zSCTLR_EL3 = CONVERT_OF(fbits, lbits)(zghz31139, true);
    }
    unit zgsz3486;
    zgsz3486 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31136);
      CONVERT_OF(sail_int, mach_int)(&zghz31136, INT64_C(32));
      RECREATE(lbits)(&zghz31137);
      UNDEFINED(lbits)(&zghz31137, zghz31136);
      zSPSR_EL1 = CONVERT_OF(fbits, lbits)(zghz31137, true);
    }
    unit zgsz3485;
    zgsz3485 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31134);
      CONVERT_OF(sail_int, mach_int)(&zghz31134, INT64_C(32));
      RECREATE(lbits)(&zghz31135);
      UNDEFINED(lbits)(&zghz31135, zghz31134);
      zSPSR_EL2 = CONVERT_OF(fbits, lbits)(zghz31135, true);
    }
    unit zgsz3484;
    zgsz3484 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31132);
      CONVERT_OF(sail_int, mach_int)(&zghz31132, INT64_C(32));
      RECREATE(lbits)(&zghz31133);
      UNDEFINED(lbits)(&zghz31133, zghz31132);
      zSPSR_EL3 = CONVERT_OF(fbits, lbits)(zghz31133, true);
    }
    unit zgsz3483;
    zgsz3483 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31130);
      CONVERT_OF(sail_int, mach_int)(&zghz31130, INT64_C(64));
      RECREATE(lbits)(&zghz31131);
      UNDEFINED(lbits)(&zghz31131, zghz31130);
      zACTLR_EL1 = CONVERT_OF(fbits, lbits)(zghz31131, true);
    }
    unit zgsz3482;
    zgsz3482 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31128);
      CONVERT_OF(sail_int, mach_int)(&zghz31128, INT64_C(64));
      RECREATE(lbits)(&zghz31129);
      UNDEFINED(lbits)(&zghz31129, zghz31128);
      zACTLR_EL2 = CONVERT_OF(fbits, lbits)(zghz31129, true);
    }
    unit zgsz3481;
    zgsz3481 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31126);
      CONVERT_OF(sail_int, mach_int)(&zghz31126, INT64_C(64));
      RECREATE(lbits)(&zghz31127);
      UNDEFINED(lbits)(&zghz31127, zghz31126);
      zACTLR_EL3 = CONVERT_OF(fbits, lbits)(zghz31127, true);
    }
    unit zgsz3480;
    zgsz3480 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31124);
      CONVERT_OF(sail_int, mach_int)(&zghz31124, INT64_C(64));
      RECREATE(lbits)(&zghz31125);
      UNDEFINED(lbits)(&zghz31125, zghz31124);
      zAFSR0_EL1 = CONVERT_OF(fbits, lbits)(zghz31125, true);
    }
    unit zgsz3479;
    zgsz3479 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31122);
      CONVERT_OF(sail_int, mach_int)(&zghz31122, INT64_C(64));
      RECREATE(lbits)(&zghz31123);
      UNDEFINED(lbits)(&zghz31123, zghz31122);
      zAFSR0_EL2 = CONVERT_OF(fbits, lbits)(zghz31123, true);
    }
    unit zgsz3478;
    zgsz3478 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31120);
      CONVERT_OF(sail_int, mach_int)(&zghz31120, INT64_C(64));
      RECREATE(lbits)(&zghz31121);
      UNDEFINED(lbits)(&zghz31121, zghz31120);
      zAFSR0_EL3 = CONVERT_OF(fbits, lbits)(zghz31121, true);
    }
    unit zgsz3477;
    zgsz3477 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31118);
      CONVERT_OF(sail_int, mach_int)(&zghz31118, INT64_C(64));
      RECREATE(lbits)(&zghz31119);
      UNDEFINED(lbits)(&zghz31119, zghz31118);
      zAFSR1_EL1 = CONVERT_OF(fbits, lbits)(zghz31119, true);
    }
    unit zgsz3476;
    zgsz3476 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31116);
      CONVERT_OF(sail_int, mach_int)(&zghz31116, INT64_C(64));
      RECREATE(lbits)(&zghz31117);
      UNDEFINED(lbits)(&zghz31117, zghz31116);
      zAFSR1_EL2 = CONVERT_OF(fbits, lbits)(zghz31117, true);
    }
    unit zgsz3475;
    zgsz3475 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31114);
      CONVERT_OF(sail_int, mach_int)(&zghz31114, INT64_C(64));
      RECREATE(lbits)(&zghz31115);
      UNDEFINED(lbits)(&zghz31115, zghz31114);
      zAFSR1_EL3 = CONVERT_OF(fbits, lbits)(zghz31115, true);
    }
    unit zgsz3474;
    zgsz3474 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31112);
      CONVERT_OF(sail_int, mach_int)(&zghz31112, INT64_C(64));
      RECREATE(lbits)(&zghz31113);
      UNDEFINED(lbits)(&zghz31113, zghz31112);
      zAIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz31113, true);
    }
    unit zgsz3473;
    zgsz3473 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31110);
      CONVERT_OF(sail_int, mach_int)(&zghz31110, INT64_C(64));
      RECREATE(lbits)(&zghz31111);
      UNDEFINED(lbits)(&zghz31111, zghz31110);
      zAMAIR_EL1 = CONVERT_OF(fbits, lbits)(zghz31111, true);
    }
    unit zgsz3472;
    zgsz3472 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31108);
      CONVERT_OF(sail_int, mach_int)(&zghz31108, INT64_C(64));
      RECREATE(lbits)(&zghz31109);
      UNDEFINED(lbits)(&zghz31109, zghz31108);
      zAMAIR_EL2 = CONVERT_OF(fbits, lbits)(zghz31109, true);
    }
    unit zgsz3471;
    zgsz3471 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31106);
      CONVERT_OF(sail_int, mach_int)(&zghz31106, INT64_C(64));
      RECREATE(lbits)(&zghz31107);
      UNDEFINED(lbits)(&zghz31107, zghz31106);
      zAMAIR_EL3 = CONVERT_OF(fbits, lbits)(zghz31107, true);
    }
    unit zgsz3470;
    zgsz3470 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31104);
      CONVERT_OF(sail_int, mach_int)(&zghz31104, INT64_C(32));
      RECREATE(lbits)(&zghz31105);
      UNDEFINED(lbits)(&zghz31105, zghz31104);
      zCCSIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz31105, true);
    }
    unit zgsz3469;
    zgsz3469 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31102);
      CONVERT_OF(sail_int, mach_int)(&zghz31102, INT64_C(32));
      RECREATE(lbits)(&zghz31103);
      UNDEFINED(lbits)(&zghz31103, zghz31102);
      zCHCR_EL2 = CONVERT_OF(fbits, lbits)(zghz31103, true);
    }
    unit zgsz3468;
    zgsz3468 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31100);
      CONVERT_OF(sail_int, mach_int)(&zghz31100, INT64_C(64));
      RECREATE(lbits)(&zghz31101);
      UNDEFINED(lbits)(&zghz31101, zghz31100);
      zCLIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz31101, true);
    }
    unit zgsz3467;
    zgsz3467 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31098);
      CONVERT_OF(sail_int, mach_int)(&zghz31098, INT64_C(32));
      RECREATE(lbits)(&zghz31099);
      UNDEFINED(lbits)(&zghz31099, zghz31098);
      zCNTFRQ_EL0 = CONVERT_OF(fbits, lbits)(zghz31099, true);
    }
    unit zgsz3466;
    zgsz3466 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31096);
      CONVERT_OF(sail_int, mach_int)(&zghz31096, INT64_C(32));
      RECREATE(lbits)(&zghz31097);
      UNDEFINED(lbits)(&zghz31097, zghz31096);
      zCNTHCTL_EL2 = CONVERT_OF(fbits, lbits)(zghz31097, true);
    }
    unit zgsz3465;
    zgsz3465 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31094);
      CONVERT_OF(sail_int, mach_int)(&zghz31094, INT64_C(32));
      RECREATE(lbits)(&zghz31095);
      UNDEFINED(lbits)(&zghz31095, zghz31094);
      zCNTKCTL_EL1 = CONVERT_OF(fbits, lbits)(zghz31095, true);
    }
    unit zgsz3464;
    zgsz3464 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31092);
      CONVERT_OF(sail_int, mach_int)(&zghz31092, INT64_C(32));
      RECREATE(lbits)(&zghz31093);
      UNDEFINED(lbits)(&zghz31093, zghz31092);
      zCNTHP_CTL_EL2 = CONVERT_OF(fbits, lbits)(zghz31093, true);
    }
    unit zgsz3463;
    zgsz3463 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31090);
      CONVERT_OF(sail_int, mach_int)(&zghz31090, INT64_C(64));
      RECREATE(lbits)(&zghz31091);
      UNDEFINED(lbits)(&zghz31091, zghz31090);
      zCNTHP_CVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31091, true);
    }
    unit zgsz3462;
    zgsz3462 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31088);
      CONVERT_OF(sail_int, mach_int)(&zghz31088, INT64_C(32));
      RECREATE(lbits)(&zghz31089);
      UNDEFINED(lbits)(&zghz31089, zghz31088);
      zCNTHP_TVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31089, true);
    }
    unit zgsz3461;
    zgsz3461 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31086);
      CONVERT_OF(sail_int, mach_int)(&zghz31086, INT64_C(32));
      RECREATE(lbits)(&zghz31087);
      UNDEFINED(lbits)(&zghz31087, zghz31086);
      zCNTHV_CTL_EL2 = CONVERT_OF(fbits, lbits)(zghz31087, true);
    }
    unit zgsz3460;
    zgsz3460 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31084);
      CONVERT_OF(sail_int, mach_int)(&zghz31084, INT64_C(64));
      RECREATE(lbits)(&zghz31085);
      UNDEFINED(lbits)(&zghz31085, zghz31084);
      zCNTHV_CVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31085, true);
    }
    unit zgsz3459;
    zgsz3459 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31082);
      CONVERT_OF(sail_int, mach_int)(&zghz31082, INT64_C(32));
      RECREATE(lbits)(&zghz31083);
      UNDEFINED(lbits)(&zghz31083, zghz31082);
      zCNTHV_TVAL_EL2 = CONVERT_OF(fbits, lbits)(zghz31083, true);
    }
    unit zgsz3458;
    zgsz3458 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31080);
      CONVERT_OF(sail_int, mach_int)(&zghz31080, INT64_C(64));
      RECREATE(lbits)(&zghz31081);
      UNDEFINED(lbits)(&zghz31081, zghz31080);
      zCNTPCT_EL0 = CONVERT_OF(fbits, lbits)(zghz31081, true);
    }
    unit zgsz3457;
    zgsz3457 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31078);
      CONVERT_OF(sail_int, mach_int)(&zghz31078, INT64_C(32));
      RECREATE(lbits)(&zghz31079);
      UNDEFINED(lbits)(&zghz31079, zghz31078);
      zCNTPS_CTL_EL1 = CONVERT_OF(fbits, lbits)(zghz31079, true);
    }
    unit zgsz3456;
    zgsz3456 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31076);
      CONVERT_OF(sail_int, mach_int)(&zghz31076, INT64_C(64));
      RECREATE(lbits)(&zghz31077);
      UNDEFINED(lbits)(&zghz31077, zghz31076);
      zCNTPS_CVAL_EL1 = CONVERT_OF(fbits, lbits)(zghz31077, true);
    }
    unit zgsz3455;
    zgsz3455 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31074);
      CONVERT_OF(sail_int, mach_int)(&zghz31074, INT64_C(32));
      RECREATE(lbits)(&zghz31075);
      UNDEFINED(lbits)(&zghz31075, zghz31074);
      zCNTPS_TVAL_EL1 = CONVERT_OF(fbits, lbits)(zghz31075, true);
    }
    unit zgsz3454;
    zgsz3454 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31072);
      CONVERT_OF(sail_int, mach_int)(&zghz31072, INT64_C(32));
      RECREATE(lbits)(&zghz31073);
      UNDEFINED(lbits)(&zghz31073, zghz31072);
      zCNTP_CTL_EL0 = CONVERT_OF(fbits, lbits)(zghz31073, true);
    }
    unit zgsz3453;
    zgsz3453 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31070);
      CONVERT_OF(sail_int, mach_int)(&zghz31070, INT64_C(64));
      RECREATE(lbits)(&zghz31071);
      UNDEFINED(lbits)(&zghz31071, zghz31070);
      zCNTP_CVAL_EL0 = CONVERT_OF(fbits, lbits)(zghz31071, true);
    }
    unit zgsz3452;
    zgsz3452 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31068);
      CONVERT_OF(sail_int, mach_int)(&zghz31068, INT64_C(32));
      RECREATE(lbits)(&zghz31069);
      UNDEFINED(lbits)(&zghz31069, zghz31068);
      zCNTP_TVAL_EL0 = CONVERT_OF(fbits, lbits)(zghz31069, true);
    }
    unit zgsz3451;
    zgsz3451 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31066);
      CONVERT_OF(sail_int, mach_int)(&zghz31066, INT64_C(64));
      RECREATE(lbits)(&zghz31067);
      UNDEFINED(lbits)(&zghz31067, zghz31066);
      zCNTVCT_EL0 = CONVERT_OF(fbits, lbits)(zghz31067, true);
    }
    unit zgsz3450;
    zgsz3450 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31064);
      CONVERT_OF(sail_int, mach_int)(&zghz31064, INT64_C(64));
      RECREATE(lbits)(&zghz31065);
      UNDEFINED(lbits)(&zghz31065, zghz31064);
      zCNTVOFF_EL2 = CONVERT_OF(fbits, lbits)(zghz31065, true);
    }
    unit zgsz3449;
    zgsz3449 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31062);
      CONVERT_OF(sail_int, mach_int)(&zghz31062, INT64_C(32));
      RECREATE(lbits)(&zghz31063);
      UNDEFINED(lbits)(&zghz31063, zghz31062);
      zCNTV_CTL_EL0 = CONVERT_OF(fbits, lbits)(zghz31063, true);
    }
    unit zgsz3448;
    zgsz3448 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31060);
      CONVERT_OF(sail_int, mach_int)(&zghz31060, INT64_C(64));
      RECREATE(lbits)(&zghz31061);
      UNDEFINED(lbits)(&zghz31061, zghz31060);
      zCNTV_CVAL_EL0 = CONVERT_OF(fbits, lbits)(zghz31061, true);
    }
    unit zgsz3447;
    zgsz3447 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31058);
      CONVERT_OF(sail_int, mach_int)(&zghz31058, INT64_C(32));
      RECREATE(lbits)(&zghz31059);
      UNDEFINED(lbits)(&zghz31059, zghz31058);
      zCNTV_TVAL_EL0 = CONVERT_OF(fbits, lbits)(zghz31059, true);
    }
    unit zgsz3446;
    zgsz3446 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31056);
      CONVERT_OF(sail_int, mach_int)(&zghz31056, INT64_C(32));
      RECREATE(lbits)(&zghz31057);
      UNDEFINED(lbits)(&zghz31057, zghz31056);
      zCONTEXTIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz31057, true);
    }
    unit zgsz3445;
    zgsz3445 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31054);
      CONVERT_OF(sail_int, mach_int)(&zghz31054, INT64_C(32));
      RECREATE(lbits)(&zghz31055);
      UNDEFINED(lbits)(&zghz31055, zghz31054);
      zCONTEXTIDR_EL2 = CONVERT_OF(fbits, lbits)(zghz31055, true);
    }
    unit zgsz3444;
    zgsz3444 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31052);
      CONVERT_OF(sail_int, mach_int)(&zghz31052, INT64_C(32));
      RECREATE(lbits)(&zghz31053);
      UNDEFINED(lbits)(&zghz31053, zghz31052);
      zCSCR_EL3 = CONVERT_OF(fbits, lbits)(zghz31053, true);
    }
    unit zgsz3443;
    zgsz3443 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31050);
      CONVERT_OF(sail_int, mach_int)(&zghz31050, INT64_C(32));
      RECREATE(lbits)(&zghz31051);
      UNDEFINED(lbits)(&zghz31051, zghz31050);
      zCSSELR_EL1 = CONVERT_OF(fbits, lbits)(zghz31051, true);
    }
    unit zgsz3442;
    zgsz3442 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31048);
      CONVERT_OF(sail_int, mach_int)(&zghz31048, INT64_C(32));
      RECREATE(lbits)(&zghz31049);
      UNDEFINED(lbits)(&zghz31049, zghz31048);
      zCTR_EL0 = CONVERT_OF(fbits, lbits)(zghz31049, true);
    }
    unit zgsz3441;
    zgsz3441 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31046);
      CONVERT_OF(sail_int, mach_int)(&zghz31046, INT64_C(32));
      RECREATE(lbits)(&zghz31047);
      UNDEFINED(lbits)(&zghz31047, zghz31046);
      zDACR32_EL2 = CONVERT_OF(fbits, lbits)(zghz31047, true);
    }
    unit zgsz3440;
    zgsz3440 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31044);
      CONVERT_OF(sail_int, mach_int)(&zghz31044, INT64_C(32));
      RECREATE(lbits)(&zghz31045);
      UNDEFINED(lbits)(&zghz31045, zghz31044);
      zDBGAUTHSTATUS_EL1 = CONVERT_OF(fbits, lbits)(zghz31045, true);
    }
    unit zgsz3439;
    zgsz3439 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31042);
      CONVERT_OF(sail_int, mach_int)(&zghz31042, INT64_C(32));
      RECREATE(lbits)(&zghz31043);
      UNDEFINED(lbits)(&zghz31043, zghz31042);
      zMDCR_EL2 = CONVERT_OF(fbits, lbits)(zghz31043, true);
    }
    unit zgsz3438;
    zgsz3438 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31040);
      CONVERT_OF(sail_int, mach_int)(&zghz31040, INT64_C(32));
      RECREATE(lbits)(&zghz31041);
      UNDEFINED(lbits)(&zghz31041, zghz31040);
      zMDCR_EL3 = CONVERT_OF(fbits, lbits)(zghz31041, true);
    }
    unit zgsz3437;
    zgsz3437 = UNIT;
  }
  {
    uint64_t zgaz3334;
    {
      RECREATE(sail_int)(&zghz31038);
      CONVERT_OF(sail_int, mach_int)(&zghz31038, INT64_C(32));
      RECREATE(lbits)(&zghz31039);
      UNDEFINED(lbits)(&zghz31039, zghz31038);
      zgaz3334 = CONVERT_OF(fbits, lbits)(zghz31039, true);
    }
    {
      RECREATE(sail_int)(&zghz31037);
      CONVERT_OF(sail_int, mach_int)(&zghz31037, INT64_C(16));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zDBGBCR_EL1, zghz31037, zgaz3334);
    }
    unit zgsz3436;
    zgsz3436 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31036);
      CONVERT_OF(sail_int, mach_int)(&zghz31036, INT64_C(129));
      UNDEFINED(lbits)(&zCDLR_EL0, zghz31036);
    }
    unit zgsz3435;
    zgsz3435 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31034);
      CONVERT_OF(sail_int, mach_int)(&zghz31034, INT64_C(32));
      RECREATE(lbits)(&zghz31035);
      UNDEFINED(lbits)(&zghz31035, zghz31034);
      zDSPSR_EL0 = CONVERT_OF(fbits, lbits)(zghz31035, true);
    }
    unit zgsz3434;
    zgsz3434 = UNIT;
  }
  {
    zDBGEN = sailgen_undefined_signal(UNIT);
    unit zgsz3433;
    zgsz3433 = UNIT;
  }
  {
    zSPIDEN = sailgen_undefined_signal(UNIT);
    unit zgsz3432;
    zgsz3432 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31032);
      CONVERT_OF(sail_int, mach_int)(&zghz31032, INT64_C(32));
      RECREATE(lbits)(&zghz31033);
      UNDEFINED(lbits)(&zghz31033, zghz31032);
      zDBGPRCR_EL1 = CONVERT_OF(fbits, lbits)(zghz31033, true);
    }
    unit zgsz3431;
    zgsz3431 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31030);
      CONVERT_OF(sail_int, mach_int)(&zghz31030, INT64_C(32));
      RECREATE(lbits)(&zghz31031);
      UNDEFINED(lbits)(&zghz31031, zghz31030);
      zOSDLR_EL1 = CONVERT_OF(fbits, lbits)(zghz31031, true);
    }
    unit zgsz3430;
    zgsz3430 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31028);
      CONVERT_OF(sail_int, mach_int)(&zghz31028, INT64_C(32));
      RECREATE(lbits)(&zghz31029);
      UNDEFINED(lbits)(&zghz31029, zghz31028);
      zOSLSR_EL1 = CONVERT_OF(fbits, lbits)(zghz31029, true);
    }
    unit zgsz3429;
    zgsz3429 = UNIT;
  }
  {
    uint64_t zgaz3335;
    {
      RECREATE(sail_int)(&zghz31026);
      CONVERT_OF(sail_int, mach_int)(&zghz31026, INT64_C(64));
      RECREATE(lbits)(&zghz31027);
      UNDEFINED(lbits)(&zghz31027, zghz31026);
      zgaz3335 = CONVERT_OF(fbits, lbits)(zghz31027, true);
    }
    {
      RECREATE(sail_int)(&zghz31025);
      CONVERT_OF(sail_int, mach_int)(&zghz31025, INT64_C(16));
      UNDEFINED(vector_zz5vecz8z5bv64z9)(&zDBGBVR_EL1, zghz31025, zgaz3335);
    }
    unit zgsz3428;
    zgsz3428 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31023);
      CONVERT_OF(sail_int, mach_int)(&zghz31023, INT64_C(32));
      RECREATE(lbits)(&zghz31024);
      UNDEFINED(lbits)(&zghz31024, zghz31023);
      zDBGCLAIMCLR_EL1 = CONVERT_OF(fbits, lbits)(zghz31024, true);
    }
    unit zgsz3427;
    zgsz3427 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31021);
      CONVERT_OF(sail_int, mach_int)(&zghz31021, INT64_C(32));
      RECREATE(lbits)(&zghz31022);
      UNDEFINED(lbits)(&zghz31022, zghz31021);
      zDBGCLAIMSET_EL1 = CONVERT_OF(fbits, lbits)(zghz31022, true);
    }
    unit zgsz3426;
    zgsz3426 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31019);
      CONVERT_OF(sail_int, mach_int)(&zghz31019, INT64_C(32));
      RECREATE(lbits)(&zghz31020);
      UNDEFINED(lbits)(&zghz31020, zghz31019);
      zDBGDTRRX_EL0 = CONVERT_OF(fbits, lbits)(zghz31020, true);
    }
    unit zgsz3425;
    zgsz3425 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31017);
      CONVERT_OF(sail_int, mach_int)(&zghz31017, INT64_C(32));
      RECREATE(lbits)(&zghz31018);
      UNDEFINED(lbits)(&zghz31018, zghz31017);
      zMDSCR_EL1 = CONVERT_OF(fbits, lbits)(zghz31018, true);
    }
    unit zgsz3424;
    zgsz3424 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31016);
      CONVERT_OF(sail_int, mach_int)(&zghz31016, INT64_C(129));
      UNDEFINED(lbits)(&zCDBGDTR_EL0, zghz31016);
    }
    unit zgsz3423;
    zgsz3423 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31014);
      CONVERT_OF(sail_int, mach_int)(&zghz31014, INT64_C(32));
      RECREATE(lbits)(&zghz31015);
      UNDEFINED(lbits)(&zghz31015, zghz31014);
      zDBGVCR32_EL2 = CONVERT_OF(fbits, lbits)(zghz31015, true);
    }
    unit zgsz3422;
    zgsz3422 = UNIT;
  }
  {
    uint64_t zgaz3336;
    {
      RECREATE(sail_int)(&zghz31012);
      CONVERT_OF(sail_int, mach_int)(&zghz31012, INT64_C(32));
      RECREATE(lbits)(&zghz31013);
      UNDEFINED(lbits)(&zghz31013, zghz31012);
      zgaz3336 = CONVERT_OF(fbits, lbits)(zghz31013, true);
    }
    {
      RECREATE(sail_int)(&zghz31011);
      CONVERT_OF(sail_int, mach_int)(&zghz31011, INT64_C(16));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zDBGWCR_EL1, zghz31011, zgaz3336);
    }
    unit zgsz3421;
    zgsz3421 = UNIT;
  }
  {
    uint64_t zgaz3337;
    {
      RECREATE(sail_int)(&zghz31009);
      CONVERT_OF(sail_int, mach_int)(&zghz31009, INT64_C(64));
      RECREATE(lbits)(&zghz31010);
      UNDEFINED(lbits)(&zghz31010, zghz31009);
      zgaz3337 = CONVERT_OF(fbits, lbits)(zghz31010, true);
    }
    {
      RECREATE(sail_int)(&zghz31008);
      CONVERT_OF(sail_int, mach_int)(&zghz31008, INT64_C(16));
      UNDEFINED(vector_zz5vecz8z5bv64z9)(&zDBGWVR_EL1, zghz31008, zgaz3337);
    }
    unit zgsz3420;
    zgsz3420 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31006);
      CONVERT_OF(sail_int, mach_int)(&zghz31006, INT64_C(32));
      RECREATE(lbits)(&zghz31007);
      UNDEFINED(lbits)(&zghz31007, zghz31006);
      zDCZID_EL0 = CONVERT_OF(fbits, lbits)(zghz31007, true);
    }
    unit zgsz3419;
    zgsz3419 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31004);
      CONVERT_OF(sail_int, mach_int)(&zghz31004, INT64_C(32));
      RECREATE(lbits)(&zghz31005);
      UNDEFINED(lbits)(&zghz31005, zghz31004);
      zDISR_EL1 = CONVERT_OF(fbits, lbits)(zghz31005, true);
    }
    unit zgsz3418;
    zgsz3418 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31002);
      CONVERT_OF(sail_int, mach_int)(&zghz31002, INT64_C(32));
      RECREATE(lbits)(&zghz31003);
      UNDEFINED(lbits)(&zghz31003, zghz31002);
      zVDISR_EL2 = CONVERT_OF(fbits, lbits)(zghz31003, true);
    }
    unit zgsz3417;
    zgsz3417 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz31000);
      CONVERT_OF(sail_int, mach_int)(&zghz31000, INT64_C(32));
      RECREATE(lbits)(&zghz31001);
      UNDEFINED(lbits)(&zghz31001, zghz31000);
      zERRIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz31001, true);
    }
    unit zgsz3416;
    zgsz3416 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3998);
      CONVERT_OF(sail_int, mach_int)(&zghz3998, INT64_C(32));
      RECREATE(lbits)(&zghz3999);
      UNDEFINED(lbits)(&zghz3999, zghz3998);
      zERRSELR_EL1 = CONVERT_OF(fbits, lbits)(zghz3999, true);
    }
    unit zgsz3415;
    zgsz3415 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3996);
      CONVERT_OF(sail_int, mach_int)(&zghz3996, INT64_C(64));
      RECREATE(lbits)(&zghz3997);
      UNDEFINED(lbits)(&zghz3997, zghz3996);
      zERXADDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3997, true);
    }
    unit zgsz3414;
    zgsz3414 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3994);
      CONVERT_OF(sail_int, mach_int)(&zghz3994, INT64_C(64));
      RECREATE(lbits)(&zghz3995);
      UNDEFINED(lbits)(&zghz3995, zghz3994);
      zERXCTLR_EL1 = CONVERT_OF(fbits, lbits)(zghz3995, true);
    }
    unit zgsz3413;
    zgsz3413 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3992);
      CONVERT_OF(sail_int, mach_int)(&zghz3992, INT64_C(64));
      RECREATE(lbits)(&zghz3993);
      UNDEFINED(lbits)(&zghz3993, zghz3992);
      zERXFR_EL1 = CONVERT_OF(fbits, lbits)(zghz3993, true);
    }
    unit zgsz3412;
    zgsz3412 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3990);
      CONVERT_OF(sail_int, mach_int)(&zghz3990, INT64_C(64));
      RECREATE(lbits)(&zghz3991);
      UNDEFINED(lbits)(&zghz3991, zghz3990);
      zERXMISC0_EL1 = CONVERT_OF(fbits, lbits)(zghz3991, true);
    }
    unit zgsz3411;
    zgsz3411 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3988);
      CONVERT_OF(sail_int, mach_int)(&zghz3988, INT64_C(64));
      RECREATE(lbits)(&zghz3989);
      UNDEFINED(lbits)(&zghz3989, zghz3988);
      zERXMISC1_EL1 = CONVERT_OF(fbits, lbits)(zghz3989, true);
    }
    unit zgsz3410;
    zgsz3410 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3986);
      CONVERT_OF(sail_int, mach_int)(&zghz3986, INT64_C(64));
      RECREATE(lbits)(&zghz3987);
      UNDEFINED(lbits)(&zghz3987, zghz3986);
      zERXSTATUS_EL1 = CONVERT_OF(fbits, lbits)(zghz3987, true);
    }
    unit zgsz3409;
    zgsz3409 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3984);
      CONVERT_OF(sail_int, mach_int)(&zghz3984, INT64_C(32));
      RECREATE(lbits)(&zghz3985);
      UNDEFINED(lbits)(&zghz3985, zghz3984);
      zFPCR = CONVERT_OF(fbits, lbits)(zghz3985, true);
    }
    unit zgsz3408;
    zgsz3408 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3982);
      CONVERT_OF(sail_int, mach_int)(&zghz3982, INT64_C(32));
      RECREATE(lbits)(&zghz3983);
      UNDEFINED(lbits)(&zghz3983, zghz3982);
      zFPEXC32_EL2 = CONVERT_OF(fbits, lbits)(zghz3983, true);
    }
    unit zgsz3407;
    zgsz3407 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3980);
      CONVERT_OF(sail_int, mach_int)(&zghz3980, INT64_C(32));
      RECREATE(lbits)(&zghz3981);
      UNDEFINED(lbits)(&zghz3981, zghz3980);
      zFPSR = CONVERT_OF(fbits, lbits)(zghz3981, true);
    }
    unit zgsz3406;
    zgsz3406 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3978);
      CONVERT_OF(sail_int, mach_int)(&zghz3978, INT64_C(64));
      RECREATE(lbits)(&zghz3979);
      UNDEFINED(lbits)(&zghz3979, zghz3978);
      zHACR_EL2 = CONVERT_OF(fbits, lbits)(zghz3979, true);
    }
    unit zgsz3405;
    zgsz3405 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3976);
      CONVERT_OF(sail_int, mach_int)(&zghz3976, INT64_C(32));
      RECREATE(lbits)(&zghz3977);
      UNDEFINED(lbits)(&zghz3977, zghz3976);
      zHSTR_EL2 = CONVERT_OF(fbits, lbits)(zghz3977, true);
    }
    unit zgsz3404;
    zgsz3404 = UNIT;
  }
  {
    uint64_t zgaz3338;
    {
      RECREATE(sail_int)(&zghz3974);
      CONVERT_OF(sail_int, mach_int)(&zghz3974, INT64_C(32));
      RECREATE(lbits)(&zghz3975);
      UNDEFINED(lbits)(&zghz3975, zghz3974);
      zgaz3338 = CONVERT_OF(fbits, lbits)(zghz3975, true);
    }
    {
      RECREATE(sail_int)(&zghz3973);
      CONVERT_OF(sail_int, mach_int)(&zghz3973, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICC_AP0R_EL1, zghz3973, zgaz3338);
    }
    unit zgsz3403;
    zgsz3403 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3971);
      CONVERT_OF(sail_int, mach_int)(&zghz3971, INT64_C(32));
      RECREATE(lbits)(&zghz3972);
      UNDEFINED(lbits)(&zghz3972, zghz3971);
      zICC_SRE_EL1_NS = CONVERT_OF(fbits, lbits)(zghz3972, true);
    }
    unit zgsz3402;
    zgsz3402 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3969);
      CONVERT_OF(sail_int, mach_int)(&zghz3969, INT64_C(32));
      RECREATE(lbits)(&zghz3970);
      UNDEFINED(lbits)(&zghz3970, zghz3969);
      zICC_SRE_EL1_S = CONVERT_OF(fbits, lbits)(zghz3970, true);
    }
    unit zgsz3401;
    zgsz3401 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3967);
      CONVERT_OF(sail_int, mach_int)(&zghz3967, INT64_C(32));
      RECREATE(lbits)(&zghz3968);
      UNDEFINED(lbits)(&zghz3968, zghz3967);
      zICC_SRE_EL2 = CONVERT_OF(fbits, lbits)(zghz3968, true);
    }
    unit zgsz3400;
    zgsz3400 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3965);
      CONVERT_OF(sail_int, mach_int)(&zghz3965, INT64_C(32));
      RECREATE(lbits)(&zghz3966);
      UNDEFINED(lbits)(&zghz3966, zghz3965);
      zICC_SRE_EL3 = CONVERT_OF(fbits, lbits)(zghz3966, true);
    }
    unit zgsz3399;
    zgsz3399 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3963);
      CONVERT_OF(sail_int, mach_int)(&zghz3963, INT64_C(32));
      RECREATE(lbits)(&zghz3964);
      UNDEFINED(lbits)(&zghz3964, zghz3963);
      zICH_HCR_EL2 = CONVERT_OF(fbits, lbits)(zghz3964, true);
    }
    unit zgsz3398;
    zgsz3398 = UNIT;
  }
  {
    uint64_t zgaz3339;
    {
      RECREATE(sail_int)(&zghz3961);
      CONVERT_OF(sail_int, mach_int)(&zghz3961, INT64_C(32));
      RECREATE(lbits)(&zghz3962);
      UNDEFINED(lbits)(&zghz3962, zghz3961);
      zgaz3339 = CONVERT_OF(fbits, lbits)(zghz3962, true);
    }
    {
      RECREATE(sail_int)(&zghz3960);
      CONVERT_OF(sail_int, mach_int)(&zghz3960, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICV_AP0R_EL1, zghz3960, zgaz3339);
    }
    unit zgsz3397;
    zgsz3397 = UNIT;
  }
  {
    uint64_t zgaz3340;
    {
      RECREATE(sail_int)(&zghz3958);
      CONVERT_OF(sail_int, mach_int)(&zghz3958, INT64_C(32));
      RECREATE(lbits)(&zghz3959);
      UNDEFINED(lbits)(&zghz3959, zghz3958);
      zgaz3340 = CONVERT_OF(fbits, lbits)(zghz3959, true);
    }
    {
      RECREATE(sail_int)(&zghz3957);
      CONVERT_OF(sail_int, mach_int)(&zghz3957, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICC_AP1R_EL1, zghz3957, zgaz3340);
    }
    unit zgsz3396;
    zgsz3396 = UNIT;
  }
  {
    uint64_t zgaz3341;
    {
      RECREATE(sail_int)(&zghz3955);
      CONVERT_OF(sail_int, mach_int)(&zghz3955, INT64_C(32));
      RECREATE(lbits)(&zghz3956);
      UNDEFINED(lbits)(&zghz3956, zghz3955);
      zgaz3341 = CONVERT_OF(fbits, lbits)(zghz3956, true);
    }
    {
      RECREATE(sail_int)(&zghz3954);
      CONVERT_OF(sail_int, mach_int)(&zghz3954, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_NS, zghz3954, zgaz3341);
    }
    unit zgsz3395;
    zgsz3395 = UNIT;
  }
  {
    uint64_t zgaz3342;
    {
      RECREATE(sail_int)(&zghz3952);
      CONVERT_OF(sail_int, mach_int)(&zghz3952, INT64_C(32));
      RECREATE(lbits)(&zghz3953);
      UNDEFINED(lbits)(&zghz3953, zghz3952);
      zgaz3342 = CONVERT_OF(fbits, lbits)(zghz3953, true);
    }
    {
      RECREATE(sail_int)(&zghz3951);
      CONVERT_OF(sail_int, mach_int)(&zghz3951, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_S, zghz3951, zgaz3342);
    }
    unit zgsz3394;
    zgsz3394 = UNIT;
  }
  {
    uint64_t zgaz3343;
    {
      RECREATE(sail_int)(&zghz3949);
      CONVERT_OF(sail_int, mach_int)(&zghz3949, INT64_C(32));
      RECREATE(lbits)(&zghz3950);
      UNDEFINED(lbits)(&zghz3950, zghz3949);
      zgaz3343 = CONVERT_OF(fbits, lbits)(zghz3950, true);
    }
    {
      RECREATE(sail_int)(&zghz3948);
      CONVERT_OF(sail_int, mach_int)(&zghz3948, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICV_AP1R_EL1, zghz3948, zgaz3343);
    }
    unit zgsz3393;
    zgsz3393 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3946);
      CONVERT_OF(sail_int, mach_int)(&zghz3946, INT64_C(32));
      RECREATE(lbits)(&zghz3947);
      UNDEFINED(lbits)(&zghz3947, zghz3946);
      zICC_BPR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3947, true);
    }
    unit zgsz3392;
    zgsz3392 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3944);
      CONVERT_OF(sail_int, mach_int)(&zghz3944, INT64_C(32));
      RECREATE(lbits)(&zghz3945);
      UNDEFINED(lbits)(&zghz3945, zghz3944);
      zICV_BPR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3945, true);
    }
    unit zgsz3391;
    zgsz3391 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3942);
      CONVERT_OF(sail_int, mach_int)(&zghz3942, INT64_C(32));
      RECREATE(lbits)(&zghz3943);
      UNDEFINED(lbits)(&zghz3943, zghz3942);
      zICC_BPR1_EL1_NS = CONVERT_OF(fbits, lbits)(zghz3943, true);
    }
    unit zgsz3390;
    zgsz3390 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3940);
      CONVERT_OF(sail_int, mach_int)(&zghz3940, INT64_C(32));
      RECREATE(lbits)(&zghz3941);
      UNDEFINED(lbits)(&zghz3941, zghz3940);
      zICC_BPR1_EL1_S = CONVERT_OF(fbits, lbits)(zghz3941, true);
    }
    unit zgsz3389;
    zgsz3389 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3938);
      CONVERT_OF(sail_int, mach_int)(&zghz3938, INT64_C(32));
      RECREATE(lbits)(&zghz3939);
      UNDEFINED(lbits)(&zghz3939, zghz3938);
      zICV_BPR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3939, true);
    }
    unit zgsz3388;
    zgsz3388 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3936);
      CONVERT_OF(sail_int, mach_int)(&zghz3936, INT64_C(32));
      RECREATE(lbits)(&zghz3937);
      UNDEFINED(lbits)(&zghz3937, zghz3936);
      zICC_CTLR_EL1_NS = CONVERT_OF(fbits, lbits)(zghz3937, true);
    }
    unit zgsz3387;
    zgsz3387 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3934);
      CONVERT_OF(sail_int, mach_int)(&zghz3934, INT64_C(32));
      RECREATE(lbits)(&zghz3935);
      UNDEFINED(lbits)(&zghz3935, zghz3934);
      zICC_CTLR_EL1_S = CONVERT_OF(fbits, lbits)(zghz3935, true);
    }
    unit zgsz3386;
    zgsz3386 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3932);
      CONVERT_OF(sail_int, mach_int)(&zghz3932, INT64_C(32));
      RECREATE(lbits)(&zghz3933);
      UNDEFINED(lbits)(&zghz3933, zghz3932);
      zICV_CTLR_EL1 = CONVERT_OF(fbits, lbits)(zghz3933, true);
    }
    unit zgsz3385;
    zgsz3385 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3930);
      CONVERT_OF(sail_int, mach_int)(&zghz3930, INT64_C(32));
      RECREATE(lbits)(&zghz3931);
      UNDEFINED(lbits)(&zghz3931, zghz3930);
      zICC_CTLR_EL3 = CONVERT_OF(fbits, lbits)(zghz3931, true);
    }
    unit zgsz3384;
    zgsz3384 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3928);
      CONVERT_OF(sail_int, mach_int)(&zghz3928, INT64_C(32));
      RECREATE(lbits)(&zghz3929);
      UNDEFINED(lbits)(&zghz3929, zghz3928);
      zICC_HPPIR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3929, true);
    }
    unit zgsz3383;
    zgsz3383 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3926);
      CONVERT_OF(sail_int, mach_int)(&zghz3926, INT64_C(32));
      RECREATE(lbits)(&zghz3927);
      UNDEFINED(lbits)(&zghz3927, zghz3926);
      zICV_HPPIR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3927, true);
    }
    unit zgsz3382;
    zgsz3382 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3924);
      CONVERT_OF(sail_int, mach_int)(&zghz3924, INT64_C(32));
      RECREATE(lbits)(&zghz3925);
      UNDEFINED(lbits)(&zghz3925, zghz3924);
      zICC_HPPIR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3925, true);
    }
    unit zgsz3381;
    zgsz3381 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3922);
      CONVERT_OF(sail_int, mach_int)(&zghz3922, INT64_C(32));
      RECREATE(lbits)(&zghz3923);
      UNDEFINED(lbits)(&zghz3923, zghz3922);
      zICV_HPPIR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3923, true);
    }
    unit zgsz3380;
    zgsz3380 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3920);
      CONVERT_OF(sail_int, mach_int)(&zghz3920, INT64_C(32));
      RECREATE(lbits)(&zghz3921);
      UNDEFINED(lbits)(&zghz3921, zghz3920);
      zICC_IAR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3921, true);
    }
    unit zgsz3379;
    zgsz3379 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3918);
      CONVERT_OF(sail_int, mach_int)(&zghz3918, INT64_C(32));
      RECREATE(lbits)(&zghz3919);
      UNDEFINED(lbits)(&zghz3919, zghz3918);
      zICV_IAR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3919, true);
    }
    unit zgsz3378;
    zgsz3378 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3916);
      CONVERT_OF(sail_int, mach_int)(&zghz3916, INT64_C(32));
      RECREATE(lbits)(&zghz3917);
      UNDEFINED(lbits)(&zghz3917, zghz3916);
      zICC_IAR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3917, true);
    }
    unit zgsz3377;
    zgsz3377 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3914);
      CONVERT_OF(sail_int, mach_int)(&zghz3914, INT64_C(32));
      RECREATE(lbits)(&zghz3915);
      UNDEFINED(lbits)(&zghz3915, zghz3914);
      zICV_IAR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3915, true);
    }
    unit zgsz3376;
    zgsz3376 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3912);
      CONVERT_OF(sail_int, mach_int)(&zghz3912, INT64_C(32));
      RECREATE(lbits)(&zghz3913);
      UNDEFINED(lbits)(&zghz3913, zghz3912);
      zICC_IGRPEN0_EL1 = CONVERT_OF(fbits, lbits)(zghz3913, true);
    }
    unit zgsz3375;
    zgsz3375 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3910);
      CONVERT_OF(sail_int, mach_int)(&zghz3910, INT64_C(32));
      RECREATE(lbits)(&zghz3911);
      UNDEFINED(lbits)(&zghz3911, zghz3910);
      zICV_IGRPEN0_EL1 = CONVERT_OF(fbits, lbits)(zghz3911, true);
    }
    unit zgsz3374;
    zgsz3374 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3908);
      CONVERT_OF(sail_int, mach_int)(&zghz3908, INT64_C(32));
      RECREATE(lbits)(&zghz3909);
      UNDEFINED(lbits)(&zghz3909, zghz3908);
      zICC_IGRPEN1_EL1_NS = CONVERT_OF(fbits, lbits)(zghz3909, true);
    }
    unit zgsz3373;
    zgsz3373 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3906);
      CONVERT_OF(sail_int, mach_int)(&zghz3906, INT64_C(32));
      RECREATE(lbits)(&zghz3907);
      UNDEFINED(lbits)(&zghz3907, zghz3906);
      zICC_IGRPEN1_EL1_S = CONVERT_OF(fbits, lbits)(zghz3907, true);
    }
    unit zgsz3372;
    zgsz3372 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3904);
      CONVERT_OF(sail_int, mach_int)(&zghz3904, INT64_C(32));
      RECREATE(lbits)(&zghz3905);
      UNDEFINED(lbits)(&zghz3905, zghz3904);
      zICV_IGRPEN1_EL1 = CONVERT_OF(fbits, lbits)(zghz3905, true);
    }
    unit zgsz3371;
    zgsz3371 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3902);
      CONVERT_OF(sail_int, mach_int)(&zghz3902, INT64_C(32));
      RECREATE(lbits)(&zghz3903);
      UNDEFINED(lbits)(&zghz3903, zghz3902);
      zICC_IGRPEN1_EL3 = CONVERT_OF(fbits, lbits)(zghz3903, true);
    }
    unit zgsz3370;
    zgsz3370 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3900);
      CONVERT_OF(sail_int, mach_int)(&zghz3900, INT64_C(32));
      RECREATE(lbits)(&zghz3901);
      UNDEFINED(lbits)(&zghz3901, zghz3900);
      zICC_PMR_EL1 = CONVERT_OF(fbits, lbits)(zghz3901, true);
    }
    unit zgsz3369;
    zgsz3369 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3898);
      CONVERT_OF(sail_int, mach_int)(&zghz3898, INT64_C(32));
      RECREATE(lbits)(&zghz3899);
      UNDEFINED(lbits)(&zghz3899, zghz3898);
      zICV_PMR_EL1 = CONVERT_OF(fbits, lbits)(zghz3899, true);
    }
    unit zgsz3368;
    zgsz3368 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3896);
      CONVERT_OF(sail_int, mach_int)(&zghz3896, INT64_C(32));
      RECREATE(lbits)(&zghz3897);
      UNDEFINED(lbits)(&zghz3897, zghz3896);
      zICC_RPR_EL1 = CONVERT_OF(fbits, lbits)(zghz3897, true);
    }
    unit zgsz3367;
    zgsz3367 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3894);
      CONVERT_OF(sail_int, mach_int)(&zghz3894, INT64_C(32));
      RECREATE(lbits)(&zghz3895);
      UNDEFINED(lbits)(&zghz3895, zghz3894);
      zICV_RPR_EL1 = CONVERT_OF(fbits, lbits)(zghz3895, true);
    }
    unit zgsz3366;
    zgsz3366 = UNIT;
  }
  {
    uint64_t zgaz3344;
    {
      RECREATE(sail_int)(&zghz3892);
      CONVERT_OF(sail_int, mach_int)(&zghz3892, INT64_C(32));
      RECREATE(lbits)(&zghz3893);
      UNDEFINED(lbits)(&zghz3893, zghz3892);
      zgaz3344 = CONVERT_OF(fbits, lbits)(zghz3893, true);
    }
    {
      RECREATE(sail_int)(&zghz3891);
      CONVERT_OF(sail_int, mach_int)(&zghz3891, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICH_AP0R_EL2, zghz3891, zgaz3344);
    }
    unit zgsz3365;
    zgsz3365 = UNIT;
  }
  {
    uint64_t zgaz3345;
    {
      RECREATE(sail_int)(&zghz3889);
      CONVERT_OF(sail_int, mach_int)(&zghz3889, INT64_C(32));
      RECREATE(lbits)(&zghz3890);
      UNDEFINED(lbits)(&zghz3890, zghz3889);
      zgaz3345 = CONVERT_OF(fbits, lbits)(zghz3890, true);
    }
    {
      RECREATE(sail_int)(&zghz3888);
      CONVERT_OF(sail_int, mach_int)(&zghz3888, INT64_C(4));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zICH_AP1R_EL2, zghz3888, zgaz3345);
    }
    unit zgsz3364;
    zgsz3364 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3886);
      CONVERT_OF(sail_int, mach_int)(&zghz3886, INT64_C(32));
      RECREATE(lbits)(&zghz3887);
      UNDEFINED(lbits)(&zghz3887, zghz3886);
      zICH_EISR_EL2 = CONVERT_OF(fbits, lbits)(zghz3887, true);
    }
    unit zgsz3363;
    zgsz3363 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3884);
      CONVERT_OF(sail_int, mach_int)(&zghz3884, INT64_C(32));
      RECREATE(lbits)(&zghz3885);
      UNDEFINED(lbits)(&zghz3885, zghz3884);
      zICH_ELRSR_EL2 = CONVERT_OF(fbits, lbits)(zghz3885, true);
    }
    unit zgsz3362;
    zgsz3362 = UNIT;
  }
  {
    uint64_t zgaz3346;
    {
      RECREATE(sail_int)(&zghz3882);
      CONVERT_OF(sail_int, mach_int)(&zghz3882, INT64_C(64));
      RECREATE(lbits)(&zghz3883);
      UNDEFINED(lbits)(&zghz3883, zghz3882);
      zgaz3346 = CONVERT_OF(fbits, lbits)(zghz3883, true);
    }
    {
      RECREATE(sail_int)(&zghz3881);
      CONVERT_OF(sail_int, mach_int)(&zghz3881, INT64_C(16));
      UNDEFINED(vector_zz5vecz8z5bv64z9)(&zICH_LR_EL2, zghz3881, zgaz3346);
    }
    unit zgsz3361;
    zgsz3361 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3879);
      CONVERT_OF(sail_int, mach_int)(&zghz3879, INT64_C(32));
      RECREATE(lbits)(&zghz3880);
      UNDEFINED(lbits)(&zghz3880, zghz3879);
      zICH_MISR_EL2 = CONVERT_OF(fbits, lbits)(zghz3880, true);
    }
    unit zgsz3360;
    zgsz3360 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3877);
      CONVERT_OF(sail_int, mach_int)(&zghz3877, INT64_C(32));
      RECREATE(lbits)(&zghz3878);
      UNDEFINED(lbits)(&zghz3878, zghz3877);
      zICH_VMCR_EL2 = CONVERT_OF(fbits, lbits)(zghz3878, true);
    }
    unit zgsz3359;
    zgsz3359 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3875);
      CONVERT_OF(sail_int, mach_int)(&zghz3875, INT64_C(32));
      RECREATE(lbits)(&zghz3876);
      UNDEFINED(lbits)(&zghz3876, zghz3875);
      zICH_VTR_EL2 = CONVERT_OF(fbits, lbits)(zghz3876, true);
    }
    unit zgsz3358;
    zgsz3358 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3873);
      CONVERT_OF(sail_int, mach_int)(&zghz3873, INT64_C(32));
      RECREATE(lbits)(&zghz3874);
      UNDEFINED(lbits)(&zghz3874, zghz3873);
      zID_AA64AFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3874, true);
    }
    unit zgsz3357;
    zgsz3357 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3871);
      CONVERT_OF(sail_int, mach_int)(&zghz3871, INT64_C(32));
      RECREATE(lbits)(&zghz3872);
      UNDEFINED(lbits)(&zghz3872, zghz3871);
      zID_AA64AFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3872, true);
    }
    unit zgsz3356;
    zgsz3356 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3869);
      CONVERT_OF(sail_int, mach_int)(&zghz3869, INT64_C(64));
      RECREATE(lbits)(&zghz3870);
      UNDEFINED(lbits)(&zghz3870, zghz3869);
      zID_AA64DFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3870, true);
    }
    unit zgsz3355;
    zgsz3355 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3867);
      CONVERT_OF(sail_int, mach_int)(&zghz3867, INT64_C(32));
      RECREATE(lbits)(&zghz3868);
      UNDEFINED(lbits)(&zghz3868, zghz3867);
      zID_AA64DFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3868, true);
    }
    unit zgsz3354;
    zgsz3354 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3865);
      CONVERT_OF(sail_int, mach_int)(&zghz3865, INT64_C(64));
      RECREATE(lbits)(&zghz3866);
      UNDEFINED(lbits)(&zghz3866, zghz3865);
      zID_AA64ISAR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3866, true);
    }
    unit zgsz3353;
    zgsz3353 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3863);
      CONVERT_OF(sail_int, mach_int)(&zghz3863, INT64_C(64));
      RECREATE(lbits)(&zghz3864);
      UNDEFINED(lbits)(&zghz3864, zghz3863);
      zID_AA64ISAR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3864, true);
    }
    unit zgsz3352;
    zgsz3352 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3861);
      CONVERT_OF(sail_int, mach_int)(&zghz3861, INT64_C(32));
      RECREATE(lbits)(&zghz3862);
      UNDEFINED(lbits)(&zghz3862, zghz3861);
      zID_AA64MMFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3862, true);
    }
    unit zgsz3351;
    zgsz3351 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3859);
      CONVERT_OF(sail_int, mach_int)(&zghz3859, INT64_C(64));
      RECREATE(lbits)(&zghz3860);
      UNDEFINED(lbits)(&zghz3860, zghz3859);
      zID_AA64MMFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3860, true);
    }
    unit zgsz3350;
    zgsz3350 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3857);
      CONVERT_OF(sail_int, mach_int)(&zghz3857, INT64_C(64));
      RECREATE(lbits)(&zghz3858);
      UNDEFINED(lbits)(&zghz3858, zghz3857);
      zID_AA64MMFR2_EL1 = CONVERT_OF(fbits, lbits)(zghz3858, true);
    }
    unit zgsz3349;
    zgsz3349 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3855);
      CONVERT_OF(sail_int, mach_int)(&zghz3855, INT64_C(64));
      RECREATE(lbits)(&zghz3856);
      UNDEFINED(lbits)(&zghz3856, zghz3855);
      zID_AA64PFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3856, true);
    }
    unit zgsz3348;
    zgsz3348 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3853);
      CONVERT_OF(sail_int, mach_int)(&zghz3853, INT64_C(32));
      RECREATE(lbits)(&zghz3854);
      UNDEFINED(lbits)(&zghz3854, zghz3853);
      zID_AA64PFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3854, true);
    }
    unit zgsz3347;
    zgsz3347 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3851);
      CONVERT_OF(sail_int, mach_int)(&zghz3851, INT64_C(64));
      RECREATE(lbits)(&zghz3852);
      UNDEFINED(lbits)(&zghz3852, zghz3851);
      zID_AA64ZFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3852, true);
    }
    unit zgsz3346;
    zgsz3346 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3849);
      CONVERT_OF(sail_int, mach_int)(&zghz3849, INT64_C(32));
      RECREATE(lbits)(&zghz3850);
      UNDEFINED(lbits)(&zghz3850, zghz3849);
      zID_AFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3850, true);
    }
    unit zgsz3345;
    zgsz3345 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3847);
      CONVERT_OF(sail_int, mach_int)(&zghz3847, INT64_C(32));
      RECREATE(lbits)(&zghz3848);
      UNDEFINED(lbits)(&zghz3848, zghz3847);
      zID_DFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3848, true);
    }
    unit zgsz3344;
    zgsz3344 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3845);
      CONVERT_OF(sail_int, mach_int)(&zghz3845, INT64_C(32));
      RECREATE(lbits)(&zghz3846);
      UNDEFINED(lbits)(&zghz3846, zghz3845);
      zID_ISAR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3846, true);
    }
    unit zgsz3343;
    zgsz3343 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3843);
      CONVERT_OF(sail_int, mach_int)(&zghz3843, INT64_C(32));
      RECREATE(lbits)(&zghz3844);
      UNDEFINED(lbits)(&zghz3844, zghz3843);
      zID_ISAR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3844, true);
    }
    unit zgsz3342;
    zgsz3342 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3841);
      CONVERT_OF(sail_int, mach_int)(&zghz3841, INT64_C(32));
      RECREATE(lbits)(&zghz3842);
      UNDEFINED(lbits)(&zghz3842, zghz3841);
      zID_ISAR2_EL1 = CONVERT_OF(fbits, lbits)(zghz3842, true);
    }
    unit zgsz3341;
    zgsz3341 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3839);
      CONVERT_OF(sail_int, mach_int)(&zghz3839, INT64_C(32));
      RECREATE(lbits)(&zghz3840);
      UNDEFINED(lbits)(&zghz3840, zghz3839);
      zID_ISAR3_EL1 = CONVERT_OF(fbits, lbits)(zghz3840, true);
    }
    unit zgsz3340;
    zgsz3340 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3837);
      CONVERT_OF(sail_int, mach_int)(&zghz3837, INT64_C(32));
      RECREATE(lbits)(&zghz3838);
      UNDEFINED(lbits)(&zghz3838, zghz3837);
      zID_ISAR4_EL1 = CONVERT_OF(fbits, lbits)(zghz3838, true);
    }
    unit zgsz3339;
    zgsz3339 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3835);
      CONVERT_OF(sail_int, mach_int)(&zghz3835, INT64_C(32));
      RECREATE(lbits)(&zghz3836);
      UNDEFINED(lbits)(&zghz3836, zghz3835);
      zID_ISAR5_EL1 = CONVERT_OF(fbits, lbits)(zghz3836, true);
    }
    unit zgsz3338;
    zgsz3338 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3833);
      CONVERT_OF(sail_int, mach_int)(&zghz3833, INT64_C(32));
      RECREATE(lbits)(&zghz3834);
      UNDEFINED(lbits)(&zghz3834, zghz3833);
      zID_ISAR6_EL1 = CONVERT_OF(fbits, lbits)(zghz3834, true);
    }
    unit zgsz3337;
    zgsz3337 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3831);
      CONVERT_OF(sail_int, mach_int)(&zghz3831, INT64_C(32));
      RECREATE(lbits)(&zghz3832);
      UNDEFINED(lbits)(&zghz3832, zghz3831);
      zID_MMFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3832, true);
    }
    unit zgsz3336;
    zgsz3336 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3829);
      CONVERT_OF(sail_int, mach_int)(&zghz3829, INT64_C(32));
      RECREATE(lbits)(&zghz3830);
      UNDEFINED(lbits)(&zghz3830, zghz3829);
      zID_MMFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3830, true);
    }
    unit zgsz3335;
    zgsz3335 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3827);
      CONVERT_OF(sail_int, mach_int)(&zghz3827, INT64_C(32));
      RECREATE(lbits)(&zghz3828);
      UNDEFINED(lbits)(&zghz3828, zghz3827);
      zID_MMFR2_EL1 = CONVERT_OF(fbits, lbits)(zghz3828, true);
    }
    unit zgsz3334;
    zgsz3334 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3825);
      CONVERT_OF(sail_int, mach_int)(&zghz3825, INT64_C(32));
      RECREATE(lbits)(&zghz3826);
      UNDEFINED(lbits)(&zghz3826, zghz3825);
      zID_MMFR3_EL1 = CONVERT_OF(fbits, lbits)(zghz3826, true);
    }
    unit zgsz3333;
    zgsz3333 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3823);
      CONVERT_OF(sail_int, mach_int)(&zghz3823, INT64_C(32));
      RECREATE(lbits)(&zghz3824);
      UNDEFINED(lbits)(&zghz3824, zghz3823);
      zID_MMFR4_EL1 = CONVERT_OF(fbits, lbits)(zghz3824, true);
    }
    unit zgsz3332;
    zgsz3332 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3821);
      CONVERT_OF(sail_int, mach_int)(&zghz3821, INT64_C(32));
      RECREATE(lbits)(&zghz3822);
      UNDEFINED(lbits)(&zghz3822, zghz3821);
      zID_MMFR5_EL1 = CONVERT_OF(fbits, lbits)(zghz3822, true);
    }
    unit zgsz3331;
    zgsz3331 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3819);
      CONVERT_OF(sail_int, mach_int)(&zghz3819, INT64_C(32));
      RECREATE(lbits)(&zghz3820);
      UNDEFINED(lbits)(&zghz3820, zghz3819);
      zID_PFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3820, true);
    }
    unit zgsz3330;
    zgsz3330 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3817);
      CONVERT_OF(sail_int, mach_int)(&zghz3817, INT64_C(32));
      RECREATE(lbits)(&zghz3818);
      UNDEFINED(lbits)(&zghz3818, zghz3817);
      zID_PFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3818, true);
    }
    unit zgsz3329;
    zgsz3329 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3815);
      CONVERT_OF(sail_int, mach_int)(&zghz3815, INT64_C(32));
      RECREATE(lbits)(&zghz3816);
      UNDEFINED(lbits)(&zghz3816, zghz3815);
      zID_PFR2_EL1 = CONVERT_OF(fbits, lbits)(zghz3816, true);
    }
    unit zgsz3328;
    zgsz3328 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3813);
      CONVERT_OF(sail_int, mach_int)(&zghz3813, INT64_C(32));
      RECREATE(lbits)(&zghz3814);
      UNDEFINED(lbits)(&zghz3814, zghz3813);
      zIFSR32_EL2 = CONVERT_OF(fbits, lbits)(zghz3814, true);
    }
    unit zgsz3327;
    zgsz3327 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3811);
      CONVERT_OF(sail_int, mach_int)(&zghz3811, INT64_C(32));
      RECREATE(lbits)(&zghz3812);
      UNDEFINED(lbits)(&zghz3812, zghz3811);
      zISR_EL1 = CONVERT_OF(fbits, lbits)(zghz3812, true);
    }
    unit zgsz3326;
    zgsz3326 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3809);
      CONVERT_OF(sail_int, mach_int)(&zghz3809, INT64_C(32));
      RECREATE(lbits)(&zghz3810);
      UNDEFINED(lbits)(&zghz3810, zghz3809);
      zLORC_EL1 = CONVERT_OF(fbits, lbits)(zghz3810, true);
    }
    unit zgsz3325;
    zgsz3325 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3807);
      CONVERT_OF(sail_int, mach_int)(&zghz3807, INT64_C(64));
      RECREATE(lbits)(&zghz3808);
      UNDEFINED(lbits)(&zghz3808, zghz3807);
      zLOREA_EL1 = CONVERT_OF(fbits, lbits)(zghz3808, true);
    }
    unit zgsz3324;
    zgsz3324 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3805);
      CONVERT_OF(sail_int, mach_int)(&zghz3805, INT64_C(32));
      RECREATE(lbits)(&zghz3806);
      UNDEFINED(lbits)(&zghz3806, zghz3805);
      zLORID_EL1 = CONVERT_OF(fbits, lbits)(zghz3806, true);
    }
    unit zgsz3323;
    zgsz3323 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3803);
      CONVERT_OF(sail_int, mach_int)(&zghz3803, INT64_C(32));
      RECREATE(lbits)(&zghz3804);
      UNDEFINED(lbits)(&zghz3804, zghz3803);
      zLORN_EL1 = CONVERT_OF(fbits, lbits)(zghz3804, true);
    }
    unit zgsz3322;
    zgsz3322 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3801);
      CONVERT_OF(sail_int, mach_int)(&zghz3801, INT64_C(64));
      RECREATE(lbits)(&zghz3802);
      UNDEFINED(lbits)(&zghz3802, zghz3801);
      zLORSA_EL1 = CONVERT_OF(fbits, lbits)(zghz3802, true);
    }
    unit zgsz3321;
    zgsz3321 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3799);
      CONVERT_OF(sail_int, mach_int)(&zghz3799, INT64_C(64));
      RECREATE(lbits)(&zghz3800);
      UNDEFINED(lbits)(&zghz3800, zghz3799);
      zMAIR_EL1 = CONVERT_OF(fbits, lbits)(zghz3800, true);
    }
    unit zgsz3320;
    zgsz3320 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3797);
      CONVERT_OF(sail_int, mach_int)(&zghz3797, INT64_C(64));
      RECREATE(lbits)(&zghz3798);
      UNDEFINED(lbits)(&zghz3798, zghz3797);
      zMAIR_EL2 = CONVERT_OF(fbits, lbits)(zghz3798, true);
    }
    unit zgsz3319;
    zgsz3319 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3795);
      CONVERT_OF(sail_int, mach_int)(&zghz3795, INT64_C(64));
      RECREATE(lbits)(&zghz3796);
      UNDEFINED(lbits)(&zghz3796, zghz3795);
      zMAIR_EL3 = CONVERT_OF(fbits, lbits)(zghz3796, true);
    }
    unit zgsz3318;
    zgsz3318 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3793);
      CONVERT_OF(sail_int, mach_int)(&zghz3793, INT64_C(32));
      RECREATE(lbits)(&zghz3794);
      UNDEFINED(lbits)(&zghz3794, zghz3793);
      zMDCCINT_EL1 = CONVERT_OF(fbits, lbits)(zghz3794, true);
    }
    unit zgsz3317;
    zgsz3317 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3791);
      CONVERT_OF(sail_int, mach_int)(&zghz3791, INT64_C(32));
      RECREATE(lbits)(&zghz3792);
      UNDEFINED(lbits)(&zghz3792, zghz3791);
      zMDCCSR_EL0 = CONVERT_OF(fbits, lbits)(zghz3792, true);
    }
    unit zgsz3316;
    zgsz3316 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3789);
      CONVERT_OF(sail_int, mach_int)(&zghz3789, INT64_C(64));
      RECREATE(lbits)(&zghz3790);
      UNDEFINED(lbits)(&zghz3790, zghz3789);
      zMDRAR_EL1 = CONVERT_OF(fbits, lbits)(zghz3790, true);
    }
    unit zgsz3315;
    zgsz3315 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3787);
      CONVERT_OF(sail_int, mach_int)(&zghz3787, INT64_C(32));
      RECREATE(lbits)(&zghz3788);
      UNDEFINED(lbits)(&zghz3788, zghz3787);
      zMIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3788, true);
    }
    unit zgsz3314;
    zgsz3314 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3785);
      CONVERT_OF(sail_int, mach_int)(&zghz3785, INT64_C(32));
      RECREATE(lbits)(&zghz3786);
      UNDEFINED(lbits)(&zghz3786, zghz3785);
      zVPIDR_EL2 = CONVERT_OF(fbits, lbits)(zghz3786, true);
    }
    unit zgsz3313;
    zgsz3313 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3783);
      CONVERT_OF(sail_int, mach_int)(&zghz3783, INT64_C(64));
      RECREATE(lbits)(&zghz3784);
      UNDEFINED(lbits)(&zghz3784, zghz3783);
      zMPAM0_EL1 = CONVERT_OF(fbits, lbits)(zghz3784, true);
    }
    unit zgsz3312;
    zgsz3312 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3781);
      CONVERT_OF(sail_int, mach_int)(&zghz3781, INT64_C(64));
      RECREATE(lbits)(&zghz3782);
      UNDEFINED(lbits)(&zghz3782, zghz3781);
      zMPAM3_EL3 = CONVERT_OF(fbits, lbits)(zghz3782, true);
    }
    unit zgsz3311;
    zgsz3311 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3779);
      CONVERT_OF(sail_int, mach_int)(&zghz3779, INT64_C(63));
      RECREATE(lbits)(&zghz3780);
      UNDEFINED(lbits)(&zghz3780, zghz3779);
      z_MPAM2_EL2_0_62 = CONVERT_OF(fbits, lbits)(zghz3780, true);
    }
    unit zgsz3310;
    zgsz3310 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3777);
      CONVERT_OF(sail_int, mach_int)(&zghz3777, INT64_C(63));
      RECREATE(lbits)(&zghz3778);
      UNDEFINED(lbits)(&zghz3778, zghz3777);
      z_MPAM1_EL1_0_62 = CONVERT_OF(fbits, lbits)(zghz3778, true);
    }
    unit zgsz3309;
    zgsz3309 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3775);
      CONVERT_OF(sail_int, mach_int)(&zghz3775, INT64_C(32));
      RECREATE(lbits)(&zghz3776);
      UNDEFINED(lbits)(&zghz3776, zghz3775);
      zMPAMHCR_EL2 = CONVERT_OF(fbits, lbits)(zghz3776, true);
    }
    unit zgsz3308;
    zgsz3308 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3773);
      CONVERT_OF(sail_int, mach_int)(&zghz3773, INT64_C(64));
      RECREATE(lbits)(&zghz3774);
      UNDEFINED(lbits)(&zghz3774, zghz3773);
      zMPAMIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3774, true);
    }
    unit zgsz3307;
    zgsz3307 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3771);
      CONVERT_OF(sail_int, mach_int)(&zghz3771, INT64_C(64));
      RECREATE(lbits)(&zghz3772);
      UNDEFINED(lbits)(&zghz3772, zghz3771);
      zMPAMVPM0_EL2 = CONVERT_OF(fbits, lbits)(zghz3772, true);
    }
    unit zgsz3306;
    zgsz3306 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3769);
      CONVERT_OF(sail_int, mach_int)(&zghz3769, INT64_C(64));
      RECREATE(lbits)(&zghz3770);
      UNDEFINED(lbits)(&zghz3770, zghz3769);
      zMPAMVPM1_EL2 = CONVERT_OF(fbits, lbits)(zghz3770, true);
    }
    unit zgsz3305;
    zgsz3305 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3767);
      CONVERT_OF(sail_int, mach_int)(&zghz3767, INT64_C(64));
      RECREATE(lbits)(&zghz3768);
      UNDEFINED(lbits)(&zghz3768, zghz3767);
      zMPAMVPM2_EL2 = CONVERT_OF(fbits, lbits)(zghz3768, true);
    }
    unit zgsz3304;
    zgsz3304 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3765);
      CONVERT_OF(sail_int, mach_int)(&zghz3765, INT64_C(64));
      RECREATE(lbits)(&zghz3766);
      UNDEFINED(lbits)(&zghz3766, zghz3765);
      zMPAMVPM3_EL2 = CONVERT_OF(fbits, lbits)(zghz3766, true);
    }
    unit zgsz3303;
    zgsz3303 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3763);
      CONVERT_OF(sail_int, mach_int)(&zghz3763, INT64_C(64));
      RECREATE(lbits)(&zghz3764);
      UNDEFINED(lbits)(&zghz3764, zghz3763);
      zMPAMVPM4_EL2 = CONVERT_OF(fbits, lbits)(zghz3764, true);
    }
    unit zgsz3302;
    zgsz3302 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3761);
      CONVERT_OF(sail_int, mach_int)(&zghz3761, INT64_C(64));
      RECREATE(lbits)(&zghz3762);
      UNDEFINED(lbits)(&zghz3762, zghz3761);
      zMPAMVPM5_EL2 = CONVERT_OF(fbits, lbits)(zghz3762, true);
    }
    unit zgsz3301;
    zgsz3301 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3759);
      CONVERT_OF(sail_int, mach_int)(&zghz3759, INT64_C(64));
      RECREATE(lbits)(&zghz3760);
      UNDEFINED(lbits)(&zghz3760, zghz3759);
      zMPAMVPM6_EL2 = CONVERT_OF(fbits, lbits)(zghz3760, true);
    }
    unit zgsz3300;
    zgsz3300 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3757);
      CONVERT_OF(sail_int, mach_int)(&zghz3757, INT64_C(64));
      RECREATE(lbits)(&zghz3758);
      UNDEFINED(lbits)(&zghz3758, zghz3757);
      zMPAMVPM7_EL2 = CONVERT_OF(fbits, lbits)(zghz3758, true);
    }
    unit zgsz3299;
    zgsz3299 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3755);
      CONVERT_OF(sail_int, mach_int)(&zghz3755, INT64_C(32));
      RECREATE(lbits)(&zghz3756);
      UNDEFINED(lbits)(&zghz3756, zghz3755);
      zMPAMVPMV_EL2 = CONVERT_OF(fbits, lbits)(zghz3756, true);
    }
    unit zgsz3298;
    zgsz3298 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3753);
      CONVERT_OF(sail_int, mach_int)(&zghz3753, INT64_C(64));
      RECREATE(lbits)(&zghz3754);
      UNDEFINED(lbits)(&zghz3754, zghz3753);
      zMPIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3754, true);
    }
    unit zgsz3297;
    zgsz3297 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3751);
      CONVERT_OF(sail_int, mach_int)(&zghz3751, INT64_C(64));
      RECREATE(lbits)(&zghz3752);
      UNDEFINED(lbits)(&zghz3752, zghz3751);
      zVMPIDR_EL2 = CONVERT_OF(fbits, lbits)(zghz3752, true);
    }
    unit zgsz3296;
    zgsz3296 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3749);
      CONVERT_OF(sail_int, mach_int)(&zghz3749, INT64_C(32));
      RECREATE(lbits)(&zghz3750);
      UNDEFINED(lbits)(&zghz3750, zghz3749);
      zMVFR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3750, true);
    }
    unit zgsz3295;
    zgsz3295 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3747);
      CONVERT_OF(sail_int, mach_int)(&zghz3747, INT64_C(32));
      RECREATE(lbits)(&zghz3748);
      UNDEFINED(lbits)(&zghz3748, zghz3747);
      zMVFR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3748, true);
    }
    unit zgsz3294;
    zgsz3294 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3745);
      CONVERT_OF(sail_int, mach_int)(&zghz3745, INT64_C(32));
      RECREATE(lbits)(&zghz3746);
      UNDEFINED(lbits)(&zghz3746, zghz3745);
      zMVFR2_EL1 = CONVERT_OF(fbits, lbits)(zghz3746, true);
    }
    unit zgsz3293;
    zgsz3293 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3743);
      CONVERT_OF(sail_int, mach_int)(&zghz3743, INT64_C(32));
      RECREATE(lbits)(&zghz3744);
      UNDEFINED(lbits)(&zghz3744, zghz3743);
      zOSDTRRX_EL1 = CONVERT_OF(fbits, lbits)(zghz3744, true);
    }
    unit zgsz3292;
    zgsz3292 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3741);
      CONVERT_OF(sail_int, mach_int)(&zghz3741, INT64_C(32));
      RECREATE(lbits)(&zghz3742);
      UNDEFINED(lbits)(&zghz3742, zghz3741);
      zOSDTRTX_EL1 = CONVERT_OF(fbits, lbits)(zghz3742, true);
    }
    unit zgsz3291;
    zgsz3291 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3739);
      CONVERT_OF(sail_int, mach_int)(&zghz3739, INT64_C(32));
      RECREATE(lbits)(&zghz3740);
      UNDEFINED(lbits)(&zghz3740, zghz3739);
      zOSECCR_EL1 = CONVERT_OF(fbits, lbits)(zghz3740, true);
    }
    unit zgsz3290;
    zgsz3290 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3737);
      CONVERT_OF(sail_int, mach_int)(&zghz3737, INT64_C(64));
      RECREATE(lbits)(&zghz3738);
      UNDEFINED(lbits)(&zghz3738, zghz3737);
      zPAR_EL1 = CONVERT_OF(fbits, lbits)(zghz3738, true);
    }
    unit zgsz3289;
    zgsz3289 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3735);
      CONVERT_OF(sail_int, mach_int)(&zghz3735, INT64_C(32));
      RECREATE(lbits)(&zghz3736);
      UNDEFINED(lbits)(&zghz3736, zghz3735);
      zPMBIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3736, true);
    }
    unit zgsz3288;
    zgsz3288 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3733);
      CONVERT_OF(sail_int, mach_int)(&zghz3733, INT64_C(64));
      RECREATE(lbits)(&zghz3734);
      UNDEFINED(lbits)(&zghz3734, zghz3733);
      zPMBLIMITR_EL1 = CONVERT_OF(fbits, lbits)(zghz3734, true);
    }
    unit zgsz3287;
    zgsz3287 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3731);
      CONVERT_OF(sail_int, mach_int)(&zghz3731, INT64_C(64));
      RECREATE(lbits)(&zghz3732);
      UNDEFINED(lbits)(&zghz3732, zghz3731);
      zPMBPTR_EL1 = CONVERT_OF(fbits, lbits)(zghz3732, true);
    }
    unit zgsz3286;
    zgsz3286 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3729);
      CONVERT_OF(sail_int, mach_int)(&zghz3729, INT64_C(32));
      RECREATE(lbits)(&zghz3730);
      UNDEFINED(lbits)(&zghz3730, zghz3729);
      zPMBSR_EL1 = CONVERT_OF(fbits, lbits)(zghz3730, true);
    }
    unit zgsz3285;
    zgsz3285 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3727);
      CONVERT_OF(sail_int, mach_int)(&zghz3727, INT64_C(32));
      RECREATE(lbits)(&zghz3728);
      UNDEFINED(lbits)(&zghz3728, zghz3727);
      zPMCCFILTR_EL0 = CONVERT_OF(fbits, lbits)(zghz3728, true);
    }
    unit zgsz3284;
    zgsz3284 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3725);
      CONVERT_OF(sail_int, mach_int)(&zghz3725, INT64_C(32));
      RECREATE(lbits)(&zghz3726);
      UNDEFINED(lbits)(&zghz3726, zghz3725);
      zPMUSERENR_EL0 = CONVERT_OF(fbits, lbits)(zghz3726, true);
    }
    unit zgsz3283;
    zgsz3283 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3723);
      CONVERT_OF(sail_int, mach_int)(&zghz3723, INT64_C(64));
      RECREATE(lbits)(&zghz3724);
      UNDEFINED(lbits)(&zghz3724, zghz3723);
      zPMCCNTR_EL0 = CONVERT_OF(fbits, lbits)(zghz3724, true);
    }
    unit zgsz3282;
    zgsz3282 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3721);
      CONVERT_OF(sail_int, mach_int)(&zghz3721, INT64_C(64));
      RECREATE(lbits)(&zghz3722);
      UNDEFINED(lbits)(&zghz3722, zghz3721);
      zPMCEID0_EL0 = CONVERT_OF(fbits, lbits)(zghz3722, true);
    }
    unit zgsz3281;
    zgsz3281 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3719);
      CONVERT_OF(sail_int, mach_int)(&zghz3719, INT64_C(64));
      RECREATE(lbits)(&zghz3720);
      UNDEFINED(lbits)(&zghz3720, zghz3719);
      zPMCEID1_EL0 = CONVERT_OF(fbits, lbits)(zghz3720, true);
    }
    unit zgsz3280;
    zgsz3280 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3717);
      CONVERT_OF(sail_int, mach_int)(&zghz3717, INT64_C(32));
      RECREATE(lbits)(&zghz3718);
      UNDEFINED(lbits)(&zghz3718, zghz3717);
      zPMCNTENCLR_EL0 = CONVERT_OF(fbits, lbits)(zghz3718, true);
    }
    unit zgsz3279;
    zgsz3279 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3715);
      CONVERT_OF(sail_int, mach_int)(&zghz3715, INT64_C(32));
      RECREATE(lbits)(&zghz3716);
      UNDEFINED(lbits)(&zghz3716, zghz3715);
      zPMCNTENSET_EL0 = CONVERT_OF(fbits, lbits)(zghz3716, true);
    }
    unit zgsz3278;
    zgsz3278 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3713);
      CONVERT_OF(sail_int, mach_int)(&zghz3713, INT64_C(32));
      RECREATE(lbits)(&zghz3714);
      UNDEFINED(lbits)(&zghz3714, zghz3713);
      zPMCR_EL0 = CONVERT_OF(fbits, lbits)(zghz3714, true);
    }
    unit zgsz3277;
    zgsz3277 = UNIT;
  }
  {
    uint64_t zgaz3347;
    {
      RECREATE(sail_int)(&zghz3711);
      CONVERT_OF(sail_int, mach_int)(&zghz3711, INT64_C(32));
      RECREATE(lbits)(&zghz3712);
      UNDEFINED(lbits)(&zghz3712, zghz3711);
      zgaz3347 = CONVERT_OF(fbits, lbits)(zghz3712, true);
    }
    {
      RECREATE(sail_int)(&zghz3710);
      CONVERT_OF(sail_int, mach_int)(&zghz3710, INT64_C(31));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zPMEVCNTR_EL0, zghz3710, zgaz3347);
    }
    unit zgsz3276;
    zgsz3276 = UNIT;
  }
  {
    uint64_t zgaz3348;
    {
      RECREATE(sail_int)(&zghz3708);
      CONVERT_OF(sail_int, mach_int)(&zghz3708, INT64_C(32));
      RECREATE(lbits)(&zghz3709);
      UNDEFINED(lbits)(&zghz3709, zghz3708);
      zgaz3348 = CONVERT_OF(fbits, lbits)(zghz3709, true);
    }
    {
      RECREATE(sail_int)(&zghz3707);
      CONVERT_OF(sail_int, mach_int)(&zghz3707, INT64_C(31));
      UNDEFINED(vector_zz5vecz8z5bv32z9)(&zPMEVTYPER_EL0, zghz3707, zgaz3348);
    }
    unit zgsz3275;
    zgsz3275 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3705);
      CONVERT_OF(sail_int, mach_int)(&zghz3705, INT64_C(32));
      RECREATE(lbits)(&zghz3706);
      UNDEFINED(lbits)(&zghz3706, zghz3705);
      zPMINTENCLR_EL1 = CONVERT_OF(fbits, lbits)(zghz3706, true);
    }
    unit zgsz3274;
    zgsz3274 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3703);
      CONVERT_OF(sail_int, mach_int)(&zghz3703, INT64_C(32));
      RECREATE(lbits)(&zghz3704);
      UNDEFINED(lbits)(&zghz3704, zghz3703);
      zPMINTENSET_EL1 = CONVERT_OF(fbits, lbits)(zghz3704, true);
    }
    unit zgsz3273;
    zgsz3273 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3701);
      CONVERT_OF(sail_int, mach_int)(&zghz3701, INT64_C(32));
      RECREATE(lbits)(&zghz3702);
      UNDEFINED(lbits)(&zghz3702, zghz3701);
      zPMOVSCLR_EL0 = CONVERT_OF(fbits, lbits)(zghz3702, true);
    }
    unit zgsz3272;
    zgsz3272 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3699);
      CONVERT_OF(sail_int, mach_int)(&zghz3699, INT64_C(32));
      RECREATE(lbits)(&zghz3700);
      UNDEFINED(lbits)(&zghz3700, zghz3699);
      zPMOVSSET_EL0 = CONVERT_OF(fbits, lbits)(zghz3700, true);
    }
    unit zgsz3271;
    zgsz3271 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3697);
      CONVERT_OF(sail_int, mach_int)(&zghz3697, INT64_C(32));
      RECREATE(lbits)(&zghz3698);
      UNDEFINED(lbits)(&zghz3698, zghz3697);
      zPMSCR_EL1 = CONVERT_OF(fbits, lbits)(zghz3698, true);
    }
    unit zgsz3270;
    zgsz3270 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3695);
      CONVERT_OF(sail_int, mach_int)(&zghz3695, INT64_C(32));
      RECREATE(lbits)(&zghz3696);
      UNDEFINED(lbits)(&zghz3696, zghz3695);
      zPMSCR_EL2 = CONVERT_OF(fbits, lbits)(zghz3696, true);
    }
    unit zgsz3269;
    zgsz3269 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3693);
      CONVERT_OF(sail_int, mach_int)(&zghz3693, INT64_C(32));
      RECREATE(lbits)(&zghz3694);
      UNDEFINED(lbits)(&zghz3694, zghz3693);
      zPMSELR_EL0 = CONVERT_OF(fbits, lbits)(zghz3694, true);
    }
    unit zgsz3268;
    zgsz3268 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3691);
      CONVERT_OF(sail_int, mach_int)(&zghz3691, INT64_C(64));
      RECREATE(lbits)(&zghz3692);
      UNDEFINED(lbits)(&zghz3692, zghz3691);
      zPMSEVFR_EL1 = CONVERT_OF(fbits, lbits)(zghz3692, true);
    }
    unit zgsz3267;
    zgsz3267 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3689);
      CONVERT_OF(sail_int, mach_int)(&zghz3689, INT64_C(32));
      RECREATE(lbits)(&zghz3690);
      UNDEFINED(lbits)(&zghz3690, zghz3689);
      zPMSFCR_EL1 = CONVERT_OF(fbits, lbits)(zghz3690, true);
    }
    unit zgsz3266;
    zgsz3266 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3687);
      CONVERT_OF(sail_int, mach_int)(&zghz3687, INT64_C(64));
      RECREATE(lbits)(&zghz3688);
      UNDEFINED(lbits)(&zghz3688, zghz3687);
      zPMSICR_EL1 = CONVERT_OF(fbits, lbits)(zghz3688, true);
    }
    unit zgsz3265;
    zgsz3265 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3685);
      CONVERT_OF(sail_int, mach_int)(&zghz3685, INT64_C(32));
      RECREATE(lbits)(&zghz3686);
      UNDEFINED(lbits)(&zghz3686, zghz3685);
      zPMSIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3686, true);
    }
    unit zgsz3264;
    zgsz3264 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3683);
      CONVERT_OF(sail_int, mach_int)(&zghz3683, INT64_C(32));
      RECREATE(lbits)(&zghz3684);
      UNDEFINED(lbits)(&zghz3684, zghz3683);
      zPMSIRR_EL1 = CONVERT_OF(fbits, lbits)(zghz3684, true);
    }
    unit zgsz3263;
    zgsz3263 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3681);
      CONVERT_OF(sail_int, mach_int)(&zghz3681, INT64_C(32));
      RECREATE(lbits)(&zghz3682);
      UNDEFINED(lbits)(&zghz3682, zghz3681);
      zPMSLATFR_EL1 = CONVERT_OF(fbits, lbits)(zghz3682, true);
    }
    unit zgsz3262;
    zgsz3262 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3679);
      CONVERT_OF(sail_int, mach_int)(&zghz3679, INT64_C(32));
      RECREATE(lbits)(&zghz3680);
      UNDEFINED(lbits)(&zghz3680, zghz3679);
      zPMXEVCNTR_EL0 = CONVERT_OF(fbits, lbits)(zghz3680, true);
    }
    unit zgsz3261;
    zgsz3261 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3677);
      CONVERT_OF(sail_int, mach_int)(&zghz3677, INT64_C(32));
      RECREATE(lbits)(&zghz3678);
      UNDEFINED(lbits)(&zghz3678, zghz3677);
      zPMXEVTYPER_EL0 = CONVERT_OF(fbits, lbits)(zghz3678, true);
    }
    unit zgsz3260;
    zgsz3260 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3675);
      CONVERT_OF(sail_int, mach_int)(&zghz3675, INT64_C(64));
      RECREATE(lbits)(&zghz3676);
      UNDEFINED(lbits)(&zghz3676, zghz3675);
      zREVIDR_EL1 = CONVERT_OF(fbits, lbits)(zghz3676, true);
    }
    unit zgsz3259;
    zgsz3259 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3673);
      CONVERT_OF(sail_int, mach_int)(&zghz3673, INT64_C(32));
      RECREATE(lbits)(&zghz3674);
      UNDEFINED(lbits)(&zghz3674, zghz3673);
      zRMR_EL1 = CONVERT_OF(fbits, lbits)(zghz3674, true);
    }
    unit zgsz3258;
    zgsz3258 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3671);
      CONVERT_OF(sail_int, mach_int)(&zghz3671, INT64_C(32));
      RECREATE(lbits)(&zghz3672);
      UNDEFINED(lbits)(&zghz3672, zghz3671);
      zRMR_EL2 = CONVERT_OF(fbits, lbits)(zghz3672, true);
    }
    unit zgsz3257;
    zgsz3257 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3669);
      CONVERT_OF(sail_int, mach_int)(&zghz3669, INT64_C(32));
      RECREATE(lbits)(&zghz3670);
      UNDEFINED(lbits)(&zghz3670, zghz3669);
      zRMR_EL3 = CONVERT_OF(fbits, lbits)(zghz3670, true);
    }
    unit zgsz3256;
    zgsz3256 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3668);
      CONVERT_OF(sail_int, mach_int)(&zghz3668, INT64_C(129));
      UNDEFINED(lbits)(&zRSP_EL0, zghz3668);
    }
    unit zgsz3255;
    zgsz3255 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3667);
      CONVERT_OF(sail_int, mach_int)(&zghz3667, INT64_C(129));
      UNDEFINED(lbits)(&zRTPIDR_EL0, zghz3667);
    }
    unit zgsz3254;
    zgsz3254 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3665);
      CONVERT_OF(sail_int, mach_int)(&zghz3665, INT64_C(64));
      RECREATE(lbits)(&zghz3666);
      UNDEFINED(lbits)(&zghz3666, zghz3665);
      zRVBAR_EL1 = CONVERT_OF(fbits, lbits)(zghz3666, true);
    }
    unit zgsz3253;
    zgsz3253 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3663);
      CONVERT_OF(sail_int, mach_int)(&zghz3663, INT64_C(64));
      RECREATE(lbits)(&zghz3664);
      UNDEFINED(lbits)(&zghz3664, zghz3663);
      zRVBAR_EL2 = CONVERT_OF(fbits, lbits)(zghz3664, true);
    }
    unit zgsz3252;
    zgsz3252 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3661);
      CONVERT_OF(sail_int, mach_int)(&zghz3661, INT64_C(64));
      RECREATE(lbits)(&zghz3662);
      UNDEFINED(lbits)(&zghz3662, zghz3661);
      zRVBAR_EL3 = CONVERT_OF(fbits, lbits)(zghz3662, true);
    }
    unit zgsz3251;
    zgsz3251 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3659);
      CONVERT_OF(sail_int, mach_int)(&zghz3659, INT64_C(64));
      RECREATE(lbits)(&zghz3660);
      UNDEFINED(lbits)(&zghz3660, zghz3659);
      zS3_op1_Cn_Cm_op2 = CONVERT_OF(fbits, lbits)(zghz3660, true);
    }
    unit zgsz3250;
    zgsz3250 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3658);
      CONVERT_OF(sail_int, mach_int)(&zghz3658, INT64_C(129));
      UNDEFINED(lbits)(&zCID_EL0, zghz3658);
    }
    unit zgsz3249;
    zgsz3249 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3656);
      CONVERT_OF(sail_int, mach_int)(&zghz3656, INT64_C(64));
      RECREATE(lbits)(&zghz3657);
      UNDEFINED(lbits)(&zghz3657, zghz3656);
      zSCXTNUM_EL1 = CONVERT_OF(fbits, lbits)(zghz3657, true);
    }
    unit zgsz3248;
    zgsz3248 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3654);
      CONVERT_OF(sail_int, mach_int)(&zghz3654, INT64_C(64));
      RECREATE(lbits)(&zghz3655);
      UNDEFINED(lbits)(&zghz3655, zghz3654);
      zSCXTNUM_EL2 = CONVERT_OF(fbits, lbits)(zghz3655, true);
    }
    unit zgsz3247;
    zgsz3247 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3652);
      CONVERT_OF(sail_int, mach_int)(&zghz3652, INT64_C(64));
      RECREATE(lbits)(&zghz3653);
      UNDEFINED(lbits)(&zghz3653, zghz3652);
      zSCXTNUM_EL3 = CONVERT_OF(fbits, lbits)(zghz3653, true);
    }
    unit zgsz3246;
    zgsz3246 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3650);
      CONVERT_OF(sail_int, mach_int)(&zghz3650, INT64_C(32));
      RECREATE(lbits)(&zghz3651);
      UNDEFINED(lbits)(&zghz3651, zghz3650);
      zSDER32_EL3 = CONVERT_OF(fbits, lbits)(zghz3651, true);
    }
    unit zgsz3245;
    zgsz3245 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3648);
      CONVERT_OF(sail_int, mach_int)(&zghz3648, INT64_C(32));
      RECREATE(lbits)(&zghz3649);
      UNDEFINED(lbits)(&zghz3649, zghz3648);
      zSPSR_abt = CONVERT_OF(fbits, lbits)(zghz3649, true);
    }
    unit zgsz3244;
    zgsz3244 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3646);
      CONVERT_OF(sail_int, mach_int)(&zghz3646, INT64_C(32));
      RECREATE(lbits)(&zghz3647);
      UNDEFINED(lbits)(&zghz3647, zghz3646);
      zSPSR_fiq = CONVERT_OF(fbits, lbits)(zghz3647, true);
    }
    unit zgsz3243;
    zgsz3243 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3644);
      CONVERT_OF(sail_int, mach_int)(&zghz3644, INT64_C(32));
      RECREATE(lbits)(&zghz3645);
      UNDEFINED(lbits)(&zghz3645, zghz3644);
      zSPSR_irq = CONVERT_OF(fbits, lbits)(zghz3645, true);
    }
    unit zgsz3242;
    zgsz3242 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3642);
      CONVERT_OF(sail_int, mach_int)(&zghz3642, INT64_C(32));
      RECREATE(lbits)(&zghz3643);
      UNDEFINED(lbits)(&zghz3643, zghz3642);
      zSPSR_und = CONVERT_OF(fbits, lbits)(zghz3643, true);
    }
    unit zgsz3241;
    zgsz3241 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3641);
      CONVERT_OF(sail_int, mach_int)(&zghz3641, INT64_C(129));
      UNDEFINED(lbits)(&zSP_EL0, zghz3641);
    }
    unit zgsz3240;
    zgsz3240 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3640);
      CONVERT_OF(sail_int, mach_int)(&zghz3640, INT64_C(129));
      UNDEFINED(lbits)(&zSP_EL1, zghz3640);
    }
    unit zgsz3239;
    zgsz3239 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3639);
      CONVERT_OF(sail_int, mach_int)(&zghz3639, INT64_C(129));
      UNDEFINED(lbits)(&zSP_EL2, zghz3639);
    }
    unit zgsz3238;
    zgsz3238 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3638);
      CONVERT_OF(sail_int, mach_int)(&zghz3638, INT64_C(129));
      UNDEFINED(lbits)(&zTPIDRRO_EL0, zghz3638);
    }
    unit zgsz3237;
    zgsz3237 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3637);
      CONVERT_OF(sail_int, mach_int)(&zghz3637, INT64_C(129));
      UNDEFINED(lbits)(&zTPIDR_EL0, zghz3637);
    }
    unit zgsz3236;
    zgsz3236 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3636);
      CONVERT_OF(sail_int, mach_int)(&zghz3636, INT64_C(129));
      UNDEFINED(lbits)(&zTPIDR_EL1, zghz3636);
    }
    unit zgsz3235;
    zgsz3235 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3635);
      CONVERT_OF(sail_int, mach_int)(&zghz3635, INT64_C(129));
      UNDEFINED(lbits)(&zTPIDR_EL2, zghz3635);
    }
    unit zgsz3234;
    zgsz3234 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3634);
      CONVERT_OF(sail_int, mach_int)(&zghz3634, INT64_C(129));
      UNDEFINED(lbits)(&zTPIDR_EL3, zghz3634);
    }
    unit zgsz3233;
    zgsz3233 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3632);
      CONVERT_OF(sail_int, mach_int)(&zghz3632, INT64_C(64));
      RECREATE(lbits)(&zghz3633);
      UNDEFINED(lbits)(&zghz3633, zghz3632);
      zTTBR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3633, true);
    }
    unit zgsz3232;
    zgsz3232 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3630);
      CONVERT_OF(sail_int, mach_int)(&zghz3630, INT64_C(64));
      RECREATE(lbits)(&zghz3631);
      UNDEFINED(lbits)(&zghz3631, zghz3630);
      zTTBR0_EL2 = CONVERT_OF(fbits, lbits)(zghz3631, true);
    }
    unit zgsz3231;
    zgsz3231 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3628);
      CONVERT_OF(sail_int, mach_int)(&zghz3628, INT64_C(64));
      RECREATE(lbits)(&zghz3629);
      UNDEFINED(lbits)(&zghz3629, zghz3628);
      zTTBR0_EL3 = CONVERT_OF(fbits, lbits)(zghz3629, true);
    }
    unit zgsz3230;
    zgsz3230 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3626);
      CONVERT_OF(sail_int, mach_int)(&zghz3626, INT64_C(64));
      RECREATE(lbits)(&zghz3627);
      UNDEFINED(lbits)(&zghz3627, zghz3626);
      zTTBR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3627, true);
    }
    unit zgsz3229;
    zgsz3229 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3624);
      CONVERT_OF(sail_int, mach_int)(&zghz3624, INT64_C(64));
      RECREATE(lbits)(&zghz3625);
      UNDEFINED(lbits)(&zghz3625, zghz3624);
      zTTBR1_EL2 = CONVERT_OF(fbits, lbits)(zghz3625, true);
    }
    unit zgsz3228;
    zgsz3228 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3622);
      CONVERT_OF(sail_int, mach_int)(&zghz3622, INT64_C(32));
      RECREATE(lbits)(&zghz3623);
      UNDEFINED(lbits)(&zghz3623, zghz3622);
      zVSESR_EL2 = CONVERT_OF(fbits, lbits)(zghz3623, true);
    }
    unit zgsz3227;
    zgsz3227 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3620);
      CONVERT_OF(sail_int, mach_int)(&zghz3620, INT64_C(32));
      RECREATE(lbits)(&zghz3621);
      UNDEFINED(lbits)(&zghz3621, zghz3620);
      zVTCR_EL2 = CONVERT_OF(fbits, lbits)(zghz3621, true);
    }
    unit zgsz3226;
    zgsz3226 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3618);
      CONVERT_OF(sail_int, mach_int)(&zghz3618, INT64_C(64));
      RECREATE(lbits)(&zghz3619);
      UNDEFINED(lbits)(&zghz3619, zghz3618);
      zVTTBR_EL2 = CONVERT_OF(fbits, lbits)(zghz3619, true);
    }
    unit zgsz3225;
    zgsz3225 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3617);
      CONVERT_OF(sail_int, mach_int)(&zghz3617, INT64_C(129));
      UNDEFINED(lbits)(&zDDC_EL0, zghz3617);
    }
    unit zgsz3224;
    zgsz3224 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3616);
      CONVERT_OF(sail_int, mach_int)(&zghz3616, INT64_C(129));
      UNDEFINED(lbits)(&zDDC_EL1, zghz3616);
    }
    unit zgsz3223;
    zgsz3223 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3615);
      CONVERT_OF(sail_int, mach_int)(&zghz3615, INT64_C(129));
      UNDEFINED(lbits)(&zDDC_EL2, zghz3615);
    }
    unit zgsz3222;
    zgsz3222 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3614);
      CONVERT_OF(sail_int, mach_int)(&zghz3614, INT64_C(129));
      UNDEFINED(lbits)(&zDDC_EL3, zghz3614);
    }
    unit zgsz3221;
    zgsz3221 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3613);
      CONVERT_OF(sail_int, mach_int)(&zghz3613, INT64_C(129));
      UNDEFINED(lbits)(&zRDDC_EL0, zghz3613);
    }
    unit zgsz3220;
    zgsz3220 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3611);
      CONVERT_OF(sail_int, mach_int)(&zghz3611, INT64_C(32));
      RECREATE(lbits)(&zghz3612);
      UNDEFINED(lbits)(&zghz3612, zghz3611);
      zDBGDTRTX_EL0 = CONVERT_OF(fbits, lbits)(zghz3612, true);
    }
    unit zgsz3219;
    zgsz3219 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3609);
      CONVERT_OF(sail_int, mach_int)(&zghz3609, INT64_C(64));
      RECREATE(lbits)(&zghz3610);
      UNDEFINED(lbits)(&zghz3610, zghz3609);
      zICC_ASGI1R_EL1 = CONVERT_OF(fbits, lbits)(zghz3610, true);
    }
    unit zgsz3218;
    zgsz3218 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3607);
      CONVERT_OF(sail_int, mach_int)(&zghz3607, INT64_C(32));
      RECREATE(lbits)(&zghz3608);
      UNDEFINED(lbits)(&zghz3608, zghz3607);
      zICC_DIR_EL1 = CONVERT_OF(fbits, lbits)(zghz3608, true);
    }
    unit zgsz3217;
    zgsz3217 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3605);
      CONVERT_OF(sail_int, mach_int)(&zghz3605, INT64_C(32));
      RECREATE(lbits)(&zghz3606);
      UNDEFINED(lbits)(&zghz3606, zghz3605);
      zICV_DIR_EL1 = CONVERT_OF(fbits, lbits)(zghz3606, true);
    }
    unit zgsz3216;
    zgsz3216 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3603);
      CONVERT_OF(sail_int, mach_int)(&zghz3603, INT64_C(32));
      RECREATE(lbits)(&zghz3604);
      UNDEFINED(lbits)(&zghz3604, zghz3603);
      zICC_EOIR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3604, true);
    }
    unit zgsz3215;
    zgsz3215 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3601);
      CONVERT_OF(sail_int, mach_int)(&zghz3601, INT64_C(32));
      RECREATE(lbits)(&zghz3602);
      UNDEFINED(lbits)(&zghz3602, zghz3601);
      zICV_EOIR0_EL1 = CONVERT_OF(fbits, lbits)(zghz3602, true);
    }
    unit zgsz3214;
    zgsz3214 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3599);
      CONVERT_OF(sail_int, mach_int)(&zghz3599, INT64_C(32));
      RECREATE(lbits)(&zghz3600);
      UNDEFINED(lbits)(&zghz3600, zghz3599);
      zICC_EOIR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3600, true);
    }
    unit zgsz3213;
    zgsz3213 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3597);
      CONVERT_OF(sail_int, mach_int)(&zghz3597, INT64_C(32));
      RECREATE(lbits)(&zghz3598);
      UNDEFINED(lbits)(&zghz3598, zghz3597);
      zICV_EOIR1_EL1 = CONVERT_OF(fbits, lbits)(zghz3598, true);
    }
    unit zgsz3212;
    zgsz3212 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3595);
      CONVERT_OF(sail_int, mach_int)(&zghz3595, INT64_C(64));
      RECREATE(lbits)(&zghz3596);
      UNDEFINED(lbits)(&zghz3596, zghz3595);
      zICC_SGI0R_EL1 = CONVERT_OF(fbits, lbits)(zghz3596, true);
    }
    unit zgsz3211;
    zgsz3211 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3593);
      CONVERT_OF(sail_int, mach_int)(&zghz3593, INT64_C(64));
      RECREATE(lbits)(&zghz3594);
      UNDEFINED(lbits)(&zghz3594, zghz3593);
      zICC_SGI1R_EL1 = CONVERT_OF(fbits, lbits)(zghz3594, true);
    }
    unit zgsz3210;
    zgsz3210 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3591);
      CONVERT_OF(sail_int, mach_int)(&zghz3591, INT64_C(32));
      RECREATE(lbits)(&zghz3592);
      UNDEFINED(lbits)(&zghz3592, zghz3591);
      zOSLAR_EL1 = CONVERT_OF(fbits, lbits)(zghz3592, true);
    }
    unit zgsz3209;
    zgsz3209 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3589);
      CONVERT_OF(sail_int, mach_int)(&zghz3589, INT64_C(32));
      RECREATE(lbits)(&zghz3590);
      UNDEFINED(lbits)(&zghz3590, zghz3589);
      zPMSWINC_EL0 = CONVERT_OF(fbits, lbits)(zghz3590, true);
    }
    unit zgsz3208;
    zgsz3208 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3587);
      CONVERT_OF(sail_int, mach_int)(&zghz3587, INT64_C(1));
      RECREATE(lbits)(&zghz3588);
      UNDEFINED(lbits)(&zghz3588, zghz3587);
      zEventRegister = CONVERT_OF(fbits, lbits)(zghz3588, true);
    }
    unit zgsz3207;
    zgsz3207 = UNIT;
  }
  {
    RECREATE(lbits)(&zghz3584);
    {
      RECREATE(sail_int)(&zghz3586);
      CONVERT_OF(sail_int, mach_int)(&zghz3586, INT64_C(128));
      UNDEFINED(lbits)(&zghz3584, zghz3586);
    }
    {
      RECREATE(sail_int)(&zghz3585);
      CONVERT_OF(sail_int, mach_int)(&zghz3585, INT64_C(32));
      UNDEFINED(vector_zz5vecz8z5bvz9)(&z_V, zghz3585, zghz3584);
    }
    unit zgsz3206;
    zgsz3206 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3582);
      CONVERT_OF(sail_int, mach_int)(&zghz3582, INT64_C(2));
      RECREATE(lbits)(&zghz3583);
      UNDEFINED(lbits)(&zghz3583, zghz3582);
      z__saved_exception_level = CONVERT_OF(fbits, lbits)(zghz3583, true);
    }
    unit zgsz3205;
    zgsz3205 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3580);
      CONVERT_OF(sail_int, mach_int)(&zghz3580, INT64_C(48));
      RECREATE(lbits)(&zghz3581);
      UNDEFINED(lbits)(&zghz3581, zghz3580);
      z__CNTControlBase = CONVERT_OF(fbits, lbits)(zghz3581, true);
    }
    unit zgsz3204;
    zgsz3204 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3578);
      CONVERT_OF(sail_int, mach_int)(&zghz3578, INT64_C(32));
      RECREATE(lbits)(&zghz3579);
      UNDEFINED(lbits)(&zghz3579, zghz3578);
      z__ThisInstr = CONVERT_OF(fbits, lbits)(zghz3579, true);
    }
    unit zgsz3203;
    zgsz3203 = UNIT;
  }
  {
    z__ThisInstrEnc = sailgen_undefined___InstrEnc(UNIT);
    unit zgsz3202;
    zgsz3202 = UNIT;
  }
  {
    sailgen_undefined_instr_ast(&z__ThisInstrAbstract, UNIT);
    unit zgsz3201;
    zgsz3201 = UNIT;
  }
  {
    z__highest_el_aarch32 = false;
    unit zgsz3200;
    zgsz3200 = UNIT;
  }
  {
    {
      RECREATE(sail_int)(&zghz3577);
      CONVERT_OF(sail_int, mach_int)(&zghz3577, INT64_C(129));
      UNDEFINED(lbits)(&zSP_EL3, zghz3577);
    }
    unit zgsz3199;
    zgsz3199 = UNIT;
  }
  {
    RECREATE(sail_int)(&zghz3576);
    CONVERT_OF(sail_int, mach_int)(&zghz3576, INT64_C(129));
    UNDEFINED(lbits)(&zTestCap, zghz3576);
  }
  zcbz339 = UNIT;
end_function_281: ;
  return zcbz339;
end_block_exception_282: ;

  return UNIT;
}



static void finish_sailgen_initializze_registers(void)
{
  KILL(sail_int)(&zghz31235);
  KILL(lbits)(&zghz31234);
  KILL(sail_int)(&zghz31233);
  KILL(lbits)(&zghz31232);
  KILL(sail_int)(&zghz31231);
  KILL(lbits)(&zghz31230);
  KILL(sail_int)(&zghz31229);
  KILL(lbits)(&zghz31228);
  KILL(sail_int)(&zghz31227);
  KILL(lbits)(&zghz31226);
  KILL(sail_int)(&zghz31225);
  KILL(lbits)(&zghz31224);
  KILL(sail_int)(&zghz31223);
  KILL(lbits)(&zghz31222);
  KILL(sail_int)(&zghz31221);
  KILL(lbits)(&zghz31220);
  KILL(sail_int)(&zghz31219);
  KILL(sail_int)(&zghz31218);
  KILL(sail_int)(&zghz31217);
  KILL(sail_int)(&zghz31216);
  KILL(sail_int)(&zghz31215);
  KILL(sail_int)(&zghz31214);
  KILL(sail_int)(&zghz31213);
  KILL(sail_int)(&zghz31212);
  KILL(sail_int)(&zghz31211);
  KILL(sail_int)(&zghz31210);
  KILL(sail_int)(&zghz31209);
  KILL(sail_int)(&zghz31208);
  KILL(sail_int)(&zghz31207);
  KILL(sail_int)(&zghz31206);
  KILL(sail_int)(&zghz31205);
  KILL(sail_int)(&zghz31204);
  KILL(sail_int)(&zghz31203);
  KILL(sail_int)(&zghz31202);
  KILL(sail_int)(&zghz31201);
  KILL(sail_int)(&zghz31200);
  KILL(sail_int)(&zghz31199);
  KILL(sail_int)(&zghz31198);
  KILL(sail_int)(&zghz31197);
  KILL(sail_int)(&zghz31196);
  KILL(sail_int)(&zghz31195);
  KILL(sail_int)(&zghz31194);
  KILL(sail_int)(&zghz31193);
  KILL(sail_int)(&zghz31192);
  KILL(sail_int)(&zghz31191);
  KILL(sail_int)(&zghz31190);
  KILL(sail_int)(&zghz31189);
  KILL(sail_int)(&zghz31188);
  KILL(lbits)(&zghz31187);
  KILL(sail_int)(&zghz31186);
  KILL(lbits)(&zghz31185);
  KILL(sail_int)(&zghz31184);
  KILL(lbits)(&zghz31183);
  KILL(sail_int)(&zghz31182);
  KILL(lbits)(&zghz31181);
  KILL(sail_int)(&zghz31180);
  KILL(lbits)(&zghz31179);
  KILL(sail_int)(&zghz31178);
  KILL(lbits)(&zghz31177);
  KILL(sail_int)(&zghz31176);
  KILL(lbits)(&zghz31175);
  KILL(sail_int)(&zghz31174);
  KILL(lbits)(&zghz31173);
  KILL(sail_int)(&zghz31172);
  KILL(lbits)(&zghz31171);
  KILL(sail_int)(&zghz31170);
  KILL(lbits)(&zghz31169);
  KILL(sail_int)(&zghz31168);
  KILL(lbits)(&zghz31167);
  KILL(sail_int)(&zghz31166);
  KILL(lbits)(&zghz31165);
  KILL(sail_int)(&zghz31164);
  KILL(lbits)(&zghz31163);
  KILL(sail_int)(&zghz31162);
  KILL(lbits)(&zghz31161);
  KILL(sail_int)(&zghz31160);
  KILL(lbits)(&zghz31159);
  KILL(sail_int)(&zghz31158);
  KILL(sail_int)(&zghz31157);
  KILL(sail_int)(&zghz31156);
  KILL(sail_int)(&zghz31155);
  KILL(sail_int)(&zghz31154);
  KILL(sail_int)(&zghz31153);
  KILL(sail_int)(&zghz31152);
  KILL(lbits)(&zghz31151);
  KILL(sail_int)(&zghz31150);
  KILL(lbits)(&zghz31149);
  KILL(sail_int)(&zghz31148);
  KILL(lbits)(&zghz31147);
  KILL(sail_int)(&zghz31146);
  KILL(lbits)(&zghz31145);
  KILL(sail_int)(&zghz31144);
  KILL(lbits)(&zghz31143);
  KILL(sail_int)(&zghz31142);
  KILL(lbits)(&zghz31141);
  KILL(sail_int)(&zghz31140);
  KILL(lbits)(&zghz31139);
  KILL(sail_int)(&zghz31138);
  KILL(lbits)(&zghz31137);
  KILL(sail_int)(&zghz31136);
  KILL(lbits)(&zghz31135);
  KILL(sail_int)(&zghz31134);
  KILL(lbits)(&zghz31133);
  KILL(sail_int)(&zghz31132);
  KILL(lbits)(&zghz31131);
  KILL(sail_int)(&zghz31130);
  KILL(lbits)(&zghz31129);
  KILL(sail_int)(&zghz31128);
  KILL(lbits)(&zghz31127);
  KILL(sail_int)(&zghz31126);
  KILL(lbits)(&zghz31125);
  KILL(sail_int)(&zghz31124);
  KILL(lbits)(&zghz31123);
  KILL(sail_int)(&zghz31122);
  KILL(lbits)(&zghz31121);
  KILL(sail_int)(&zghz31120);
  KILL(lbits)(&zghz31119);
  KILL(sail_int)(&zghz31118);
  KILL(lbits)(&zghz31117);
  KILL(sail_int)(&zghz31116);
  KILL(lbits)(&zghz31115);
  KILL(sail_int)(&zghz31114);
  KILL(lbits)(&zghz31113);
  KILL(sail_int)(&zghz31112);
  KILL(lbits)(&zghz31111);
  KILL(sail_int)(&zghz31110);
  KILL(lbits)(&zghz31109);
  KILL(sail_int)(&zghz31108);
  KILL(lbits)(&zghz31107);
  KILL(sail_int)(&zghz31106);
  KILL(lbits)(&zghz31105);
  KILL(sail_int)(&zghz31104);
  KILL(lbits)(&zghz31103);
  KILL(sail_int)(&zghz31102);
  KILL(lbits)(&zghz31101);
  KILL(sail_int)(&zghz31100);
  KILL(lbits)(&zghz31099);
  KILL(sail_int)(&zghz31098);
  KILL(lbits)(&zghz31097);
  KILL(sail_int)(&zghz31096);
  KILL(lbits)(&zghz31095);
  KILL(sail_int)(&zghz31094);
  KILL(lbits)(&zghz31093);
  KILL(sail_int)(&zghz31092);
  KILL(lbits)(&zghz31091);
  KILL(sail_int)(&zghz31090);
  KILL(lbits)(&zghz31089);
  KILL(sail_int)(&zghz31088);
  KILL(lbits)(&zghz31087);
  KILL(sail_int)(&zghz31086);
  KILL(lbits)(&zghz31085);
  KILL(sail_int)(&zghz31084);
  KILL(lbits)(&zghz31083);
  KILL(sail_int)(&zghz31082);
  KILL(lbits)(&zghz31081);
  KILL(sail_int)(&zghz31080);
  KILL(lbits)(&zghz31079);
  KILL(sail_int)(&zghz31078);
  KILL(lbits)(&zghz31077);
  KILL(sail_int)(&zghz31076);
  KILL(lbits)(&zghz31075);
  KILL(sail_int)(&zghz31074);
  KILL(lbits)(&zghz31073);
  KILL(sail_int)(&zghz31072);
  KILL(lbits)(&zghz31071);
  KILL(sail_int)(&zghz31070);
  KILL(lbits)(&zghz31069);
  KILL(sail_int)(&zghz31068);
  KILL(lbits)(&zghz31067);
  KILL(sail_int)(&zghz31066);
  KILL(lbits)(&zghz31065);
  KILL(sail_int)(&zghz31064);
  KILL(lbits)(&zghz31063);
  KILL(sail_int)(&zghz31062);
  KILL(lbits)(&zghz31061);
  KILL(sail_int)(&zghz31060);
  KILL(lbits)(&zghz31059);
  KILL(sail_int)(&zghz31058);
  KILL(lbits)(&zghz31057);
  KILL(sail_int)(&zghz31056);
  KILL(lbits)(&zghz31055);
  KILL(sail_int)(&zghz31054);
  KILL(lbits)(&zghz31053);
  KILL(sail_int)(&zghz31052);
  KILL(lbits)(&zghz31051);
  KILL(sail_int)(&zghz31050);
  KILL(lbits)(&zghz31049);
  KILL(sail_int)(&zghz31048);
  KILL(lbits)(&zghz31047);
  KILL(sail_int)(&zghz31046);
  KILL(lbits)(&zghz31045);
  KILL(sail_int)(&zghz31044);
  KILL(lbits)(&zghz31043);
  KILL(sail_int)(&zghz31042);
  KILL(lbits)(&zghz31041);
  KILL(sail_int)(&zghz31040);
  KILL(lbits)(&zghz31039);
  KILL(sail_int)(&zghz31038);
  KILL(sail_int)(&zghz31037);
  KILL(sail_int)(&zghz31036);
  KILL(lbits)(&zghz31035);
  KILL(sail_int)(&zghz31034);
  KILL(lbits)(&zghz31033);
  KILL(sail_int)(&zghz31032);
  KILL(lbits)(&zghz31031);
  KILL(sail_int)(&zghz31030);
  KILL(lbits)(&zghz31029);
  KILL(sail_int)(&zghz31028);
  KILL(lbits)(&zghz31027);
  KILL(sail_int)(&zghz31026);
  KILL(sail_int)(&zghz31025);
  KILL(lbits)(&zghz31024);
  KILL(sail_int)(&zghz31023);
  KILL(lbits)(&zghz31022);
  KILL(sail_int)(&zghz31021);
  KILL(lbits)(&zghz31020);
  KILL(sail_int)(&zghz31019);
  KILL(lbits)(&zghz31018);
  KILL(sail_int)(&zghz31017);
  KILL(sail_int)(&zghz31016);
  KILL(lbits)(&zghz31015);
  KILL(sail_int)(&zghz31014);
  KILL(lbits)(&zghz31013);
  KILL(sail_int)(&zghz31012);
  KILL(sail_int)(&zghz31011);
  KILL(lbits)(&zghz31010);
  KILL(sail_int)(&zghz31009);
  KILL(sail_int)(&zghz31008);
  KILL(lbits)(&zghz31007);
  KILL(sail_int)(&zghz31006);
  KILL(lbits)(&zghz31005);
  KILL(sail_int)(&zghz31004);
  KILL(lbits)(&zghz31003);
  KILL(sail_int)(&zghz31002);
  KILL(lbits)(&zghz31001);
  KILL(sail_int)(&zghz31000);
  KILL(lbits)(&zghz3999);
  KILL(sail_int)(&zghz3998);
  KILL(lbits)(&zghz3997);
  KILL(sail_int)(&zghz3996);
  KILL(lbits)(&zghz3995);
  KILL(sail_int)(&zghz3994);
  KILL(lbits)(&zghz3993);
  KILL(sail_int)(&zghz3992);
  KILL(lbits)(&zghz3991);
  KILL(sail_int)(&zghz3990);
  KILL(lbits)(&zghz3989);
  KILL(sail_int)(&zghz3988);
  KILL(lbits)(&zghz3987);
  KILL(sail_int)(&zghz3986);
  KILL(lbits)(&zghz3985);
  KILL(sail_int)(&zghz3984);
  KILL(lbits)(&zghz3983);
  KILL(sail_int)(&zghz3982);
  KILL(lbits)(&zghz3981);
  KILL(sail_int)(&zghz3980);
  KILL(lbits)(&zghz3979);
  KILL(sail_int)(&zghz3978);
  KILL(lbits)(&zghz3977);
  KILL(sail_int)(&zghz3976);
  KILL(lbits)(&zghz3975);
  KILL(sail_int)(&zghz3974);
  KILL(sail_int)(&zghz3973);
  KILL(lbits)(&zghz3972);
  KILL(sail_int)(&zghz3971);
  KILL(lbits)(&zghz3970);
  KILL(sail_int)(&zghz3969);
  KILL(lbits)(&zghz3968);
  KILL(sail_int)(&zghz3967);
  KILL(lbits)(&zghz3966);
  KILL(sail_int)(&zghz3965);
  KILL(lbits)(&zghz3964);
  KILL(sail_int)(&zghz3963);
  KILL(lbits)(&zghz3962);
  KILL(sail_int)(&zghz3961);
  KILL(sail_int)(&zghz3960);
  KILL(lbits)(&zghz3959);
  KILL(sail_int)(&zghz3958);
  KILL(sail_int)(&zghz3957);
  KILL(lbits)(&zghz3956);
  KILL(sail_int)(&zghz3955);
  KILL(sail_int)(&zghz3954);
  KILL(lbits)(&zghz3953);
  KILL(sail_int)(&zghz3952);
  KILL(sail_int)(&zghz3951);
  KILL(lbits)(&zghz3950);
  KILL(sail_int)(&zghz3949);
  KILL(sail_int)(&zghz3948);
  KILL(lbits)(&zghz3947);
  KILL(sail_int)(&zghz3946);
  KILL(lbits)(&zghz3945);
  KILL(sail_int)(&zghz3944);
  KILL(lbits)(&zghz3943);
  KILL(sail_int)(&zghz3942);
  KILL(lbits)(&zghz3941);
  KILL(sail_int)(&zghz3940);
  KILL(lbits)(&zghz3939);
  KILL(sail_int)(&zghz3938);
  KILL(lbits)(&zghz3937);
  KILL(sail_int)(&zghz3936);
  KILL(lbits)(&zghz3935);
  KILL(sail_int)(&zghz3934);
  KILL(lbits)(&zghz3933);
  KILL(sail_int)(&zghz3932);
  KILL(lbits)(&zghz3931);
  KILL(sail_int)(&zghz3930);
  KILL(lbits)(&zghz3929);
  KILL(sail_int)(&zghz3928);
  KILL(lbits)(&zghz3927);
  KILL(sail_int)(&zghz3926);
  KILL(lbits)(&zghz3925);
  KILL(sail_int)(&zghz3924);
  KILL(lbits)(&zghz3923);
  KILL(sail_int)(&zghz3922);
  KILL(lbits)(&zghz3921);
  KILL(sail_int)(&zghz3920);
  KILL(lbits)(&zghz3919);
  KILL(sail_int)(&zghz3918);
  KILL(lbits)(&zghz3917);
  KILL(sail_int)(&zghz3916);
  KILL(lbits)(&zghz3915);
  KILL(sail_int)(&zghz3914);
  KILL(lbits)(&zghz3913);
  KILL(sail_int)(&zghz3912);
  KILL(lbits)(&zghz3911);
  KILL(sail_int)(&zghz3910);
  KILL(lbits)(&zghz3909);
  KILL(sail_int)(&zghz3908);
  KILL(lbits)(&zghz3907);
  KILL(sail_int)(&zghz3906);
  KILL(lbits)(&zghz3905);
  KILL(sail_int)(&zghz3904);
  KILL(lbits)(&zghz3903);
  KILL(sail_int)(&zghz3902);
  KILL(lbits)(&zghz3901);
  KILL(sail_int)(&zghz3900);
  KILL(lbits)(&zghz3899);
  KILL(sail_int)(&zghz3898);
  KILL(lbits)(&zghz3897);
  KILL(sail_int)(&zghz3896);
  KILL(lbits)(&zghz3895);
  KILL(sail_int)(&zghz3894);
  KILL(lbits)(&zghz3893);
  KILL(sail_int)(&zghz3892);
  KILL(sail_int)(&zghz3891);
  KILL(lbits)(&zghz3890);
  KILL(sail_int)(&zghz3889);
  KILL(sail_int)(&zghz3888);
  KILL(lbits)(&zghz3887);
  KILL(sail_int)(&zghz3886);
  KILL(lbits)(&zghz3885);
  KILL(sail_int)(&zghz3884);
  KILL(lbits)(&zghz3883);
  KILL(sail_int)(&zghz3882);
  KILL(sail_int)(&zghz3881);
  KILL(lbits)(&zghz3880);
  KILL(sail_int)(&zghz3879);
  KILL(lbits)(&zghz3878);
  KILL(sail_int)(&zghz3877);
  KILL(lbits)(&zghz3876);
  KILL(sail_int)(&zghz3875);
  KILL(lbits)(&zghz3874);
  KILL(sail_int)(&zghz3873);
  KILL(lbits)(&zghz3872);
  KILL(sail_int)(&zghz3871);
  KILL(lbits)(&zghz3870);
  KILL(sail_int)(&zghz3869);
  KILL(lbits)(&zghz3868);
  KILL(sail_int)(&zghz3867);
  KILL(lbits)(&zghz3866);
  KILL(sail_int)(&zghz3865);
  KILL(lbits)(&zghz3864);
  KILL(sail_int)(&zghz3863);
  KILL(lbits)(&zghz3862);
  KILL(sail_int)(&zghz3861);
  KILL(lbits)(&zghz3860);
  KILL(sail_int)(&zghz3859);
  KILL(lbits)(&zghz3858);
  KILL(sail_int)(&zghz3857);
  KILL(lbits)(&zghz3856);
  KILL(sail_int)(&zghz3855);
  KILL(lbits)(&zghz3854);
  KILL(sail_int)(&zghz3853);
  KILL(lbits)(&zghz3852);
  KILL(sail_int)(&zghz3851);
  KILL(lbits)(&zghz3850);
  KILL(sail_int)(&zghz3849);
  KILL(lbits)(&zghz3848);
  KILL(sail_int)(&zghz3847);
  KILL(lbits)(&zghz3846);
  KILL(sail_int)(&zghz3845);
  KILL(lbits)(&zghz3844);
  KILL(sail_int)(&zghz3843);
  KILL(lbits)(&zghz3842);
  KILL(sail_int)(&zghz3841);
  KILL(lbits)(&zghz3840);
  KILL(sail_int)(&zghz3839);
  KILL(lbits)(&zghz3838);
  KILL(sail_int)(&zghz3837);
  KILL(lbits)(&zghz3836);
  KILL(sail_int)(&zghz3835);
  KILL(lbits)(&zghz3834);
  KILL(sail_int)(&zghz3833);
  KILL(lbits)(&zghz3832);
  KILL(sail_int)(&zghz3831);
  KILL(lbits)(&zghz3830);
  KILL(sail_int)(&zghz3829);
  KILL(lbits)(&zghz3828);
  KILL(sail_int)(&zghz3827);
  KILL(lbits)(&zghz3826);
  KILL(sail_int)(&zghz3825);
  KILL(lbits)(&zghz3824);
  KILL(sail_int)(&zghz3823);
  KILL(lbits)(&zghz3822);
  KILL(sail_int)(&zghz3821);
  KILL(lbits)(&zghz3820);
  KILL(sail_int)(&zghz3819);
  KILL(lbits)(&zghz3818);
  KILL(sail_int)(&zghz3817);
  KILL(lbits)(&zghz3816);
  KILL(sail_int)(&zghz3815);
  KILL(lbits)(&zghz3814);
  KILL(sail_int)(&zghz3813);
  KILL(lbits)(&zghz3812);
  KILL(sail_int)(&zghz3811);
  KILL(lbits)(&zghz3810);
  KILL(sail_int)(&zghz3809);
  KILL(lbits)(&zghz3808);
  KILL(sail_int)(&zghz3807);
  KILL(lbits)(&zghz3806);
  KILL(sail_int)(&zghz3805);
  KILL(lbits)(&zghz3804);
  KILL(sail_int)(&zghz3803);
  KILL(lbits)(&zghz3802);
  KILL(sail_int)(&zghz3801);
  KILL(lbits)(&zghz3800);
  KILL(sail_int)(&zghz3799);
  KILL(lbits)(&zghz3798);
  KILL(sail_int)(&zghz3797);
  KILL(lbits)(&zghz3796);
  KILL(sail_int)(&zghz3795);
  KILL(lbits)(&zghz3794);
  KILL(sail_int)(&zghz3793);
  KILL(lbits)(&zghz3792);
  KILL(sail_int)(&zghz3791);
  KILL(lbits)(&zghz3790);
  KILL(sail_int)(&zghz3789);
  KILL(lbits)(&zghz3788);
  KILL(sail_int)(&zghz3787);
  KILL(lbits)(&zghz3786);
  KILL(sail_int)(&zghz3785);
  KILL(lbits)(&zghz3784);
  KILL(sail_int)(&zghz3783);
  KILL(lbits)(&zghz3782);
  KILL(sail_int)(&zghz3781);
  KILL(lbits)(&zghz3780);
  KILL(sail_int)(&zghz3779);
  KILL(lbits)(&zghz3778);
  KILL(sail_int)(&zghz3777);
  KILL(lbits)(&zghz3776);
  KILL(sail_int)(&zghz3775);
  KILL(lbits)(&zghz3774);
  KILL(sail_int)(&zghz3773);
  KILL(lbits)(&zghz3772);
  KILL(sail_int)(&zghz3771);
  KILL(lbits)(&zghz3770);
  KILL(sail_int)(&zghz3769);
  KILL(lbits)(&zghz3768);
  KILL(sail_int)(&zghz3767);
  KILL(lbits)(&zghz3766);
  KILL(sail_int)(&zghz3765);
  KILL(lbits)(&zghz3764);
  KILL(sail_int)(&zghz3763);
  KILL(lbits)(&zghz3762);
  KILL(sail_int)(&zghz3761);
  KILL(lbits)(&zghz3760);
  KILL(sail_int)(&zghz3759);
  KILL(lbits)(&zghz3758);
  KILL(sail_int)(&zghz3757);
  KILL(lbits)(&zghz3756);
  KILL(sail_int)(&zghz3755);
  KILL(lbits)(&zghz3754);
  KILL(sail_int)(&zghz3753);
  KILL(lbits)(&zghz3752);
  KILL(sail_int)(&zghz3751);
  KILL(lbits)(&zghz3750);
  KILL(sail_int)(&zghz3749);
  KILL(lbits)(&zghz3748);
  KILL(sail_int)(&zghz3747);
  KILL(lbits)(&zghz3746);
  KILL(sail_int)(&zghz3745);
  KILL(lbits)(&zghz3744);
  KILL(sail_int)(&zghz3743);
  KILL(lbits)(&zghz3742);
  KILL(sail_int)(&zghz3741);
  KILL(lbits)(&zghz3740);
  KILL(sail_int)(&zghz3739);
  KILL(lbits)(&zghz3738);
  KILL(sail_int)(&zghz3737);
  KILL(lbits)(&zghz3736);
  KILL(sail_int)(&zghz3735);
  KILL(lbits)(&zghz3734);
  KILL(sail_int)(&zghz3733);
  KILL(lbits)(&zghz3732);
  KILL(sail_int)(&zghz3731);
  KILL(lbits)(&zghz3730);
  KILL(sail_int)(&zghz3729);
  KILL(lbits)(&zghz3728);
  KILL(sail_int)(&zghz3727);
  KILL(lbits)(&zghz3726);
  KILL(sail_int)(&zghz3725);
  KILL(lbits)(&zghz3724);
  KILL(sail_int)(&zghz3723);
  KILL(lbits)(&zghz3722);
  KILL(sail_int)(&zghz3721);
  KILL(lbits)(&zghz3720);
  KILL(sail_int)(&zghz3719);
  KILL(lbits)(&zghz3718);
  KILL(sail_int)(&zghz3717);
  KILL(lbits)(&zghz3716);
  KILL(sail_int)(&zghz3715);
  KILL(lbits)(&zghz3714);
  KILL(sail_int)(&zghz3713);
  KILL(lbits)(&zghz3712);
  KILL(sail_int)(&zghz3711);
  KILL(sail_int)(&zghz3710);
  KILL(lbits)(&zghz3709);
  KILL(sail_int)(&zghz3708);
  KILL(sail_int)(&zghz3707);
  KILL(lbits)(&zghz3706);
  KILL(sail_int)(&zghz3705);
  KILL(lbits)(&zghz3704);
  KILL(sail_int)(&zghz3703);
  KILL(lbits)(&zghz3702);
  KILL(sail_int)(&zghz3701);
  KILL(lbits)(&zghz3700);
  KILL(sail_int)(&zghz3699);
  KILL(lbits)(&zghz3698);
  KILL(sail_int)(&zghz3697);
  KILL(lbits)(&zghz3696);
  KILL(sail_int)(&zghz3695);
  KILL(lbits)(&zghz3694);
  KILL(sail_int)(&zghz3693);
  KILL(lbits)(&zghz3692);
  KILL(sail_int)(&zghz3691);
  KILL(lbits)(&zghz3690);
  KILL(sail_int)(&zghz3689);
  KILL(lbits)(&zghz3688);
  KILL(sail_int)(&zghz3687);
  KILL(lbits)(&zghz3686);
  KILL(sail_int)(&zghz3685);
  KILL(lbits)(&zghz3684);
  KILL(sail_int)(&zghz3683);
  KILL(lbits)(&zghz3682);
  KILL(sail_int)(&zghz3681);
  KILL(lbits)(&zghz3680);
  KILL(sail_int)(&zghz3679);
  KILL(lbits)(&zghz3678);
  KILL(sail_int)(&zghz3677);
  KILL(lbits)(&zghz3676);
  KILL(sail_int)(&zghz3675);
  KILL(lbits)(&zghz3674);
  KILL(sail_int)(&zghz3673);
  KILL(lbits)(&zghz3672);
  KILL(sail_int)(&zghz3671);
  KILL(lbits)(&zghz3670);
  KILL(sail_int)(&zghz3669);
  KILL(sail_int)(&zghz3668);
  KILL(sail_int)(&zghz3667);
  KILL(lbits)(&zghz3666);
  KILL(sail_int)(&zghz3665);
  KILL(lbits)(&zghz3664);
  KILL(sail_int)(&zghz3663);
  KILL(lbits)(&zghz3662);
  KILL(sail_int)(&zghz3661);
  KILL(lbits)(&zghz3660);
  KILL(sail_int)(&zghz3659);
  KILL(sail_int)(&zghz3658);
  KILL(lbits)(&zghz3657);
  KILL(sail_int)(&zghz3656);
  KILL(lbits)(&zghz3655);
  KILL(sail_int)(&zghz3654);
  KILL(lbits)(&zghz3653);
  KILL(sail_int)(&zghz3652);
  KILL(lbits)(&zghz3651);
  KILL(sail_int)(&zghz3650);
  KILL(lbits)(&zghz3649);
  KILL(sail_int)(&zghz3648);
  KILL(lbits)(&zghz3647);
  KILL(sail_int)(&zghz3646);
  KILL(lbits)(&zghz3645);
  KILL(sail_int)(&zghz3644);
  KILL(lbits)(&zghz3643);
  KILL(sail_int)(&zghz3642);
  KILL(sail_int)(&zghz3641);
  KILL(sail_int)(&zghz3640);
  KILL(sail_int)(&zghz3639);
  KILL(sail_int)(&zghz3638);
  KILL(sail_int)(&zghz3637);
  KILL(sail_int)(&zghz3636);
  KILL(sail_int)(&zghz3635);
  KILL(sail_int)(&zghz3634);
  KILL(lbits)(&zghz3633);
  KILL(sail_int)(&zghz3632);
  KILL(lbits)(&zghz3631);
  KILL(sail_int)(&zghz3630);
  KILL(lbits)(&zghz3629);
  KILL(sail_int)(&zghz3628);
  KILL(lbits)(&zghz3627);
  KILL(sail_int)(&zghz3626);
  KILL(lbits)(&zghz3625);
  KILL(sail_int)(&zghz3624);
  KILL(lbits)(&zghz3623);
  KILL(sail_int)(&zghz3622);
  KILL(lbits)(&zghz3621);
  KILL(sail_int)(&zghz3620);
  KILL(lbits)(&zghz3619);
  KILL(sail_int)(&zghz3618);
  KILL(sail_int)(&zghz3617);
  KILL(sail_int)(&zghz3616);
  KILL(sail_int)(&zghz3615);
  KILL(sail_int)(&zghz3614);
  KILL(sail_int)(&zghz3613);
  KILL(lbits)(&zghz3612);
  KILL(sail_int)(&zghz3611);
  KILL(lbits)(&zghz3610);
  KILL(sail_int)(&zghz3609);
  KILL(lbits)(&zghz3608);
  KILL(sail_int)(&zghz3607);
  KILL(lbits)(&zghz3606);
  KILL(sail_int)(&zghz3605);
  KILL(lbits)(&zghz3604);
  KILL(sail_int)(&zghz3603);
  KILL(lbits)(&zghz3602);
  KILL(sail_int)(&zghz3601);
  KILL(lbits)(&zghz3600);
  KILL(sail_int)(&zghz3599);
  KILL(lbits)(&zghz3598);
  KILL(sail_int)(&zghz3597);
  KILL(lbits)(&zghz3596);
  KILL(sail_int)(&zghz3595);
  KILL(lbits)(&zghz3594);
  KILL(sail_int)(&zghz3593);
  KILL(lbits)(&zghz3592);
  KILL(sail_int)(&zghz3591);
  KILL(lbits)(&zghz3590);
  KILL(sail_int)(&zghz3589);
  KILL(lbits)(&zghz3588);
  KILL(sail_int)(&zghz3587);
  KILL(sail_int)(&zghz3586);
  KILL(sail_int)(&zghz3585);
  KILL(lbits)(&zghz3584);
  KILL(lbits)(&zghz3583);
  KILL(sail_int)(&zghz3582);
  KILL(lbits)(&zghz3581);
  KILL(sail_int)(&zghz3580);
  KILL(lbits)(&zghz3579);
  KILL(sail_int)(&zghz3578);
  KILL(sail_int)(&zghz3577);
  KILL(sail_int)(&zghz3576);
}

static void model_init(void)
{
  setup_rts();
  current_exception = sail_malloc(sizeof(struct zexception));
  CREATE(zexception)(current_exception);
  throw_location = sail_malloc(sizeof(sail_string));
  CREATE(sail_string)(throw_location);
  startup_sailgen_sail_ones();
  startup_sailgen_eq_bits_int();
  startup_sailgen_integer_subrange();
  startup_sailgen_undefined_ProcState();
  startup_sailgen_CapBoundsUsesValue();
  startup_sailgen_CapIsInternalExponent();
  startup_sailgen_CapGetExponent();
  startup_sailgen_CapGetValue();
  startup_sailgen_CapBoundsAddress();
  startup_sailgen_CapGetBottom();
  startup_sailgen_CapUnsignedLessThan();
  startup_sailgen_CapGetTop();
  startup_sailgen_CapUnsignedGreaterThan();
  startup_sailgen_CapGetBounds();
  startup_sailgen_CapBoundsEqual();
  startup_sailgen_CapIsRepresentable();
  startup_sailgen_CapSetTag();
  startup_sailgen_CapWithTagClear();
  startup_sailgen_CapGetObjectType();
  startup_sailgen_CapIsSealed();
  startup_sailgen_CapGetPermissions();
  startup_sailgen_undefined_instr_ast();
  startup_sailgen_CapUnsignedGreaterThanOrEqual();
  startup_sailgen_CapIsRepresentableFast();
  startup_sailgen_CapUnsignedLessThanOrEqual();
  startup_sailgen_CapGetTag();
  startup_sailgen_CapSetBounds();
  startup_sailgen_doCSetBounds();
  startup_sailgen_main();
  startup_sailgen_initializze_registers();
  create_letbind_0();
  create_letbind_1();
  create_letbind_2();
  create_letbind_3();
  create_letbind_4();
  create_letbind_5();
  create_letbind_6();
  create_letbind_7();
  create_letbind_8();
  create_letbind_9();
  create_letbind_10();
  create_letbind_11();
  create_letbind_12();
  create_letbind_13();
  create_letbind_14();
  create_letbind_15();
  create_letbind_16();
  create_letbind_17();
  create_letbind_18();
  create_letbind_19();
  create_letbind_20();
  create_letbind_21();
  create_letbind_22();
  create_letbind_23();
  create_letbind_24();
  create_letbind_25();
  create_letbind_26();
  create_letbind_27();
  create_letbind_28();
  create_letbind_29();
  create_letbind_30();
  create_letbind_31();
  create_letbind_32();
  create_letbind_33();
  create_letbind_34();
  create_letbind_35();
  create_letbind_36();
  create_letbind_37();
  create_letbind_38();
  create_letbind_39();
  create_letbind_40();
  create_letbind_41();
  create_letbind_42();
  create_letbind_43();
  create_letbind_44();
  create_letbind_45();
  create_letbind_46();
  create_letbind_47();
  create_letbind_48();
  create_letbind_49();
  create_letbind_50();
  create_letbind_51();
  create_letbind_52();
  create_letbind_53();
  create_letbind_54();
  create_letbind_55();
  create_letbind_56();
  create_letbind_57();
  create_letbind_58();
  create_letbind_59();
  create_letbind_60();
  create_letbind_61();
  create_letbind_62();
  create_letbind_63();
  create_letbind_64();
  create_letbind_65();
  create_letbind_66();
  create_letbind_67();
  create_letbind_68();
  create_letbind_69();
  create_letbind_70();
  CREATE(sail_int)(&zSEE);
  CREATE(lbits)(&zPCC);
  CREATE(lbits)(&z_R00);
  CREATE(lbits)(&z_R01);
  CREATE(lbits)(&z_R02);
  CREATE(lbits)(&z_R03);
  CREATE(lbits)(&z_R04);
  CREATE(lbits)(&z_R05);
  CREATE(lbits)(&z_R06);
  CREATE(lbits)(&z_R07);
  CREATE(lbits)(&z_R08);
  CREATE(lbits)(&z_R09);
  CREATE(lbits)(&z_R10);
  CREATE(lbits)(&z_R11);
  CREATE(lbits)(&z_R12);
  CREATE(lbits)(&z_R13);
  CREATE(lbits)(&z_R14);
  CREATE(lbits)(&z_R15);
  CREATE(lbits)(&z_R16);
  CREATE(lbits)(&z_R17);
  CREATE(lbits)(&z_R18);
  CREATE(lbits)(&z_R19);
  CREATE(lbits)(&z_R20);
  CREATE(lbits)(&z_R21);
  CREATE(lbits)(&z_R22);
  CREATE(lbits)(&z_R23);
  CREATE(lbits)(&z_R24);
  CREATE(lbits)(&z_R25);
  CREATE(lbits)(&z_R26);
  CREATE(lbits)(&z_R27);
  CREATE(lbits)(&z_R28);
  CREATE(lbits)(&z_R29);
  CREATE(lbits)(&z_R30);
  CREATE(lbits)(&zELR_EL1);
  CREATE(lbits)(&zELR_EL2);
  CREATE(lbits)(&zELR_EL3);
  CREATE(lbits)(&zVBAR_EL1);
  CREATE(lbits)(&zVBAR_EL2);
  CREATE(lbits)(&zVBAR_EL3);
  CREATE(zz5vecz8z5bv32z9)(&zDBGBCR_EL1);
  CREATE(lbits)(&zCDLR_EL0);
  CREATE(zz5vecz8z5bv64z9)(&zDBGBVR_EL1);
  CREATE(lbits)(&zCDBGDTR_EL0);
  CREATE(zz5vecz8z5bv32z9)(&zDBGWCR_EL1);
  CREATE(zz5vecz8z5bv64z9)(&zDBGWVR_EL1);
  CREATE(zz5vecz8z5bv32z9)(&zICC_AP0R_EL1);
  CREATE(zz5vecz8z5bv32z9)(&zICV_AP0R_EL1);
  CREATE(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1);
  CREATE(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_NS);
  CREATE(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_S);
  CREATE(zz5vecz8z5bv32z9)(&zICV_AP1R_EL1);
  CREATE(zz5vecz8z5bv32z9)(&zICH_AP0R_EL2);
  CREATE(zz5vecz8z5bv32z9)(&zICH_AP1R_EL2);
  CREATE(zz5vecz8z5bv64z9)(&zICH_LR_EL2);
  CREATE(zz5vecz8z5bv32z9)(&zPMEVCNTR_EL0);
  CREATE(zz5vecz8z5bv32z9)(&zPMEVTYPER_EL0);
  CREATE(lbits)(&zRSP_EL0);
  CREATE(lbits)(&zRTPIDR_EL0);
  CREATE(lbits)(&zCID_EL0);
  CREATE(lbits)(&zSP_EL0);
  CREATE(lbits)(&zSP_EL1);
  CREATE(lbits)(&zSP_EL2);
  CREATE(lbits)(&zTPIDRRO_EL0);
  CREATE(lbits)(&zTPIDR_EL0);
  CREATE(lbits)(&zTPIDR_EL1);
  CREATE(lbits)(&zTPIDR_EL2);
  CREATE(lbits)(&zTPIDR_EL3);
  CREATE(lbits)(&zDDC_EL0);
  CREATE(lbits)(&zDDC_EL1);
  CREATE(lbits)(&zDDC_EL2);
  CREATE(lbits)(&zDDC_EL3);
  CREATE(lbits)(&zRDDC_EL0);
  CREATE(zz5vecz8z5bvz9)(&z_V);
  CREATE(zinstr_ast)(&z__ThisInstrAbstract);
  CREATE(lbits)(&zSP_EL3);
  CREATE(lbits)(&zTestCap);
  sailgen_initializze_registers(UNIT);
}

static void model_fini(void)
{
  kill_letbind_70();
  kill_letbind_69();
  kill_letbind_68();
  kill_letbind_67();
  kill_letbind_66();
  kill_letbind_65();
  kill_letbind_64();
  kill_letbind_63();
  kill_letbind_62();
  kill_letbind_61();
  kill_letbind_60();
  kill_letbind_59();
  kill_letbind_58();
  kill_letbind_57();
  kill_letbind_56();
  kill_letbind_55();
  kill_letbind_54();
  kill_letbind_53();
  kill_letbind_52();
  kill_letbind_51();
  kill_letbind_50();
  kill_letbind_49();
  kill_letbind_48();
  kill_letbind_47();
  kill_letbind_46();
  kill_letbind_45();
  kill_letbind_44();
  kill_letbind_43();
  kill_letbind_42();
  kill_letbind_41();
  kill_letbind_40();
  kill_letbind_39();
  kill_letbind_38();
  kill_letbind_37();
  kill_letbind_36();
  kill_letbind_35();
  kill_letbind_34();
  kill_letbind_33();
  kill_letbind_32();
  kill_letbind_31();
  kill_letbind_30();
  kill_letbind_29();
  kill_letbind_28();
  kill_letbind_27();
  kill_letbind_26();
  kill_letbind_25();
  kill_letbind_24();
  kill_letbind_23();
  kill_letbind_22();
  kill_letbind_21();
  kill_letbind_20();
  kill_letbind_19();
  kill_letbind_18();
  kill_letbind_17();
  kill_letbind_16();
  kill_letbind_15();
  kill_letbind_14();
  kill_letbind_13();
  kill_letbind_12();
  kill_letbind_11();
  kill_letbind_10();
  kill_letbind_9();
  kill_letbind_8();
  kill_letbind_7();
  kill_letbind_6();
  kill_letbind_5();
  kill_letbind_4();
  kill_letbind_3();
  kill_letbind_2();
  kill_letbind_1();
  kill_letbind_0();
  KILL(sail_int)(&zSEE);
  KILL(lbits)(&zPCC);
  KILL(lbits)(&z_R00);
  KILL(lbits)(&z_R01);
  KILL(lbits)(&z_R02);
  KILL(lbits)(&z_R03);
  KILL(lbits)(&z_R04);
  KILL(lbits)(&z_R05);
  KILL(lbits)(&z_R06);
  KILL(lbits)(&z_R07);
  KILL(lbits)(&z_R08);
  KILL(lbits)(&z_R09);
  KILL(lbits)(&z_R10);
  KILL(lbits)(&z_R11);
  KILL(lbits)(&z_R12);
  KILL(lbits)(&z_R13);
  KILL(lbits)(&z_R14);
  KILL(lbits)(&z_R15);
  KILL(lbits)(&z_R16);
  KILL(lbits)(&z_R17);
  KILL(lbits)(&z_R18);
  KILL(lbits)(&z_R19);
  KILL(lbits)(&z_R20);
  KILL(lbits)(&z_R21);
  KILL(lbits)(&z_R22);
  KILL(lbits)(&z_R23);
  KILL(lbits)(&z_R24);
  KILL(lbits)(&z_R25);
  KILL(lbits)(&z_R26);
  KILL(lbits)(&z_R27);
  KILL(lbits)(&z_R28);
  KILL(lbits)(&z_R29);
  KILL(lbits)(&z_R30);
  KILL(lbits)(&zELR_EL1);
  KILL(lbits)(&zELR_EL2);
  KILL(lbits)(&zELR_EL3);
  KILL(lbits)(&zVBAR_EL1);
  KILL(lbits)(&zVBAR_EL2);
  KILL(lbits)(&zVBAR_EL3);
  KILL(zz5vecz8z5bv32z9)(&zDBGBCR_EL1);
  KILL(lbits)(&zCDLR_EL0);
  KILL(zz5vecz8z5bv64z9)(&zDBGBVR_EL1);
  KILL(lbits)(&zCDBGDTR_EL0);
  KILL(zz5vecz8z5bv32z9)(&zDBGWCR_EL1);
  KILL(zz5vecz8z5bv64z9)(&zDBGWVR_EL1);
  KILL(zz5vecz8z5bv32z9)(&zICC_AP0R_EL1);
  KILL(zz5vecz8z5bv32z9)(&zICV_AP0R_EL1);
  KILL(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1);
  KILL(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_NS);
  KILL(zz5vecz8z5bv32z9)(&zICC_AP1R_EL1_S);
  KILL(zz5vecz8z5bv32z9)(&zICV_AP1R_EL1);
  KILL(zz5vecz8z5bv32z9)(&zICH_AP0R_EL2);
  KILL(zz5vecz8z5bv32z9)(&zICH_AP1R_EL2);
  KILL(zz5vecz8z5bv64z9)(&zICH_LR_EL2);
  KILL(zz5vecz8z5bv32z9)(&zPMEVCNTR_EL0);
  KILL(zz5vecz8z5bv32z9)(&zPMEVTYPER_EL0);
  KILL(lbits)(&zRSP_EL0);
  KILL(lbits)(&zRTPIDR_EL0);
  KILL(lbits)(&zCID_EL0);
  KILL(lbits)(&zSP_EL0);
  KILL(lbits)(&zSP_EL1);
  KILL(lbits)(&zSP_EL2);
  KILL(lbits)(&zTPIDRRO_EL0);
  KILL(lbits)(&zTPIDR_EL0);
  KILL(lbits)(&zTPIDR_EL1);
  KILL(lbits)(&zTPIDR_EL2);
  KILL(lbits)(&zTPIDR_EL3);
  KILL(lbits)(&zDDC_EL0);
  KILL(lbits)(&zDDC_EL1);
  KILL(lbits)(&zDDC_EL2);
  KILL(lbits)(&zDDC_EL3);
  KILL(lbits)(&zRDDC_EL0);
  KILL(zz5vecz8z5bvz9)(&z_V);
  KILL(zinstr_ast)(&z__ThisInstrAbstract);
  KILL(lbits)(&zSP_EL3);
  KILL(lbits)(&zTestCap);
  finish_sailgen_sail_ones();
  finish_sailgen_eq_bits_int();
  finish_sailgen_integer_subrange();
  finish_sailgen_undefined_ProcState();
  finish_sailgen_CapBoundsUsesValue();
  finish_sailgen_CapIsInternalExponent();
  finish_sailgen_CapGetExponent();
  finish_sailgen_CapGetValue();
  finish_sailgen_CapBoundsAddress();
  finish_sailgen_CapGetBottom();
  finish_sailgen_CapUnsignedLessThan();
  finish_sailgen_CapGetTop();
  finish_sailgen_CapUnsignedGreaterThan();
  finish_sailgen_CapGetBounds();
  finish_sailgen_CapBoundsEqual();
  finish_sailgen_CapIsRepresentable();
  finish_sailgen_CapSetTag();
  finish_sailgen_CapWithTagClear();
  finish_sailgen_CapGetObjectType();
  finish_sailgen_CapIsSealed();
  finish_sailgen_CapGetPermissions();
  finish_sailgen_undefined_instr_ast();
  finish_sailgen_CapUnsignedGreaterThanOrEqual();
  finish_sailgen_CapIsRepresentableFast();
  finish_sailgen_CapUnsignedLessThanOrEqual();
  finish_sailgen_CapGetTag();
  finish_sailgen_CapSetBounds();
  finish_sailgen_doCSetBounds();
  finish_sailgen_main();
  finish_sailgen_initializze_registers();
  cleanup_rts();
  if (have_exception) {fprintf(stderr, "Exiting due to uncaught exception: %s\n", *throw_location);}
  KILL(zexception)(current_exception);
  sail_free(current_exception);
  KILL(sail_string)(throw_location);
  sail_free(throw_location);
  if (have_exception) {exit(EXIT_FAILURE);}
}

void model_pre_exit()
{
}

static int model_main(int argc, char *argv[])
{
  model_init();
  if (process_arguments(argc, argv)) exit(EXIT_FAILURE);
  sailgen_main(UNIT);
  model_fini();
  model_pre_exit();
  return EXIT_SUCCESS;
}


