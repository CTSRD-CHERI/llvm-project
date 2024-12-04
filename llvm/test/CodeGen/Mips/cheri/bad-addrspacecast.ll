; RUN: not --crash %cheri_purecap_llc -verify-machineinstrs -o /dev/null -O3 -thread-model posix -filetype=obj %s
; REQUIRES: asserts
; TODO: report a sensible error message fr invalid IR

define i64 @snork(ptr addrspace(200) %arg) addrspace(200) {
bb:
  %tmp = addrspacecast ptr addrspace(200) %arg to ptr
  %ret = ptrtoint ptr %tmp to i64
  ret i64 %ret
}
