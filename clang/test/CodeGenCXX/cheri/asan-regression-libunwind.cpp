// RUN: %cheri_purecap_cc1 -emit-llvm -O2 -std=c++11 -fdeprecated-macro -fvisibility hidden -ftls-model=local-exec -fno-rtti -fcxx-exceptions -fexceptions -vectorize-loops -vectorize-slp -o - %s
// This previously crashed when compiling libunwind

typedef __uintcap_t a;
class b {
public:
  b(void *);
};
class c {
public:
  typedef a h;
  void d(h);
  static c e;
};
template <typename f> class g {
public:
  typedef typename f::h h;
  static int o();
  static h i(f &);
  static h j() {
    f k;
    i(k);
  }
};
template <typename f> int g<f>::o() {
  {
    int l;
    j;
  }
}
template <typename f> typename f::h g<f>::i(f &p1) {
  h n, s;
  while (s) {
    char p;
    p1.d(n);
    n += 2;
    n = n + p;
  }
}
template <typename f, typename m> class D {
public:
  D(int *, f &);
  m q;
};
template <typename f, typename m> D<f, m>::D(int *p1, f &) : q(p1) { g<f>::o; }
int r;
void t() { D<c, b>(&r, c::e); }
