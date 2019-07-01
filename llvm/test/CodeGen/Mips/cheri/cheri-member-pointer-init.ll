; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel %s -o - | %cheri_FileCheck %s
; RUsN: %cheri256_purecap_llc %s -legacy-cheri-cap-relocs -o - | FileCheck %s -check-prefix REALLY_OLD_STUFF
; ModuleID = 'cheri-member-pointer-init.cpp'
source_filename = "cheri-member-pointer-init.cpp"
target datalayout = "P200"
target triple = "cheri-unknown-freebsd"

; CHECK-LABEL: global_nonvirt_ptr:
; CHECK-NEXT:  .chericap       _ZN1A7nonvirtEv
; CHECK-NEXT:  .8byte  0
; CHECK-NEXT:  .space  [[#CAP_SIZE - 8]]
; CHECK-NEXT:  .size   global_nonvirt_ptr, [[#CAP_SIZE * 2]]

; CHECK-LABEL: global_virt_ptr:
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .8byte  1
; CHECK-NEXT:  .space  [[#CAP_SIZE - 8]]
; CHECK-NEXT:  .size   global_virt_ptr, [[#CAP_SIZE * 2]]

; CHECK-LABEL: global_fn_ptr:
; CHECK-NEXT:  .chericap       _Z9global_fnv
; CHECK-NEXT:  .size   global_fn_ptr, [[#CAP_SIZE]]

; CHECK:       .type   _ZTV1A,@object          # @_ZTV1A
; CHECK-NEXT:  .section .data.rel.ro,"aw",@progbits
; CHECK-NEXT:  .weak   _ZTV1A
; CHECK-NEXT:  .p2align        5
; CHECK-LABEL: _ZTV1A:
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .chericap 0
; CHECK-NEXT:  .chericap       _ZN1A4virtEv
; CHECK-NEXT:  .chericap       _ZN1A5virt2Ev
; CHECK-NEXT:  .size   _ZTV1A, [[#CAP_SIZE * 4]]

; LEGACY_ABI-LABEL: .size.global_nonvirt_ptr:
; LEGACY_ABI-LABEL: .size.global_fn_ptr:
; LEGACY_ABI-LABEL: .size._ZTV1A:


; REALLY_OLD_STUFF: 	.section	__cap_relocs,"a",@progbits
; REALLY_OLD_STUFF-NEXT: 	.8byte	.L
; REALLY_OLD_STUFF-NEXT: 	.8byte	_ZN1A7nonvirtEv
; REALLY_OLD_STUFF-NEXT: 	.8byte	0
; REALLY_OLD_STUFF-NEXT: 	.space	16
; REALLY_OLD_STUFF-NEXT: 	.8byte	.L
; REALLY_OLD_STUFF-NEXT: 	.8byte	_Z9global_fnv
; REALLY_OLD_STUFF-NEXT: 	.8byte	0
; REALLY_OLD_STUFF-NEXT: 	.space	16
; REALLY_OLD_STUFF-NEXT: 	.8byte	.L
; REALLY_OLD_STUFF-NEXT: 	.8byte	_ZN1A4virtEv
; REALLY_OLD_STUFF-NEXT: 	.8byte	0
; REALLY_OLD_STUFF-NEXT: 	.space	16
; REALLY_OLD_STUFF-NEXT: 	.8byte	.L
; REALLY_OLD_STUFF-NEXT: 	.8byte	_ZN1A5virt2Ev
; REALLY_OLD_STUFF-NEXT: 	.8byte	0
; REALLY_OLD_STUFF-NEXT: 	.space	16

%class.A = type { i32 (...) addrspace(200)* addrspace(200)* }


@global_nonvirt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A7nonvirtEv to i8 addrspace(200)*), i64 0 }, align 32
@global_virt_ptr = addrspace(200) global { i8 addrspace(200)*, i64 } { i8 addrspace(200)* null, i64 1 }, align 32
@global_fn_ptr = addrspace(200) global i32 () addrspace(200)* @_Z9global_fnv, align 32
@_ZTV1A = linkonce_odr unnamed_addr addrspace(200) constant { [4 x i8 addrspace(200)*] } { [4 x i8 addrspace(200)*] [i8 addrspace(200)* null, i8 addrspace(200)* null,
     i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A4virtEv to i8 addrspace(200)*),
     i8 addrspace(200)* bitcast (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A5virt2Ev to i8 addrspace(200)*)] }, align 32

; Function Attrs: noinline nounwind
define i32 @_Z9global_fnv() addrspace(200) #0 {
entry:
  ret i32 5
}

; Function Attrs: noinline nounwind
define linkonce_odr i32 @_ZN1A7nonvirtEv(%class.A addrspace(200)* %this) addrspace(200) #0 align 2 {
entry:
  %this.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %this, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %this1 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  ret i32 1
}

; Function Attrs: noinline nounwind
define i32 @_Z12call_nonvirtP1A(%class.A addrspace(200)* %a) addrspace(200) #0 {
entry:
  %a.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %a, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %0 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %1 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* @global_nonvirt_ptr, align 32
  %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  %memptr.adj.shifted = ashr i64 %memptr.adj, 1
  %this.not.adjusted = bitcast %class.A addrspace(200)* %0 to i8 addrspace(200)*
  %memptr.vtable.addr = getelementptr inbounds i8, i8 addrspace(200)* %this.not.adjusted, i64 %memptr.adj.shifted
  %this.adjusted = bitcast i8 addrspace(200)* %memptr.vtable.addr to %class.A addrspace(200)*
  %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %1, 0
  %2 = and i64 %memptr.adj, 1
  %memptr.isvirtual = icmp ne i64 %2, 0
  br i1 %memptr.isvirtual, label %memptr.virtual, label %memptr.nonvirtual

memptr.virtual:                                   ; preds = %entry
  %3 = bitcast i8 addrspace(200)* %memptr.vtable.addr to i8 addrspace(200)* addrspace(200)*
  %vtable = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %3, align 32
  %memptr.vtable.offset = ptrtoint i8 addrspace(200)* %memptr.ptr to i64
  %4 = getelementptr i8, i8 addrspace(200)* %vtable, i64 %memptr.vtable.offset
  %5 = bitcast i8 addrspace(200)* %4 to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  %memptr.virtualfn = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* %5, align 32
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %entry
  %memptr.nonvirtualfn = bitcast i8 addrspace(200)* %memptr.ptr to i32 (%class.A addrspace(200)*) addrspace(200)*
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %6 = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  %call = call i32 %6(%class.A addrspace(200)* %this.adjusted)
  ret i32 %call
}

; Function Attrs: noinline nounwind
define i32 @_Z9call_virtP1A(%class.A addrspace(200)* %a) addrspace(200) #0 {
entry:
  %a.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %a, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %0 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %1 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* @global_nonvirt_ptr, align 32
  %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  %memptr.adj.shifted = ashr i64 %memptr.adj, 1
  %this.not.adjusted = bitcast %class.A addrspace(200)* %0 to i8 addrspace(200)*
  %memptr.vtable.addr = getelementptr inbounds i8, i8 addrspace(200)* %this.not.adjusted, i64 %memptr.adj.shifted
  %this.adjusted = bitcast i8 addrspace(200)* %memptr.vtable.addr to %class.A addrspace(200)*
  %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %1, 0
  %2 = and i64 %memptr.adj, 1
  %memptr.isvirtual = icmp ne i64 %2, 0
  br i1 %memptr.isvirtual, label %memptr.virtual, label %memptr.nonvirtual

memptr.virtual:                                   ; preds = %entry
  %3 = bitcast i8 addrspace(200)* %memptr.vtable.addr to i8 addrspace(200)* addrspace(200)*
  %vtable = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %3, align 32
  %memptr.vtable.offset = ptrtoint i8 addrspace(200)* %memptr.ptr to i64
  %4 = getelementptr i8, i8 addrspace(200)* %vtable, i64 %memptr.vtable.offset
  %5 = bitcast i8 addrspace(200)* %4 to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  %memptr.virtualfn = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* %5, align 32
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %entry
  %memptr.nonvirtualfn = bitcast i8 addrspace(200)* %memptr.ptr to i32 (%class.A addrspace(200)*) addrspace(200)*
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %6 = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  %call = call i32 %6(%class.A addrspace(200)* %this.adjusted)
  ret i32 %call
}

; Function Attrs: noinline nounwind
define i32 @_Z18call_local_nonvirtP1A(%class.A addrspace(200)* %a) addrspace(200) #0 {
entry:
  %a.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  %local_nonvirt = alloca { i8 addrspace(200)*, i64 }, align 32, addrspace(200)
  %memptr_tmp = alloca { i8 addrspace(200)*, i64 }, align 32, addrspace(200)
  store %class.A addrspace(200)* %a, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %0 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %1 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %0, i64 ptrtoint (i32 (%class.A addrspace(200)*) addrspace(200)* @_ZN1A8nonvirt2Ev to i64))
  %2 = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %memptr_tmp, i32 0, i32 0
  store i8 addrspace(200)* %1, i8 addrspace(200)* addrspace(200)* %2, align 32
  %3 = getelementptr inbounds { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %memptr_tmp, i32 0, i32 1
  store i64 0, i64 addrspace(200)* %3, align 32
  %4 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %memptr_tmp, align 32
  store { i8 addrspace(200)*, i64 } %4, { i8 addrspace(200)*, i64 } addrspace(200)* %local_nonvirt, align 32
  %5 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %6 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %local_nonvirt, align 32
  %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %6, 1
  %memptr.adj.shifted = ashr i64 %memptr.adj, 1
  %this.not.adjusted = bitcast %class.A addrspace(200)* %5 to i8 addrspace(200)*
  %memptr.vtable.addr = getelementptr inbounds i8, i8 addrspace(200)* %this.not.adjusted, i64 %memptr.adj.shifted
  %this.adjusted = bitcast i8 addrspace(200)* %memptr.vtable.addr to %class.A addrspace(200)*
  %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %6, 0
  %7 = and i64 %memptr.adj, 1
  %memptr.isvirtual = icmp ne i64 %7, 0
  br i1 %memptr.isvirtual, label %memptr.virtual, label %memptr.nonvirtual

memptr.virtual:                                   ; preds = %entry
  %8 = bitcast i8 addrspace(200)* %memptr.vtable.addr to i8 addrspace(200)* addrspace(200)*
  %vtable = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %8, align 32
  %memptr.vtable.offset = ptrtoint i8 addrspace(200)* %memptr.ptr to i64
  %9 = getelementptr i8, i8 addrspace(200)* %vtable, i64 %memptr.vtable.offset
  %10 = bitcast i8 addrspace(200)* %9 to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  %memptr.virtualfn = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* %10, align 32
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %entry
  %memptr.nonvirtualfn = bitcast i8 addrspace(200)* %memptr.ptr to i32 (%class.A addrspace(200)*) addrspace(200)*
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %11 = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  %call = call i32 %11(%class.A addrspace(200)* %this.adjusted)
  ret i32 %call
}

; Function Attrs: noinline nounwind
define linkonce_odr i32 @_ZN1A8nonvirt2Ev(%class.A addrspace(200)* %this) addrspace(200) #0 align 2 {
entry:
  %this.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %this, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %this1 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  ret i32 2
}

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.pcc.get() addrspace(200) #1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)*, i64) addrspace(200) #1

; Function Attrs: noinline nounwind
define i32 @_Z15call_local_virtP1A(%class.A addrspace(200)* %a) addrspace(200) #0 {
entry:
  %a.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  %local_virt = alloca { i8 addrspace(200)*, i64 }, align 32, addrspace(200)
  store %class.A addrspace(200)* %a, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  store { i8 addrspace(200)*, i64 } { i8 addrspace(200)* inttoptr (i64 32 to i8 addrspace(200)*), i64 1 }, { i8 addrspace(200)*, i64 } addrspace(200)* %local_virt, align 32
  %0 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %1 = load { i8 addrspace(200)*, i64 }, { i8 addrspace(200)*, i64 } addrspace(200)* %local_virt, align 32
  %memptr.adj = extractvalue { i8 addrspace(200)*, i64 } %1, 1
  %memptr.adj.shifted = ashr i64 %memptr.adj, 1
  %this.not.adjusted = bitcast %class.A addrspace(200)* %0 to i8 addrspace(200)*
  %memptr.vtable.addr = getelementptr inbounds i8, i8 addrspace(200)* %this.not.adjusted, i64 %memptr.adj.shifted
  %this.adjusted = bitcast i8 addrspace(200)* %memptr.vtable.addr to %class.A addrspace(200)*
  %memptr.ptr = extractvalue { i8 addrspace(200)*, i64 } %1, 0
  %2 = and i64 %memptr.adj, 1
  %memptr.isvirtual = icmp ne i64 %2, 0
  br i1 %memptr.isvirtual, label %memptr.virtual, label %memptr.nonvirtual

memptr.virtual:                                   ; preds = %entry
  %3 = bitcast i8 addrspace(200)* %memptr.vtable.addr to i8 addrspace(200)* addrspace(200)*
  %vtable = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %3, align 32
  %memptr.vtable.offset = ptrtoint i8 addrspace(200)* %memptr.ptr to i64
  %4 = getelementptr i8, i8 addrspace(200)* %vtable, i64 %memptr.vtable.offset
  %5 = bitcast i8 addrspace(200)* %4 to i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)*
  %memptr.virtualfn = load i32 (%class.A addrspace(200)*) addrspace(200)*, i32 (%class.A addrspace(200)*) addrspace(200)* addrspace(200)* %5, align 32
  br label %memptr.end

memptr.nonvirtual:                                ; preds = %entry
  %memptr.nonvirtualfn = bitcast i8 addrspace(200)* %memptr.ptr to i32 (%class.A addrspace(200)*) addrspace(200)*
  br label %memptr.end

memptr.end:                                       ; preds = %memptr.nonvirtual, %memptr.virtual
  %6 = phi i32 (%class.A addrspace(200)*) addrspace(200)* [ %memptr.virtualfn, %memptr.virtual ], [ %memptr.nonvirtualfn, %memptr.nonvirtual ]
  %call = call i32 %6(%class.A addrspace(200)* %this.adjusted)
  ret i32 %call
}

; Function Attrs: noinline nounwind
define i32 @_Z17call_local_fn_ptrP1A(%class.A addrspace(200)* %a) addrspace(200) #0 {
entry:
  %a.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  %local_fn_ptr = alloca i32 () addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %a, %class.A addrspace(200)* addrspace(200)* %a.addr, align 32
  %0 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %1 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %0, i64 ptrtoint (i32 () addrspace(200)* @_Z9global_fnv to i64))
  %2 = bitcast i8 addrspace(200)* %1 to i32 () addrspace(200)*
  store i32 () addrspace(200)* %2, i32 () addrspace(200)* addrspace(200)* %local_fn_ptr, align 32
  %3 = load i32 () addrspace(200)*, i32 () addrspace(200)* addrspace(200)* %local_fn_ptr, align 32
  %call = call i32 %3()
  ret i32 %call
}

; Function Attrs: noinline norecurse nounwind
define i32 @main() addrspace(200) #2 {
entry:
  %a = alloca %class.A, align 32, addrspace(200)
  call void @_ZN1AC2Ev(%class.A addrspace(200)* %a) #3
  %0 = load i32 () addrspace(200)*, i32 () addrspace(200)* addrspace(200)* @global_fn_ptr, align 32
  %call = call i32 %0()
  %1 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %2 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %1, i64 ptrtoint (i32 (%class.A addrspace(200)*) addrspace(200)* @_Z9call_virtP1A to i64))
  %3 = bitcast i8 addrspace(200)* %2 to i32 (%class.A addrspace(200)*) addrspace(200)*
  %call1 = call i32 %3(%class.A addrspace(200)* %a)
  %4 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %5 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %4, i64 ptrtoint (i32 (%class.A addrspace(200)*) addrspace(200)* @_Z12call_nonvirtP1A to i64))
  %6 = bitcast i8 addrspace(200)* %5 to i32 (%class.A addrspace(200)*) addrspace(200)*
  %call2 = call i32 %6(%class.A addrspace(200)* %a)
  %7 = call i8 addrspace(200)* @llvm.cheri.pcc.get()
  %8 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set.i64(i8 addrspace(200)* %7, i64 ptrtoint (i32 (%class.A addrspace(200)*) addrspace(200)* @_Z9call_virtP1A to i64))
  %9 = bitcast i8 addrspace(200)* %8 to i32 (%class.A addrspace(200)*) addrspace(200)*
  %call3 = call i32 %9(%class.A addrspace(200)* %a)
  ret i32 0
}

; Function Attrs: noinline nounwind
define linkonce_odr void @_ZN1AC2Ev(%class.A addrspace(200)* %this) addrspace(200) #0 align 2 {
entry:
  %this.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %this, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %this1 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %0 = bitcast %class.A addrspace(200)* %this1 to i32 (...) addrspace(200)* addrspace(200)* addrspace(200)*
  store i32 (...) addrspace(200)* addrspace(200)* bitcast (i8 addrspace(200)* addrspace(200)* getelementptr inbounds ({ [4 x i8 addrspace(200)*] }, { [4 x i8 addrspace(200)*] } addrspace(200)* @_ZTV1A, i32 0, inrange i32 0, i32 2) to i32 (...) addrspace(200)* addrspace(200)*), i32 (...) addrspace(200)* addrspace(200)* addrspace(200)* %0, align 32
  ret void
}

; Function Attrs: noinline nounwind
define linkonce_odr i32 @_ZN1A4virtEv(%class.A addrspace(200)* %this) addrspace(200) #0 align 2 {
entry:
  %this.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %this, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %this1 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  ret i32 3
}

; Function Attrs: noinline nounwind
define linkonce_odr i32 @_ZN1A5virt2Ev(%class.A addrspace(200)* %this) addrspace(200) #0 align 2 {
entry:
  %this.addr = alloca %class.A addrspace(200)*, align 32, addrspace(200)
  store %class.A addrspace(200)* %this, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  %this1 = load %class.A addrspace(200)*, %class.A addrspace(200)* addrspace(200)* %this.addr, align 32
  ret i32 4
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="cheri" "target-features"="+cheri" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 5.0.0 "}
