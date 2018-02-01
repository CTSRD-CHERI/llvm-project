// RUN: %cheri_purecap_cc1 -o - -O0 -emit-llvm  %s
// Found while trying to use the builtin in QtBase
void a(char* c, int b) {
  __builtin_p2align_down(c, b);
}
