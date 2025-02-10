; Check that  GVN does not attempt to read capability fields that it can't get the bits for
; This is https://github.com/CTSRD-CHERI/llvm-project/issues/385
; GVN was previously doing the following invalid transformation (Note the shift by 64 of the ptrtoint result)
;   %ai = alloca %suspicious_type, align 16, addrspace(200)
;   %tmp33 = bitcast %2 addrspace(200)* %ai to i8 addrspace(200)* addrspace(200)*
;   %tmp34 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %tmp33, align 16
;   %0 = ptrtoint i8 addrspace(200)* %tmp34 to i64 ; INCORRECT transformation (does not transfer all bits)
;   %1 = lshr i64 %0, 64   ; Shift right by 64 to get field #2
;   %2 = trunc i64 %1 to i32 ; truncate to drop the high bits
; It assumed it could get bits 32-63 by doing a ptrtoint, but on CHERI-MIPS ptrtoint returns bits 65-127

; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -S -aa-pipeline=basic-aa -passes=gvn -o - %s | FileCheck %s
; RUN: opt @PURECAP_HARDFLOAT_ARGS@ -S -aa-pipeline=basic-aa -passes=gvn -o - %s | llc @PURECAP_HARDFLOAT_ARGS@ -O0 -o - | FileCheck %s --check-prefix=ASM

; Check in the baseline (broken test now) to show the diff in the fixed commit

target datalayout = "@PURECAP_DATALAYOUT@"

%struct.addrinfo = type { i32, i32, i32, i32, i32, ptr addrspace(200), ptr addrspace(200), ptr addrspace(200) }


define i32 @first_i32_store_to_load_fwd(ptr addrspace(200) %arg) local_unnamed_addr addrspace(200) nounwind {
  %stackval = alloca %struct.addrinfo, align @CAP_BYTES@, addrspace(200)
  %field = getelementptr inbounds %struct.addrinfo, ptr addrspace(200) %stackval, i64 0, i32 0
  store ptr addrspace(200) %arg, ptr addrspace(200) %stackval, align @CAP_BYTES@
  %result = load i32, ptr addrspace(200) %field, align 4
  ret i32 %result
}

define i32 @second_i32_store_to_load_fwd(ptr addrspace(200) %arg) local_unnamed_addr addrspace(200) nounwind {
  %stackval = alloca %struct.addrinfo, align @CAP_BYTES@, addrspace(200)
  %field = getelementptr inbounds %struct.addrinfo, ptr addrspace(200) %stackval, i64 0, i32 1
  store ptr addrspace(200) %arg, ptr addrspace(200) %stackval, align @CAP_BYTES@
  %result = load i32, ptr addrspace(200) %field, align 4
  ret i32 %result
}

define i32 @third_i32_store_to_load_fwd(ptr addrspace(200) %arg) local_unnamed_addr addrspace(200) nounwind {
  %stackval = alloca %struct.addrinfo, align @CAP_BYTES@, addrspace(200)
  %field = getelementptr inbounds %struct.addrinfo, ptr addrspace(200) %stackval, i64 0, i32 2
  store ptr addrspace(200) %arg, ptr addrspace(200) %stackval, align @CAP_BYTES@
  %result = load i32, ptr addrspace(200) %field, align 4
  ret i32 %result
}

define i32 @fourth_i32_store_to_load_fwd(ptr addrspace(200) %arg) local_unnamed_addr addrspace(200) nounwind {
  %stackval = alloca %struct.addrinfo, align @CAP_BYTES@, addrspace(200)
  %field = getelementptr inbounds %struct.addrinfo, ptr addrspace(200) %stackval, i64 0, i32 3
  store ptr addrspace(200) %arg, ptr addrspace(200) %stackval, align @CAP_BYTES@
  %result = load i32, ptr addrspace(200) %field, align 4
  ret i32 %result
}
