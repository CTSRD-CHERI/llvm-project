; Genereated from the following source code:
; GENERATE: %cheri_purecap_cc1 -fuse-init-array -mllvm -cheri-cap-table-abi=pcrel -mllvm -mxcaptable=false -nostdinc++ -std=c++11 -fdeprecated-macro -ftls-model=local-exec -fcxx-exceptions -fexceptions -fcolor-diagnostics -vectorize-loops -vectorize-slp -o %s.ll -O0  -emit-llvm %s
;typedef struct { char a[128]; } b;
;class c {
;  b e;
;};
;template <class> class d {
;  c f(c) { return g(); }
;  c g();
;};
;template class d<char>;


; RUN: %cheri_purecap_llc -cheri-cap-table-abi=pcrel -o - -print-before-all %s
; Regression test for assertion: (Node2Index[SU.NodeNum] > Node2Index[PD.getSUnit()->NodeNum] && "Wrong topological sorting"), function InitDAGTopologicalSorting
target datalayout = "E-m:e-pf200:128:128-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200"

; unreduced IR:
;%class.c = type { %struct.b }
;%struct.b = type { [128 x i8] }
;%class.d = type { i8 }
;
;$_ZN1dIcE1fE1c = comdat any
;
;; Function Attrs: noinline optnone
;define weak_odr void @_ZN1dIcE1fE1c(%class.c addrspace(200)* noalias sret %agg.result, %class.d addrspace(200)* %this, i64 inreg %.coerce0, i64 inreg %.coerce1, i64 inreg %.coerce2, i64 inreg %.coerce3, i64 inreg %.coerce4, i64 inreg %.coerce5, i64 inreg %.coerce6, i64 inreg %.coerce7, i64 inreg %.coerce8, i64 inreg %.coerce9, i64 inreg %.coerce10, i64 inreg %.coerce11, i64 inreg %.coerce12, i64 inreg %.coerce13, i64 inreg %.coerce14, i64 inreg %.coerce15) #0 comdat align 2 {
;entry:
;  %0 = alloca %class.c, align 1, addrspace(200)
;  %this.addr = alloca %class.d addrspace(200)*, align 16, addrspace(200)
;  %1 = bitcast %class.c addrspace(200)* %0 to { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)*
;  %2 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 0
;  store i64 %.coerce0, i64 addrspace(200)* %2, align 1
;  %3 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 1
;  store i64 %.coerce1, i64 addrspace(200)* %3, align 1
;  %4 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 2
;  store i64 %.coerce2, i64 addrspace(200)* %4, align 1
;  %5 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 3
;  store i64 %.coerce3, i64 addrspace(200)* %5, align 1
;  %6 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 4
;  store i64 %.coerce4, i64 addrspace(200)* %6, align 1
;  %7 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 5
;  store i64 %.coerce5, i64 addrspace(200)* %7, align 1
;  %8 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 6
;  store i64 %.coerce6, i64 addrspace(200)* %8, align 1
;  %9 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 7
;  store i64 %.coerce7, i64 addrspace(200)* %9, align 1
;  %10 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 8
;  store i64 %.coerce8, i64 addrspace(200)* %10, align 1
;  %11 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 9
;  store i64 %.coerce9, i64 addrspace(200)* %11, align 1
;  %12 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 10
;  store i64 %.coerce10, i64 addrspace(200)* %12, align 1
;  %13 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 11
;  store i64 %.coerce11, i64 addrspace(200)* %13, align 1
;  %14 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 12
;  store i64 %.coerce12, i64 addrspace(200)* %14, align 1
;  %15 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 13
;  store i64 %.coerce13, i64 addrspace(200)* %15, align 1
;  %16 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 14
;  store i64 %.coerce14, i64 addrspace(200)* %16, align 1
;  %17 = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 } addrspace(200)* %1, i32 0, i32 15
;  store i64 %.coerce15, i64 addrspace(200)* %17, align 1
;  store %class.d addrspace(200)* %this, %class.d addrspace(200)* addrspace(200)* %this.addr, align 16
;  %this1 = load %class.d addrspace(200)*, %class.d addrspace(200)* addrspace(200)* %this.addr, align 16
;  call void @_ZN1dIcE1gEv(%class.c addrspace(200)* sret %agg.result, %class.d addrspace(200)* %this1)
;  ret void
;}
;
;declare void @_ZN1dIcE1gEv(%class.c addrspace(200)* sret, %class.d addrspace(200)*)


; Reduced IR:
%class.c = type { %struct.b }
%struct.b = type { [128 x i8] }
%class.d = type { i8 }

$_ZN1dIcE1fE1c = comdat any

define weak_odr void @_ZN1dIcE1fE1c(%class.c addrspace(200)* noalias sret %agg.result, %class.d addrspace(200)* %this, i64 inreg %.coerce0, i64 inreg %.coerce1, i64 inreg %.coerce2, i64 inreg %.coerce3, i64 inreg %.coerce4, i64 inreg %.coerce5, i64 inreg %.coerce6, i64 inreg %.coerce7, i64 inreg %.coerce8, i64 inreg %.coerce9, i64 inreg %.coerce10, i64 inreg %.coerce11, i64 inreg %.coerce12, i64 inreg %.coerce13, i64 inreg %.coerce14, i64 inreg %.coerce15) comdat align 2 {
entry:
  ret void
}

declare void @_ZN1dIcE1gEv(%class.c addrspace(200)* sret, %class.d addrspace(200)*)
