// RUN: %cheri_clang -v -S -o - -O2 %s -mllvm -print-before=cheriaddrmodefolder -mllvm -print-after=cheriaddrmodefolder | FileCheck %s
// REQUIRES: asserts
// XFAIL: *

// https://github.com/CTSRD-CHERI/llvm-project/issues/292

// CHECK-LABEL doLoop:
// CHECK:      daddiu	$sp, $sp, -16
// CHECK-NEXT: sd	$fp, 8($sp) # 8-byte Folded Spill
// CHECK-NEXT: move	$fp, $sp
// CHECK-NEXT: cfromptr	$1, $c1, $c0
// CHECK-NEXT: cfromptr	$2, $c2, $c0
// CHECK-NEXT: clb	$1, $6, 1($c1)
// CHECK-NEXT: csb	$1, $6, 1($c2)
// CHECK-NEXT: clb	$1, $6, 2($c1)
// CHECK-NEXT: csb	$1, $6, 2($c2)
// CHECK-NEXT: move	$sp, $fp
// CHECK-NEXT: ld	$fp, 8($sp) # 8-byte Folded Reload
// CHECK-NEXT: jr	$ra
// CHECK-NEXT: daddiu	$sp, $sp, 16
// CHECK-NEXT:

void doLoop(char * in, char * out, int i)
{
  char * __capability inLocal = (__cheri_tocap char * __capability)in;
  char * __capability outLocal = (__cheri_tocap char * __capability)out;
  outLocal[i+1]=inLocal[i+1];
  outLocal[i+2]=inLocal[i+2];
}

void doLoop2(char * in, char * out, long i)
{
	char * __capability inLocal = (char * __capability)in;
	char * __capability outLocal = (char * __capability)out;
	do {
		outLocal[i]=inLocal[i];
		i--;
	} while (i>0);
}
