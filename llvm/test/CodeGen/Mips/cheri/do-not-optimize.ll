; RUN: %cheri_purecap_llc -o - -O0 -verify-machineinstrs %s | FileCheck %s
; RUN: %cheri_purecap_llc -o - -O3 -verify-machineinstrs %s | FileCheck %s

define void @test_C_constraint() addrspace(200) #0 {
entry:
  call void asm sideeffect "# operand is '$0'", "C,~{memory},~{$1}"(i64 addrspace(200)* null) #1
  ; CHECK-LABEL: test_C_constraint:
  ; CHECK: # operand is '$c1'
  ret void
}

define void @test_r_constraint() addrspace(200) #0 {
entry:
  call void asm sideeffect "# operand is '$0'", "r,~{memory},~{$1}"(i64 addrspace(200)* null) #1
  ; CHECK-LABEL: test_r_constraint:
  ; CHECK: # operand is '$c1'
  ret void
}

define void @test_m_constraint() addrspace(200) #0 {
entry:
  call void asm sideeffect "# operand is '$0'", "m,~{memory},~{$1}"(i64 addrspace(200)* null) #1
  ; CHECK-LABEL: test_m_constraint:
  ; CHECK: # operand is '0($c11)'
  ret void
}

define void @test_rm_constraint() addrspace(200) #0 {
entry:
  ; CHECK-LABEL: test_rm_constraint:
  ; CHECK: # operand is '0($c11)'
  call void asm sideeffect "# operand is '$0'", "r|m,~{memory},~{$1}"(i64 addrspace(200)* null) #1
  ret void
}

define void @test_crm_constraint() addrspace(200) #0 {
entry:
  call void asm sideeffect "# operand is '$0'", "C|r|m,~{memory},~{$1}"(i64 addrspace(200)* null) #1
  ; CHECK-LABEL: test_crm_constraint:
  ; CHECK: # operand is '0($c11)'
  ret void
}

attributes #0 = { "use-soft-float"="true" }
attributes #1 = { nounwind }
