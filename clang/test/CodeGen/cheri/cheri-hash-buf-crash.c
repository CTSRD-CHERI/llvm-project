// Reproducer for a crash in the new folding intrinsics
// For some reason the generated IR does not crash when using llc, so let's keep this test in clang

// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -emit-obj -disable-free -disable-llvm-verifier -discard-value-names -main-file-name hash_buf.c -mrelocation-model pic -pic-level 1 -mthread-model posix -mdisable-fp-elim -masm-verbose -mconstructor-aliases -target-feature +soft-float -target-abi purecap -Wmips-cheri-prototypes -msoft-float -mfloat-abi soft -mllvm -mxgot -mllvm -mips-ssection-threshold=0 -dwarf-column-info -debug-info-kind=standalone -dwarf-version=2 -debugger-tuning=gdb -coverage-notes-file /home/en322/cheri/build/cheribsd-obj-256/mips.mips64/usr/home/en322/cheri/cheribsd/worldcheri/usr/home/en322/cheri/cheribsd/lib/libc/hash_buf.gcno -sys-header-deps -D NLS -D __DBINTERFACE_PRIVATE -D NO_COMPAT7 -D INET6 -D _ACL_PRIVATE -D POSIX_MISTAKE -D MALLOC_PRODUCTION -D BROKEN_DES -D PORTMAP -D DES_BUILTIN -D SOFTFLOAT_FOR_GCC -D YP -D NS_CACHING -D SYMBOL_VERSIONING -D __LP64__=1 -O2 -Wno-deprecated-declarations -Werror=implicit-function-declaration -Wsystem-headers -Wall -Wno-format-y2k -Wno-uninitialized -Wno-pointer-sign -Wno-empty-body -Wno-string-plus-int -Wno-unused-const-variable -Wno-tautological-compare -Wno-unused-value -Wno-parentheses-equality -Wno-unused-function -Wno-enum-conversion -Wno-unused-local-typedef -Wno-address-of-packed-member -Wno-switch -Wno-switch-enum -Wno-knr-promoted-parameter -std=gnu99 -ferror-limit 19 -fmessage-length 0 -ftls-model=local-exec -fobjc-runtime=gnustep -fdiagnostics-show-option -vectorize-loops -vectorize-slp -cheri-linker -x c -o /dev/null %s
typedef a;
typedef b;
#define c(d) (b *)(a) d
*f;
e() { f = c(e); }
