; RUN: rm -f %t.csv
; RUN: %cheri_purecap_llc -o /dev/null -collect-csetbounds-stats=csv -collect-csetbounds-output=%t.csv -cheri-cap-table-abi=pcrel %s
; RUN: FileCheck -input-file=%t.csv %s
; This used to crash due to use of an unsized global

declare void @a(i64 addrspace(200)*, i32 (i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)*) addrspace(200)

@globali64 = local_unnamed_addr addrspace(200) global i64 123, align 8

define void @b() addrspace(200) {
  call void @a(i64 addrspace(200)* @globali64, i32(i8 addrspace(200) *, i8 addrspace(200) *) addrspace(200) * @atcomp)
  unreachable
}

declare i32 @atcomp(i8 addrspace(200) *, i8 addrspace(200) *) addrspace(200)

; CHECK-LABEL:  alignment_bits,size,kind,source_loc,compiler_pass,details
; CHECK-NEXT:  0,<unknown>,g,"<somewhere in b>","MipsTargetLowering::lowerGlobalAddress","load of global atcomp (alloc size=-1)"
; CHECK-NEXT:  3,8,g,"<somewhere in b>","MipsTargetLowering::lowerGlobalAddress","load of global globali64 (alloc size=8)"

