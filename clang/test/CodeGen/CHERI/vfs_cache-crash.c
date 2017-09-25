// https://github.com/CTSRD-CHERI/llvm/issues/245
// RUN: %cheri_purecap_cc1 "-emit-obj" "-disable-free" "-disable-llvm-verifier" "-discard-value-names" "-main-file-name" "vfs_cache.c" "-mrelocation-model" "pic" "-pic-level" "1" "-mthread-model" "posix" "-mdisable-fp-elim" "-masm-verbose" "-mconstructor-aliases" "-target-feature" "+soft-float" "-msoft-float" "-mfloat-abi" "soft" "-mllvm" "-mips-ssection-threshold=0" "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb" "-nostdsysteminc" "-nobuiltininc" "-sys-header-deps" "-D" "_KERNEL" "-D" "HAVE_KERNEL_OPTION_HEADERS" "-D" "KERNLOADADDR=0xffffffff80100000" "-D" "__printf__=__freebsd_kprintf__" "-O2" "-Wno-error" "-std=iso9899:1999" "-ferror-limit" "19" "-fmessage-length" "0" "-ffreestanding" "-fwrapv" "-fobjc-runtime=gnustep" "-fdiagnostics-show-option" "-fcolor-diagnostics" "-vectorize-loops" "-vectorize-slp" "-cheri-linker" %s -o /dev/null

#define a(b) __attribute__((__section__(b)))
#define d(h, ab, c) for (h = ab; 0;)
struct {
  char bi;
  long bj
} * e;
struct bl {
  struct bm *bn
};
struct bm {
  struct {
    int *f
  };
  struct bl br;
  struct {
    struct bl bs
  };
  char bt
}
#define bx() &bz[ca]
    * bz,
    g;
static a(".data.read_mostly") ca, cd, t;
cc;
#define ap(aq, ar) at ar
#define au() ap(, (""));
#define bc(c, b) au()
i(){bc(, )} j(k) {
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
