; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel -o - -O0 %s -debug-only=isel
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=legacy -o - -O0 %s

; This needs the kernel to set the capability hwr so that we can load it
; XFAIL: *

@__tls_guard = external thread_local addrspace(200) global i1 #0

define void @store() {
  store i1 true, i1 addrspace(200)* @__tls_guard
  ret void
}

define void @load() {
  %arg = load i1, i1 addrspace(200)* @__tls_guard
  ret void
}
