// RUN: %cheri_clang_cc1 -triple cheri-unknown-freebsd -emit-obj -disable-free -main-file-name opt.c -mrelocation-model pic -pic-level 2 -mthread-model posix -fno-signed-zeros -masm-verbose -mconstructor-aliases -target-feature +soft-float -target-abi purecap -Wmips-cheri-prototypes -msoft-float -mfloat-abi soft -dwarf-column-info -debug-info-kind=standalone -dwarf-version=2 -debugger-tuning=gdb -coverage-notes-file /local/scratch/rmk35/ffmpeg/libavutil/opt.gcno -D _ISOC99_SOURCE -D _FILE_OFFSET_BITS=64 -D _LARGEFILE_SOURCE -D PIC -D HAVE_AV_CONFIG_H -O3 -Wdeclaration-after-statement -Wall -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wwrite-strings -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes -Wempty-body -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wno-pointer-sign -Wno-unused-const-variable -Werror=implicit-function-declaration -Werror=missing-prototypes -Werror=return-type -std=c11 -fconst-strings -ferror-limit 19 -fmessage-length 159 -pthread -mstack-alignment=16 -fobjc-runtime=gnustep -fdiagnostics-show-option -fcolor-diagnostics -vectorize-loops -vectorize-slp -cheri-linker -x c %s -o -
#define AV_OPT_SERIALIZE_SKIP_DEFAULTS 1
# 1 "" 3
a, b;
*c;
fn1(p1) {
  int d;
  while (c)
    if (p1 && c != b)
      if (p1 & AV_OPT_SERIALIZE_SKIP_DEFAULTS && fn2())
        a = d++;
}
