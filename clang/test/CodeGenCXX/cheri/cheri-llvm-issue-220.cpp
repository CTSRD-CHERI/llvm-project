// Check that the source from https://github.com/CTSRD-CHERI/llvm/issues/220 doesn't crash:

// RUN: %clang_cc1 -triple mips64-unknown-freebsd -emit-obj -disable-free -disable-llvm-verifier -discard-value-names -main-file-name fbsd-kvm.c -static-define -mrelocation-model pic -pic-level 2 -mthread-model posix -mdisable-fp-elim -relaxed-aliasing -mconstructor-aliases -target-cpu mips64r2 -target-feature +soft-float -target-abi n64 -msoft-float -mfloat-abi soft -mllvm -mxgot -mllvm -mips-ssection-threshold=0 -dwarf-column-info -debug-info-kind=standalone -dwarf-version=2 -debugger-tuning=gdb -coverage-notes-file /usr/home/en322/cheri/build/gdb-128-build/gdb/fbsd-kvm.gcno -D RL_NO_COMPAT -D LIBICONV_PLUG -D 'LOCALEDIR="/usr/local/share/locale"' -D HAVE_CONFIG_H -D TUI=1 -D RL_NO_COMPAT -D LIBICONV_PLUG -O2 -Wall -Werror=cheri-capability-misuse -Werror=implicit-function-declaration -Werror=format -Werror=undefined-internal -Werror=incompatible-pointer-types -Werror=mips-cheri-prototypes -Wno-absolute-value -Wno-parentheses-equality -Wno-unknown-warning-option -Wno-unused-function -Wno-unused-variable -Wno-error=implicit-function-declaration -Wno-error=format -Wno-error=incompatible-pointer-types -Wall -Werror=cheri-capability-misuse -Werror=implicit-function-declaration -Werror=format -Werror=undefined-internal -Werror=incompatible-pointer-types -Werror=mips-cheri-prototypes -Wno-absolute-value -Wno-parentheses-equality -Wno-unknown-warning-option -Wno-unused-function -Wno-unused-variable -Wno-error=implicit-function-declaration -Wno-error=format -Wno-error=incompatible-pointer-types -Wall -Wpointer-arith -Wno-unused -Wunused-value -Wunused-function -Wno-switch -Wno-char-subscripts -Wempty-body -Wunused-but-set-parameter -Wunused-but-set-variable -Wno-sign-compare -Wno-write-strings -Wno-narrowing -Wformat-nonliteral -fdeprecated-macro -ferror-limit 19 -fmessage-length 0 -fobjc-runtime=gnustep -fcxx-exceptions -fexceptions -fdiagnostics-show-option -vectorize-loops -vectorize-slp -cheri-linker -x c++ %s -o /dev/null
typedef int a;
#if 1
struct b {
  b();
}
#if c == d
#define e(f, g)                                                                \
  }                                                                            \
  catch (gdb_exception_##g)
#endif
;
struct gdb_exception_RETURN_MASK_ERROR {};
#endif
static a h() try {
  b i;
  e(, RETURN_MASK_ERROR) {}
  void j() {
    try {
      b i;
      e(, RETURN_MASK_ERROR) {}
      h();
    }
