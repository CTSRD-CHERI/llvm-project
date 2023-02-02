; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32-unknown-unknown -target-abi cheriot  %s -mattr=+xcheri,+cap-mode -o - | FileCheck %s
; ModuleID = '/tmp/timer.ll'
source_filename = "/tmp/timer-8b64ae.cpp"
target datalayout = "e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128-A200-P200-G200"
target triple = "riscv32-unknown-unknown"

@_ZN5sched5Timer10pmtimercmpE = external addrspace(200) global i32 addrspace(200)*, align 8

define void @_ZN5sched5Timer23timer_interrupt_setnextEv() addrspace(200) align 2 {
entry:
  %0 = load i32 addrspace(200)*, i32 addrspace(200)* addrspace(200)* @_ZN5sched5Timer10pmtimercmpE, align 8
  %add.ptr1 = getelementptr inbounds i32, i32 addrspace(200)* %0, i32 1
  store i32 addrspace(200)* %add.ptr1, i32 addrspace(200)* addrspace(200)* undef, align 8
  ; CHECK: clc
  ret void
}
