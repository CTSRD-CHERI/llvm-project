// RUN: %cheri_purecap_cc1 %s -emit-llvm -mrelocation-model pic -pic-level 1 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o /dev/null
// check that creating assembly for this doesn't crash the SelctionDAG
// RUN: %cheri_purecap_cc1 %s -S -mrelocation-model pic -pic-level 1 -std=gnu99 -mllvm -cheri-cap-table-abi=plt -x c -o /dev/null
struct {
  void *a;
  void *b;
} _cheri_system_object;
__attribute__((cheri_ccall))
__attribute__((cheri_method_class(_cheri_system_object))) int
cheri_system_putchar(void) {
  return cheri_system_putchar();
}
