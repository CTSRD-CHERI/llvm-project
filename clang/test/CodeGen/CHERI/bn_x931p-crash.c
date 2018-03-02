// RUN: %cheri_purecap_cc1 -O2 -std=gnu89 -ftls-model=local-exec -fcolor-diagnostics -vectorize-loops -vectorize-slp -o /dev/null  -S %s
// This previously caused clang to crash
struct a {
  long *b
} c(struct a d, *e, *f, struct a g, struct a h, struct a i, *j, *k, *l) {
  if (j)
    m();
  n();
  if (f)
    n();
  if (o())
    if (o())
      if (p())
        if (q())
          if (p())
            if (q())
              if (p())
                r() && s();
  if (t())
    if (s())
      for (;;) {
        if (u())
          goto aa;
        if (v())
          w();
        if (x())
          s();
      }
  y(l);
aa:
  z();
}
