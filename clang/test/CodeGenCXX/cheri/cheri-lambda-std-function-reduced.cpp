// https://github.com/CTSRD-CHERI/clang/issues/148
// It now fails to compile whereas before it would crash the compiler
// RUN: not %cheri_purecap_cc1 -emit-llvm -std=c++11 -o /dev/null %s

inline namespace a {
template <class> struct d;
template <class ab> struct d<ab &> { typedef ab b; };
template <class ab> typename d<ab>::b ag(ab &&) {}
template <long, class> class e;
template <unsigned long...> struct f {};
template <class at, at... au> struct g { template <long> using aw = f<au...>; };
template <long ax, long ay>
using az = typename __make_integer_seq<g, long, ax>::template aw<ay>;
template <int ax, long ay = 0> struct h { typedef az<ax, ay> b; };
template <class...> class i;
template <long bc, class... ab> typename e<bc, i<ab...>>::b bd(i<ab...> &);
template <class> struct H;
template <long bc, class... bf> class e<bc, H<bf...>> {
public:
  typedef __type_pack_element<bc, bf...> b;
};
template <long bc, class... ab> class e<bc, i<ab...>> {
public:
  typedef typename e<bc, H<ab...>>::b b;
};
int bh;
template <class bi> class bj { bi c; };
template <class...> struct j;
template <long... bl, class... ab> struct j<f<bl...>, ab...> : bj<ab>... {};
template <class... ab> class i { j<typename h<sizeof...(ab)>::b, ab...> bm; };
template <class... ab> i<ab &&...> bn(ab...);
template <class bo> class k : bo {
public:
  template <class... br, unsigned long... bs>
  k(int, i<br...> l, i<>, f<bs...>, f<>) : bo(bd<bs>(l)...) {}
};
template <class bo> class m : k<bo> {
public:
  template <class... br, class... bv>
  m(int n, i<br...> l, i<> p3)
      : k<bo>(n, ag(l), p3, typename h<sizeof...(br)>::b(),
              typename h<sizeof...(bv)>::b()) {}
};
template <class> class by;
template <class, class> class cb;
template <class ah, class cd, class... ce> class cb<ah, cd(ce...)> {
  m<ah> cf;

public:
  cb(ah n) : cf(bh, bn(n), bn()) {}
};
template <class cd, class... ce> class by<cd(ce...)> {
public:
  template <class ah, class = void> by(ah);
};
template <class cd, class... ce>
template <class ah, class>
by<cd(ce...)>::by(ah n) {
  cb<ah, cd()> ag(n);
}
}
void ci(by<void()>) {
  auto cj = [] {};
  ci(cj);
}

