# REQUIRES: mips
# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: not ld.lld -shared --building-freebsd-rtld %t.o -o /dev/null 2>&1 | FileCheck %s
.text

.global __start
.ent __start
__start:
  lui $2, %captab_tlsgd_hi(foo)
  daddiu $3, $2, %captab_tlsgd_lo(foo)
.end __start

.global test
.ent test
test:
    clcbi $c1, %captab20(foo2)($c1)
.end test

# CHECK:      ld.lld: error: relocation R_MIPS_CHERI_CAPABILITY against <undefined> foo2 cannot be using when building FreeBSD RTLD
# CHECK-NEXT: >>> referenced by foo2@CAPTABLE
# CHECK-NEXT: >>> first used in function test
# CHECK-NEXT: >>> defined in  ({{.+}}building-rtld.s.tmp.o:(test))
# CHECK-EMPTY:
# CHECK-NEXT: ld.lld: error: relocation R_MIPS_TLS_DTPMOD64 against foo cannot be using when building FreeBSD RTLD
# CHECK-NEXT: >>> defined in {{.+}}building-rtld.s.tmp.o
# CHECK-NEXT: >>> referenced by <internal>:(.captable+0x10)
# CHECK-EMPTY:
# CHECK-NEXT: ld.lld: error: relocation R_MIPS_TLS_DTPREL64 against foo cannot be using when building FreeBSD RTLD
# CHECK-NEXT: >>> defined in {{.+}}building-rtld.s.tmp.o
# CHECK-NEXT: >>> referenced by <internal>:(.captable+0x18)
