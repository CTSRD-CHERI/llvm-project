; RUN: %cheri_purecap_llc -float-abi soft %s -o - -O3 "-thread-model" "posix" -filetype=obj | llvm-objdump -d - | FileCheck %s
; This code used to generate a csw of $c28 which is inacessible:
; https://github.com/CTSRD-CHERI/llvm/issues/238
; CHECK-NOT: $c28
; ModuleID = '<stdin>'
source_filename = "libxo3.ll"
target datalayout = "Emepf200256256i8832i161632i6464n3264S128-A200"

%struct.a.1 = type { i32, i32, %struct.ab.0 }
%struct.ab.0 = type {}
%struct.b.2 = type {}

@ad = external addrspace(200) global i32
@c = external addrspace(200) global i32
@ae = external addrspace(200) global i32
@ag = external addrspace(200) global i32
@ah = external addrspace(200) global i32
@ai = external addrspace(200) global i32
@br = external addrspace(200) global i32
@aj = external addrspace(200) global i8 addrspace(200)*
@d = external addrspace(200) global %struct.a.1

; Function Attrs: nounwind readnone
declare i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)*, i64) #0

declare void @llvm.e(i8*, i8*)

define i32 @ak(i32 %r) #1 {
"0":
  %al.b = alloca i8, addrspace(200)
  %s.addr.b.b = alloca i8 addrspace(200)*, addrspace(200)
  %am.b.b = alloca i8 addrspace(200)*, addrspace(200)
  %an.b = alloca i8, addrspace(200)
  %ao.b = alloca i8, addrspace(200)
  %ap.b = alloca i8 addrspace(200)*, align 1, addrspace(200)
  %aq.b = alloca i32, align 1, addrspace(200)
  %ar.b = alloca i8, align 1, addrspace(200)
  %as = alloca i8, align 1, addrspace(200)
  br label %h.i.at

h.i.at:                                           ; preds = %"0"
  br i1 undef, label %if.au, label %if.k

if.k:                                             ; preds = %h.i.at
  br label %if.au

if.au:                                            ; preds = %if.k, %h.i.at
  %av.0 = phi i32 [ 0, %if.k ], [ 1, %h.i.at ]
  br i1 undef, label %for.l, label %for.aw.ax.ay

for.aw.ax.ay:                                     ; preds = %if.au
  %0 = addrspacecast i8 addrspace(200)* undef to i8*
  %1 = bitcast i32 addrspace(200)* %aq.b to i8 addrspace(200)*
  %m = icmp eq i32 %av.0, 0
  br label %for.aw

for.aw:                                           ; preds = %for.bb, %for.aw.ax.ay
  %az.027 = phi i32 addrspace(200)* [ null, %for.aw.ax.ay ], [ %ba.n, %for.bb ]
  %bc.026 = phi i32 [ 0, %for.aw.ax.ay ], [ 1, %for.bb ]
  %t = icmp eq i32 addrspace(200)* %az.027, null
  br i1 %t, label %if.o, label %bd.be

bd.be:                                            ; preds = %for.aw
  br i1 %m, label %for.bb, label %bf.p

bf.p:                                             ; preds = %bd.be
  unreachable

if.o:                                             ; preds = %for.aw
  br label %for.aw.b

for.aw.b:                                         ; preds = %for.aw.b, %if.o
  %r.addr.03.b = phi i8 addrspace(200)* [ %ba.n.b, %for.aw.b ], [ %as, %if.o ]
  %ba.n.b = getelementptr i8, i8 addrspace(200)* %r.addr.03.b, i64 1
  %bg.b = icmp ult i8 addrspace(200)* %ba.n.b, %ar.b
  br i1 %bg.b, label %for.aw.b, label %for.bh.for.q.b

for.bh.for.q.b:                                   ; preds = %for.aw.b
  call void @g(i8 addrspace(200)* %ao.b, i8 addrspace(200)* addrspace(200)* %ap.b, i32 0)
  call void (i32, ...) @bi(i32 0, i32 0)
  store i8 addrspace(200)* null, i8 addrspace(200)* addrspace(200)* %s.addr.b.b, align 1
  call void @llvm.e(i8* undef, i8* %0)
  %2 = load i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)* %am.b.b, align 1
  call void @f(i32 0, i32 0, i8 addrspace(200)* %an.b, i8 addrspace(200)* %2)
  %b = call i32 @bj(i32 0)
  %bk.w = icmp eq i32 0, 0
  br i1 %bk.w, label %bl.bm, label %for.aw.x

for.aw.x:                                         ; preds = %for.bh.for.q.b
  store i8 addrspace(200)* %al.b, i8 addrspace(200)* addrspace(200)* @aj, align 1
  unreachable

bl.bm:                                            ; preds = %for.bh.for.q.b
  %3 = load i32, i32 addrspace(200)* @br, align 1
  %4 = load i32, i32 addrspace(200)* @ai, align 1
  %5 = load i32, i32 addrspace(200)* @ah, align 1
  %6 = load i32, i32 addrspace(200)* @c, align 1
  %m.b = icmp ne i32 %6, 0
  br label %for.bh.b

for.bh.b:                                         ; preds = %bv.b, %bl.bm
  %y = call %struct.b.2 addrspace(200)* @j()
  %bk.z = icmp eq i32 0, 0
  br i1 %bk.z, label %if.au.b, label %bn.bm

if.au.b:                                          ; preds = %for.bh.b
  %bo.b = icmp eq i32 undef, 3
  %bp.bq.b = and i1 false, %bo.b
  br i1 %bp.bq.b, label %bd.u.b, label %for.v-bs-bt.b

bd.u.b:                                           ; preds = %if.au.b
  %bp.bh.b = and i1 %m.b, false
  br i1 %bp.bh.b, label %if.bu.b, label %bv.b

if.bu.b:                                          ; preds = %bd.u.b
  unreachable

for.v-bs-bt.b:                                    ; preds = %if.au.b
  store i32 7, i32 addrspace(200)* @ae, align 1
  br label %bv.b

bv.b:                                             ; preds = %for.v-bs-bt.b, %bd.u.b
  br label %for.bh.b

bn.bm:                                            ; preds = %for.bh.b
  %bk.b = icmp eq i32 %4, 0
  br i1 %bk.b, label %bw.bm, label %if.k.b

if.k.b:                                           ; preds = %bn.bm
  %. = call i32 @bj(i32 0)
  br label %bw.bm

bw.bm:                                            ; preds = %if.k.b, %bn.bm
  %7 = call i8 addrspace(200)* @llvm.cheri.cap.offset.set(i8 addrspace(200)* undef, i64 0)
  %8 = bitcast i8 addrspace(200)* %7 to i32 (i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)*
  call void @aa(i8 addrspace(200)* bitcast (i32 addrspace(200)* @ad to i8 addrspace(200)*), i8 addrspace(200)* null, i32 0, i32 0, i32 (i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)* %8)
  %.b = call i8 addrspace(200)* @af(i8 addrspace(200)* bitcast (%struct.ab.0 addrspace(200)* getelementptr inbounds (%struct.a.1, %struct.a.1 addrspace(200)* @d, i64 0, i32 2) to i8 addrspace(200)*), i32 0)
  call void @ac(i8 addrspace(200)* null, i8 addrspace(200)* bitcast (i32 addrspace(200)* @ad to i8 addrspace(200)*), i32 0)
  %9 = load i32, i32 addrspace(200)* getelementptr inbounds (%struct.a.1, %struct.a.1 addrspace(200)* @d, i64 0, i32 0)
  %10 = call i8 addrspace(200)* @af(i8 addrspace(200)* bitcast (%struct.ab.0 addrspace(200)* getelementptr inbounds (%struct.a.1, %struct.a.1 addrspace(200)* @d, i64 0, i32 2) to i8 addrspace(200)*), i32 %9)
  store i32 %3, i32 addrspace(200)* @ag, align 1
  call void @ac(i8 addrspace(200)* %1, i8 addrspace(200)* null, i32 %5)
  br label %for.bb

for.bb:                                           ; preds = %bw.bm, %bd.be
  %ba.n = getelementptr i32, i32 addrspace(200)* %az.027, i64 1
  %bb = add i32 %bc.026, 0
  %bg = icmp ult i32 %bb, %r
  br i1 %bg, label %for.aw, label %for.l

for.l:                                            ; preds = %for.l, %for.bb, %if.au
  br label %for.l
}

declare void @g(i8 addrspace(200)*, i8 addrspace(200)* addrspace(200)*, i32)

define internal void @bi(i32, ...) {
  ret void
}

define internal i32 @bj(i32) {
  unreachable
}

declare void @f(i32, i32, i8 addrspace(200)*, i8 addrspace(200)*)

declare %struct.b.2 addrspace(200)* @j()

declare void @ac(i8 addrspace(200)*, i8 addrspace(200)*, i32)

declare void @aa(i8 addrspace(200)*, i8 addrspace(200)*, i32, i32, i32 (i8 addrspace(200)*, i8 addrspace(200)*) addrspace(200)*)

declare i8 addrspace(200)* @af(i8 addrspace(200)*, i32)

attributes #0 = { nounwind readnone }
attributes #1 = { "no-frame-pointer-elim"="true" }

