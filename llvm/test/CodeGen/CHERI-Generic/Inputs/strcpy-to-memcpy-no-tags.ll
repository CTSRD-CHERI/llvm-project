; Check that we can inline the loads/stores generated when simplifiying
; string libcalls to memcpy() (since it should be marked as non-tag-preserving).
; CHERI-GENERIC-UTC: llc
; Note: unlike other tests we do want to test attributes in this one.
; CHERI-GENERIC-UTC: opt --function-signature
; RUN: opt < %s -passes=instcombine -S | FileCheck %s --check-prefix=CHECK-IR
; RUN: opt < %s -passes=instcombine -S | llc @PURECAP_HARDFLOAT_ARGS@ - -o - | FileCheck %s --check-prefix=CHECK-ASM
target datalayout = "@PURECAP_DATALAYOUT@"

@str = private unnamed_addr addrspace(200) constant [17 x i8] c"exactly 16 chars\00", align @CAP_RANGE_BYTES@

declare i8 addrspace(200)* @strcpy(i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)
declare i8 addrspace(200)* @stpcpy(i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)
declare i8 addrspace(200)* @strcat(i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)
declare i8 addrspace(200)* @strncpy(i8 addrspace(200)*, i8 addrspace(200)*, i64) addrspace(200)
declare i8 addrspace(200)* @stpncpy(i8 addrspace(200)*, i8 addrspace(200)*, i64) addrspace(200)

define void @test_strcpy_to_memcpy(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %dst) addrspace(200) nounwind {
entry:
  %call = call i8 addrspace(200)* @strcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(200)* @str, i64 0, i64 0))
  ret void
}

define void @test_stpcpy_to_memcpy(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %dst) addrspace(200) nounwind {
entry:
  %call = call i8 addrspace(200)* @stpcpy(i8 addrspace(200)* %dst, i8 addrspace(200)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(200)* @str, i64 0, i64 0))
  ret void
}

define void @test_strcat_to_memcpy(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %dst) addrspace(200) nounwind {
entry:
  %call = call i8 addrspace(200)* @strcat(i8 addrspace(200)* %dst, i8 addrspace(200)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(200)* @str, i64 0, i64 0))
  ret void
}


define void @test_strncpy_to_memcpy(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %dst) addrspace(200) nounwind {
entry:
  %call = call i8 addrspace(200)* @strncpy(i8 addrspace(200)* %dst, i8 addrspace(200)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(200)* @str, i64 0, i64 0), i64 17)
  ret void
}

; Note: stpncpy is not handled by SimplifyLibcalls yet, so this should not be changed.
define void @test_stpncpy_to_memcpy(i8 addrspace(200)* align @CAP_RANGE_BYTES@ %dst) addrspace(200) nounwind {
entry:
  %call = call i8 addrspace(200)* @stpncpy(i8 addrspace(200)* %dst, i8 addrspace(200)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(200)* @str, i64 0, i64 0), i64 17)
  ret void
}

; UTC_ARGS: --disable
; CHECK-IR: attributes #[[ATTR0]] = { nounwind }
; The no_preserve_cheri_tags should be attribute 3/4 in all cases
; CHECK-IR: attributes #[[ATTR3]] = { no_preserve_cheri_tags nounwind }
; CHECK-IR: attributes #[[ATTR4]] = { no_preserve_cheri_tags }
