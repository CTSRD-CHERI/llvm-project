; RUN: rm -f %t.csv
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -O0 -o /dev/null \
; RUN:    -collect-csetbounds-output=%t.csv -collect-csetbounds-stats=csv
; RUN: %cheri_FileCheck %s -input-file=%t.csv -check-prefix CSV

declare i8 addrspace(200)* @do_alloc(i32) #1
declare i8 addrspace(200)* @do_alloc_callsite_annotated(i32, i32)
@alloc_fn_ptr = addrspace(200) global i8 addrspace(200)* (i32, i32, i32) addrspace(200)* null, align 32

define i8 addrspace(200)* @test_direct_call_1() {
entry:
  %result = call i8 addrspace(200)* @do_alloc(i32 100)
  ret i8 addrspace(200)* %result
}

; Check that an annotated callsite also works if the function doesn't have it
define i8 addrspace(200)* @test_direct_call_2() {
entry:
  %result = call i8 addrspace(200)* @do_alloc_callsite_annotated(i32 0, i32 200) #2
  ret i8 addrspace(200)* %result
}

; Same for function pointers (see also https://reviews.llvm.org/D55212
; and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88372)
define i8 addrspace(200)* @test_indirect_call() {
entry:
  %0 = load i8 addrspace(200)* (i32, i32, i32) addrspace(200)*, i8 addrspace(200)* (i32, i32, i32) addrspace(200)* addrspace(200)* @alloc_fn_ptr, align 32
  %result = call i8 addrspace(200)* %0(i32 1, i32 8, i32 4) #3
  ret i8 addrspace(200)* %result
}

attributes #1 = { allocsize(0) }
attributes #2 = { allocsize(1) }
attributes #3 = { allocsize(1,2) }

; CSV-LABEL: alignment_bits,size,kind,source_loc,compiler_pass,details
; CSV-NEXT: 0,100,h,"<somewhere in test_direct_call_1>","function with alloc_size","call to do_alloc"
; CSV-NEXT: 0,200,h,"<somewhere in test_direct_call_2>","function with alloc_size","call to do_alloc_callsite_annotated"
; CSV-NEXT: 5,[[#CAP_SIZE]],g,"<somewhere in test_indirect_call>","MipsTargetLowering::lowerGlobalAddress","load of global alloc_fn_ptr (alloc size=[[#CAP_SIZE]])"
; CSV-NEXT: 0,32,h,"<somewhere in test_indirect_call>","function with alloc_size","call to function pointer"
