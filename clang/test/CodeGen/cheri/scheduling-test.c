// REQUIRES: mips-registered-target
// Run with delay slot filling disabled since this makes the test fragile by moving instructions after scheduling
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=beri -S %s -o - -O2 -fPIC -mllvm -disable-mips-delay-filler | FileCheck %s -check-prefixes CHECK,BERI
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -march=beri -S %s -o - -O2 -fPIC -mllvm -disable-mips-delay-filler | FileCheck %s -check-prefixes CHECK,BERI
// RUN: %cheri_purecap_cc1 -S %s -o - -O2 -mllvm -disable-mips-delay-filler | FileCheck %s -check-prefixes CHECK,PURECAP
// With -mcpu=mips4 we get lots of pipeline bubbles:
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=mips4 -S %s -o - -O2 -fPIC -mllvm -disable-mips-delay-filler  | FileCheck %s -check-prefixes CHECK,MIPS4
// Check that adding the -cheri flag enables BERI scheduling:
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=mips4 -cheri=128 -S %s -o - -O2 -fPIC -mllvm -disable-mips-delay-filler  | FileCheck %s -check-prefixes CHECK,BERI

// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=mips4 -c %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-MIPS4
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=mips4 -cheri=128 -c %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-CHERI128
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=beri -c %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-BERI
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -march=beri -c %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-BERI
// RUN: %clang -target mips64-unknown-freebsd -mabi=n64 -mcpu=mips4 -Xclang -target-feature -Xclang +beri -c %s -o - | llvm-readobj -h - | FileCheck %s -check-prefixes FLAGS,FLAGS-BERI
// FLAGS:                Flags [
// FLAGS-CHERI128-SAME:   (0x30C10007)
// FLAGS-BERI-SAME:       (0x30BE0007)
// FLAGS-MIPS4-SAME:      (0x30000007)
// FLAGS-NEXT:             EF_MIPS_ARCH_4 (0x30000000)
// FLAGS-NEXT:             EF_MIPS_CPIC (0x4)
// FLAGS-BERI-NEXT:        EF_MIPS_MACH_BERI (0xBE0000)
// FLAGS-CHERI128-NEXT:    EF_MIPS_MACH_CHERI128 (0xC10000)
// FLAGS-NEXT:             EF_MIPS_NOREORDER (0x1)
// FLAGS-NEXT:             EF_MIPS_PIC (0x2)
// FLAGS-NEXT:           ]


// TODO: This should be an IR level test but that's annoying to get working for purecap and non-purecap

long test2(const void* a1, const void* a2) {
  const long l1 = *(const long*)a1;
  const long l2 = *(const long*)a2;
  const long diff = (const char*)a1 - (const char*)a2;
  return l1 - l2 + diff;
  // Check that there are two instructions between load and use (this does not happen with -mcpu=mips4)
  // CHECK-LABEL: test2:
  // Check that there are two instructions between load of $1/$2 and the use:
  // BERI:      ld	$1, 0($4)
  // BERI-NEXT: ld	$2, 0($5)
  // BERI-NEXT: dsubu	$3, $4, $5
  // BERI-NEXT: daddu	$1, $3, $1
  // BERI-NEXT: dsubu	$2, $1, $2

  // Same for purecap:
  // PURECAP:      cld	$1, $zero, 0($c3)
  // PURECAP-NEXT: cld	$2, $zero, 0($c4)
  // PURECAP-NEXT: csub	$3, $c3, $c4
  // PURECAP-NEXT: dsubu	$1, $1, $2
  // PURECAP-NEXT: daddu	$2, $1, $3

  // With -mcpu=mips we use the load result immediately
  // MIPS4:      dsubu	$1, $4, $5
  // MIPS4-NEXT: ld	$2, 0($4)
  // MIPS4-NEXT: daddu	$1, $1, $2
  // MIPS4-NEXT: ld	$2, 0($5)
  // MIPS4-NEXT: dsubu	$2, $1, $2
}

long test3(const void* a1, const void* a2, long extra1, long extra2) {
  const long l1 = *(const long*)a1;
  const long l2 = *(const long*)a2;
  const long extra = extra1 + extra2;
  return l1 - l2 + extra;
  // CHECK-LABEL: test3:
  // Check that there are two instructions between load of $1/$2 and the use:
  // BERI:      ld	$1, 0($4)
  // BERI-NEXT: ld	$2, 0($5)
  // BERI-NEXT: daddu	$3, $7, $6
  // BERI-NEXT: daddu	$1, $3, $1
  // BERI-NEXT: dsubu	$2, $1, $2

  // And the same for purecap:
  // PURECAP:       cld	$1, $zero, 0($c3)
  // PURECAP-NEXT: cld	$2, $zero, 0($c4)
  // PURECAP-NEXT: daddu	$3, $5, $4
  // PURECAP-NEXT: daddu	$1, $3, $1
  // PURECAP-NEXT: dsubu	$2, $1, $2

  // But -mcpu=mips4 just uses it immediately
  // MIPS4:      daddu	$1, $7, $6
  // MIPS4-NEXT: ld	$2, 0($4)
  // MIPS4-NEXT: daddu	$1, $1, $2
  // MIPS4-NEXT: ld	$2, 0($5)
  // MIPS4-NEXT: dsubu	$2, $1, $2
}
