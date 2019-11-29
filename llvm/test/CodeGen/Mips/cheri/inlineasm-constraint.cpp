// RUN: %cheri_cc1 -S -O0 %s -o - | FileCheck %s -check-prefix MIPS
// RUN: %cheri_purecap_cc1 -S -O0 %s -o - | FileCheck %s -check-prefix PURECAP


template <class Tp>
__attribute__((always_inline)) void DoNotOptimize(Tp& value) {
#ifdef __CHERI_PURE_CAPABILITY__
  asm volatile("clb $zero, $zero, %0" : "+r,m"(value) : : "memory");
#else
  asm volatile("lb $zero, %0" : "+r,m"(value) : : "memory");
#endif
}

char DummyData4[4];
char DummyData8[8];
char DummyData16[16];
char DummyData32[32];
char DummyData64[64];

char test() {
  DoNotOptimize(DummyData4);
  DoNotOptimize(DummyData8);
  DoNotOptimize(DummyData16);
  DoNotOptimize(DummyData32);
  DoNotOptimize(DummyData64);
  return DummyData4[0];
}

// PURECAP-LABEL: _Z4testv:
// PURECAP: clcbi	[[DATA4_PTR:\$c[0-9]+]], %captab20(DummyData4)($c1)
// PURECAP-NEXT: csc	[[DATA4_PTR]], $zero, [[DATA4_ON_STACK:0\(\$c[0-9]+\)]]
// PURECAP-NEXT: clc	[[DATA4:\$c[0-9]+]], $zero, [[DATA4_ON_STACK]]
// PURECAP: #APP
// PURECAP: clb	$zero, $zero, 0([[DATA4]])
// PURECAP: #NO_APP

// PURECAP: clcbi	[[DATA8_PTR:\$c[0-9]+]], %captab20(DummyData8)($c1)
// PURECAP-NEXT: csc	[[DATA8_PTR]], $zero, [[DATA8_ON_STACK:0\(\$c[0-9]+\)]]
// PURECAP-NEXT: clc	[[DATA8:\$c[0-9]+]], $zero, [[DATA8_ON_STACK]]
// PURECAP: #APP
// PURECAP: clb	$zero, $zero, 0([[DATA8]])
// PURECAP: #NO_APP

// PURECAP: clcbi	[[DATA16_PTR:\$c[0-9]+]], %captab20(DummyData16)($c1)
// PURECAP-NEXT: csc	[[DATA16_PTR]], $zero, [[DATA16_ON_STACK:0\(\$c[0-9]+\)]]
// PURECAP-NEXT: clc	[[DATA16:\$c[0-9]+]], $zero, [[DATA16_ON_STACK]]
// PURECAP: #APP
// PURECAP: clb	$zero, $zero, 0([[DATA16]])
// PURECAP: #NO_APP

// PURECAP: clcbi	[[DATA32_PTR:\$c[0-9]+]], %captab20(DummyData32)($c1)
// PURECAP-NEXT: csc	[[DATA32_PTR]], $zero, [[DATA32_ON_STACK:0\(\$c[0-9]+\)]]
// PURECAP-NEXT: clc	[[DATA32:\$c[0-9]+]], $zero, [[DATA32_ON_STACK]]
// PURECAP: #APP
// PURECAP: clb	$zero, $zero, 0([[DATA32]])
// PURECAP: #NO_APP

// PURECAP: clcbi	[[DATA64_PTR:\$c[0-9]+]], %captab20(DummyData64)($c1)
// PURECAP-NEXT: csc	[[DATA64_PTR]], $zero, [[DATA64_ON_STACK:0\(\$c[0-9]+\)]]
// PURECAP-NEXT: clc	[[DATA64:\$c[0-9]+]], $zero, [[DATA64_ON_STACK]]
// PURECAP: #APP
// PURECAP: clb	$zero, $zero, 0([[DATA64]])
// PURECAP: #NO_APP

// MIPS-LABEL: _Z4testv:
// MIPS: ld	[[DATA4_PTR:\$[0-9]+]], %got_disp(DummyData4)([[GOT_PTR:\$[0-9]+]])
// MIPS-NEXT: sd	[[DATA4_PTR]], [[DATA4_ON_STACK:8\(\$sp\)]]
// MIPS-NEXT: ld	[[DATA4:\$[0-9]+]], [[DATA4_ON_STACK]]
// MIPS-NEXT: sd	[[GOT_PTR]], 0($sp)
// MIPS: #APP
// MIPS: lb	$zero, 0([[DATA4]])
// MIPS: #NO_APP
// MIPS: ld	[[DATA8_PTR:\$[0-9]+]], %got_disp(DummyData8)($3)
// MIPS-NEXT: sd	[[DATA8_PTR]], [[DATA8_ON_STACK:16\(\$sp\)]]
// MIPS-NEXT: ld	[[DATA8:\$[0-9]+]], [[DATA8_ON_STACK]]
// MIPS: #APP
// MIPS: lb	$zero, 0([[DATA8]])
// MIPS: #NO_APP
// MIPS: ld	[[DATA16_PTR:\$[0-9]+]], %got_disp(DummyData16)($3)
// MIPS-NEXT: sd	[[DATA16_PTR]], [[DATA16_ON_STACK:24\(\$sp\)]]
// MIPS-NEXT: ld	[[DATA16:\$[0-9]+]], [[DATA16_ON_STACK]]
// MIPS: #APP
// MIPS: lb	$zero, 0([[DATA16]])
// MIPS: #NO_APP
// MIPS: ld	[[DATA32_PTR:\$[0-9]+]], %got_disp(DummyData32)($3)
// MIPS-NEXT: sd	[[DATA32_PTR]], [[DATA32_ON_STACK:32\(\$sp\)]]
// MIPS-NEXT: ld	[[DATA32:\$[0-9]+]], [[DATA32_ON_STACK]]
// MIPS: #APP
// MIPS: lb	$zero, 0([[DATA32]])
// MIPS: #NO_APP
// MIPS: ld	[[DATA64_PTR:\$[0-9]+]], %got_disp(DummyData64)($3)
// MIPS-NEXT: sd	[[DATA64_PTR]], [[DATA64_ON_STACK:40\(\$sp\)]]
// MIPS-NEXT: ld	[[DATA64:\$[0-9]+]], [[DATA64_ON_STACK]]
// MIPS: #APP
// MIPS: lb	$zero, 0([[DATA64]])
// MIPS: #NO_APP



void* test_ptr(void* p) {
  DoNotOptimize(p);
  return p;
}
