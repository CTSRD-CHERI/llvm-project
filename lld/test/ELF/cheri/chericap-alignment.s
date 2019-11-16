
# RUN: %cheri256_purecap_llvm-mc %s -filetype=obj -defsym=FIRST=1 -o %t1.o
# RUN: %cheri256_purecap_llvm-mc %s -filetype=obj -defsym=FIRST=0 -o %t2.o
# RUN: ld.lld %t1.o %t2.o -shared -o %t.so
# RUN: llvm-objdump --cap-relocs -s -t %t.so | FileCheck %s

.if FIRST

.data
.quad 0x1234

.else

.global foo
.global cap
.data
cap:
    .chericap foo

.endif

# CHECK-LABEL: SYMBOL TABLE:
# CHECK: 0000000000020400         .data		 00000000 cap
#                   ^----- Ensure that this is aligned to 0x20
# CHECK: 0000000000000000         *UND*		 00000000 foo

# CHECK-LABEL: Contents of section .data:
# CHECK-NEXT:  203e0 00000000 00001234 00000000 00000000
# CHECK-NEXT:  203f0 00000000 00000000 00000000 00000000
# CHECK-NEXT:  20400 00000000 00000000 cacacaca cacacaca
# CHECK-NEXT:  20410 cacacaca cacacaca cacacaca cacacaca
