// RUN: %cheri_purecap_cc1 -mdisable-fp-elim -target-cpu mips4 -msoft-float -mfloat-abi soft  -target-linker-version 305 \
// RUN: -std=c++11 -fdeprecated-macro -fcxx-exceptions -fexceptions -o - -O0  -S %s 2>&1
template <int a, int, int, int, int, class... b> void c(b...) {
  double k __attribute__((__aligned__(a)));
}
template <int, int d, int e, int, int f> void g() {
  constexpr int a = 4096;
  constexpr int h = 2048;
  try {
    c<a, d, h, e, f, double, double, double, double, double, double, double,
      double, double, double, double, double, double, double, double, double,
      double, double, double, double>;
  } catch (int) {
  }
}
const bool i = true;
void j() { g<i, i, i, i, i>; }
