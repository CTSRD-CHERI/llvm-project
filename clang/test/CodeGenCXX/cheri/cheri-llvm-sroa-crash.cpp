// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=legacy -S -target-cpu mips4 -Os -std=c++1z -pthread -fcolor-diagnostics -vectorize-loops -vectorize-slp -o /dev/null %s
// RUN: not %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -emit-obj -target-cpu mips4 -Os -std=c++1z \
// RUN:   -pthread -fcolor-diagnostics -vectorize-loops -vectorize-slp -o - %s 2>&1 | FileCheck %s -check-prefix HARDFLOAT-ERR
// HARDFLOAT-ERR: error in backend: Cannot handle constant pools in captable ABI. Try compiling with -msoft-float instead
// RUN: %cheri_purecap_cc1 -mllvm -cheri-cap-table-abi=pcrel -S -target-cpu mips4 -Os -std=c++1z \
// RUN:   -pthread -fcolor-diagnostics -vectorize-loops -vectorize-slp -msoft-float -o /dev/null %s

// These two reduced tests were previously crashing with the following
// assertion failure in LLVM's SROA transform when compiling qtbase:
//
// Assertion `Offset.getBitWidth() == DL.getPointerBaseSizeInBits(getPointerAddressSpace()) && "The offset must have exactly as many bits as our pointer."' failed.
//
// The two QtBase files from which the reduced test cases were produced are named before each test

// from qquaternion.cpp

class a {
  public:
    a(float);
    float b, c, e, d;
};
a operator*(a &h, float) { return h.d; }
inline a operator-(a) {}
float f;
void g(a h) {
  a i(h);
  -i = i * f;
}

// from qvector4d.cpp

bool d(double);
class e {
  constexpr e(float, float, float, float);
  e f() const;
  float g, h, i, j;
};
constexpr e::e(float, float, float, float) : g(), h(), i(), j() {}
e e::f() const {
  if (d(1.0f))
    return *this;
  return e(float(), float(), float(), float());
}
