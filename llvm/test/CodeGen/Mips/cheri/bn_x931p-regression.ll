; After the C13 == NULL optimization openssl/crypto/bn/bn_x931p.c
; TODO: fix replace this with a check for sensible assembler output
; RUN: not %cheri_purecap_llc -verify-machineinstrs -o - %s
; RUN: not %cheri_purecap_llc -verify-machineinstrs -o - %s 2>&1 | FileCheck %s
; CHECK:  Bad machine code: Using an undefined physical register

; Calling a function with 9+ capability parameters would previously cause verification errors
define i32 @a(i64 addrspace(200) *, i32 addrspace(200) *, i32 addrspace(200) *,
              i64 addrspace(200) *, i64 addrspace(200) *, i64 addrspace(200) *,
              i32 addrspace(200) *, i32 addrspace(200) *, i32 addrspace(200) *) {
  %ret = call i32 @b()
  ret i32 %ret
}

declare i32 @b()
