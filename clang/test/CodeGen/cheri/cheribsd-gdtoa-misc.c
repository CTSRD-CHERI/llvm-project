// RUN: %cheri_purecap_cc1 "-emit-obj" "-disable-free" "-main-file-name" "misc.c" "-mrelocation-model" "pic" "-pic-level" "1" "-mthread-model" "posix" "-mdisable-fp-elim" "-masm-verbose" "-mconstructor-aliases" "-target-feature" "+soft-float" "-Wmips-cheri-prototypes" "-msoft-float" "-mfloat-abi" "soft" "-mllvm" "-mxgot" "-mllvm" "-mips-ssection-threshold=0" "-dwarf-column-info" "-debug-info-kind=standalone" "-dwarf-version=2" "-debugger-tuning=gdb" "-coverage-notes-file" "/home/alr48/cheri/build-postmerge/cheribsd-obj-256/mips.mips64/exports/users/alr48/sources/cheribsd-postmerge/lib/libc_cheri/misc.gcno" "-sys-header-deps" "-D" "FORCE_C_LOCALE" "-D" "FORCE_UTC_TZ" "-D" "__LP64__=1" "-O2" "-Wno-deprecated-declarations" "-Wno-cast-align" "-Wsystem-headers" "-Wall" "-Wno-format-y2k" "-W" "-Wno-unused-parameter" "-Wstrict-prototypes" "-Wmissing-prototypes" "-Wpointer-arith" "-Wreturn-type" "-Wcast-qual" "-Wwrite-strings" "-Wswitch" "-Wshadow" "-Wunused-parameter" "-Wcast-align" "-Wchar-subscripts" "-Winline" "-Wnested-externs" "-Wredundant-decls" "-Wold-style-definition" "-Wno-pointer-sign" "-Wmissing-variable-declarations" "-Wthread-safety" "-Wno-empty-body" "-Wno-string-plus-int" "-Wno-unused-const-variable" "-Wno-sign-compare" "-Wno-sign-compare" "-std=gnu99" "-fconst-strings" "-ferror-limit" "19" "-fmessage-length" "0" "-ftls-model=local-exec" "-fobjc-runtime=gnustep" "-fdiagnostics-show-option" "-vectorize-loops" "-vectorize-slp" "-cheri-linker" "-mllvm" "-mxmxgot" "-o" "/dev/null" "-x" "c" %s

// Compiling contrib/gdtoa/misc.c used to crash

#define	Long	int
#ifndef ULong
typedef unsigned Long ULong;
#endif
#ifndef UShort
typedef unsigned short UShort;
#endif

#ifndef ANSI
#ifdef KR_headers
#define ANSI(x) ()
#define Void /*nothing*/
#else
#define ANSI(x) x
#define Void void
#endif
#endif /* ANSI */

#ifndef CONST
#ifdef KR_headers
#define CONST /* blank */
#else
#define CONST const
#endif
#endif /* CONST */

 struct
Bigint {
	struct Bigint *next;
	int k, maxwds, sign, wds;
	ULong x[1];
	};

 typedef struct Bigint Bigint;

#define	cmp		__cmp_D2A
 extern int cmp ANSI((Bigint*, Bigint*));
 int
cmp
#ifdef KR_headers
	(a, b) Bigint *a, *b;
#else
	(Bigint *a, Bigint *b)
#endif
{
	ULong *xa, *xa0, *xb, *xb0;
	int i, j;

	i = a->wds;
	j = b->wds;
#ifdef DEBUG
	if (i > 1 && !a->x[i-1])
		Bug("cmp called with a->x[a->wds-1] == 0");
	if (j > 1 && !b->x[j-1])
		Bug("cmp called with b->x[b->wds-1] == 0");
#endif
	if (i -= j)
		return i;
	xa0 = a->x;
	xa = xa0 + j;
	xb0 = b->x;
	xb = xb0 + j;
	for(;;) {
		if (*--xa != *--xb)
			return *xa < *xb ? -1 : 1;
		if (xa <= xa0)
			break;
		}
	return 0;
	}
