// https://github.com/CTSRD-CHERI/llvm/issues/245
// RUN: %cheri_purecap_cc1 -emit-llvm -O2 %s -o %t.ll
// RUN: cat %t.ll
// RUN: %cheri_purecap_llc -O2 %t.ll -o /dev/null
// RUN: %cheri_purecap_cc1 -emit-obj -O2 %s -o /dev/null  -S
static int __attribute__((__section__(".data.read_mostly"))) a;
int b() {
  if (b)
    return 0;
c:
  d();
  a++;
  goto c;
}


#if 0 // original test case:
#define a(b) __attribute__((__section__(b)))
#define d(h, ab, c) for (h = ab; 0;)
struct {
  char bi;
  long bj;
} * e;
struct bl {
  struct bm *bn
};
struct bm {
  struct {
    int *f;
  };
  struct bl br;
  struct {
    struct bl bs;
  };
  char bt
}
#define bx() &bz[ca]
    * bz,
    g;
static int a(".data.read_mostly") ca, cd, t;
int cc;
#define ap(aq, ar) at ar
#define au() ap(, (""));
#define bc(c, b) au()
int i(){bc(, );}
int j(k) {
  int *l = 0;
  struct bl *co = 0;
  struct bm *cf;
  if (cc) {
    e = 0;
    return 0;
  }
ct:
  if (e->bi == '.')
    if (e->bj == '.') {
      i();
    cu:
      cf = co->bn;
      if (e == 0) {
        if (co->bn) {
          int ci, cj;
          az(g);
          m(&ci, &cj);
          n(cj);
          n(ci);
          az(0);
          goto cu;
        }
        return 0;
      }
      if (cf)
        o(44, "", e, l);
      p(t, k);
      if (cf->bt == 8)
        goto cx;
    }
  q(&e->bi, e);
  d(cf, bx(), ) cx : az(co = l);
  r(co, 024);
  l = cf;
  cd++;
  goto ct;
}
#endif
