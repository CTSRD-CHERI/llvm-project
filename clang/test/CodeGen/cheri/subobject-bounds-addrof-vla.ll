; ModuleID = 'clang/test/CodeGen/cheri/subobject-bounds-addrof-vla.c'
source_filename = "clang/test/CodeGen/cheri/subobject-bounds-addrof-vla.c"
target datalayout = "E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128-A200-P200-G200"
target triple = "mips64-unknown-freebsd-purecap"

%struct.VLAContainerNoLength = type { i32, %"struct.VLAContainerNoLength::ArbitraryLengthCharArray" }
%"struct.VLAContainerNoLength::ArbitraryLengthCharArray" = type { i32, [0 x i8] }
%struct.VLAContainerLen0 = type { i32, %"struct.VLAContainerLen0::ZeroLengthCharArray" }
%"struct.VLAContainerLen0::ZeroLengthCharArray" = type { i32, [0 x i8] }
%struct.VLAContainerLen1 = type { i32, %"struct.VLAContainerLen1::LengthOneCharArray" }
%"struct.VLAContainerLen1::LengthOneCharArray" = type { i32, [1 x i8] }
%struct.NotVLAContainerLen2 = type { i32, %"struct.NotVLAContainerLen2::LengthTwoCharArray" }
%"struct.NotVLAContainerLen2::LengthTwoCharArray" = type { i32, [2 x i8] }
%struct.WithVLA = type { float, %"struct.WithVLA::CharVLA" }
%"struct.WithVLA::CharVLA" = type { i32, [0 x i8] }
%struct.WithIntVLA = type { i64, %"struct.WithIntVLA::IntVLA" }
%"struct.WithIntVLA::IntVLA" = type { i32, [0 x i32] }
%struct.NestedVLA = type { double, %struct.WithVLA }
%struct.MaybeVLA = type { i32, [0 x i8] }
%struct.MaybeVLA.0 = type { i32, [1 x i8] }
%struct.MaybeVLA.1 = type { i32, [2 x i8] }
%struct.names = type { ptr addrspace(200), [1 x i8], [15 x i8] }

@global_vn = addrspace(200) global %struct.VLAContainerNoLength zeroinitializer, align 4
@global_v0 = addrspace(200) global %struct.VLAContainerLen0 zeroinitializer, align 4
@global_v1 = addrspace(200) global %struct.VLAContainerLen1 zeroinitializer, align 4
@global_v2 = addrspace(200) global %struct.NotVLAContainerLen2 zeroinitializer, align 4

; Function Attrs: mustprogress nounwind
define dso_local void @_Z25test_subobject_addrof_vlaP7WithVLAP10WithIntVLAP9NestedVLA(ptr addrspace(200) noundef %s1, ptr addrspace(200) noundef %s2, ptr addrspace(200) noundef %n) local_unnamed_addr addrspace(200) #0 {
entry:
  %buf = getelementptr inbounds %struct.WithVLA, ptr addrspace(200) %s1, i64 0, i32 1
  %cur_offset = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %buf)
  %cur_len = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %buf)
  %remaining_bytes = sub i64 %cur_len, %cur_offset
  %0 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf, i64 %remaining_bytes)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %0) #4
  %1 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf, i64 4)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %1) #4
  %data = getelementptr inbounds %struct.WithVLA, ptr addrspace(200) %s1, i64 1
  %cur_offset3 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data)
  %cur_len4 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data)
  %remaining_bytes5 = sub i64 %cur_len4, %cur_offset3
  %2 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data, i64 %remaining_bytes5)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %2) #4
  %buf6 = getelementptr inbounds %struct.WithIntVLA, ptr addrspace(200) %s2, i64 0, i32 1
  %cur_offset7 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %buf6)
  %cur_len8 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %buf6)
  %remaining_bytes9 = sub i64 %cur_len8, %cur_offset7
  %3 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf6, i64 %remaining_bytes9)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %3) #4
  %4 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf6, i64 4)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %4) #4
  %data13 = getelementptr inbounds i8, ptr addrspace(200) %s2, i64 12
  %cur_offset14 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data13)
  %cur_len15 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data13)
  %remaining_bytes16 = sub i64 %cur_len15, %cur_offset14
  %5 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data13, i64 %remaining_bytes16)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %5) #4
  %vla_struct = getelementptr inbounds %struct.NestedVLA, ptr addrspace(200) %n, i64 0, i32 1
  %cur_offset17 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %vla_struct)
  %cur_len18 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %vla_struct)
  %remaining_bytes19 = sub i64 %cur_len18, %cur_offset17
  %6 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %vla_struct, i64 %remaining_bytes19)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %6) #4
  %7 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %vla_struct, i64 4)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %7) #4
  %buf22 = getelementptr inbounds %struct.NestedVLA, ptr addrspace(200) %n, i64 0, i32 1, i32 1
  %cur_offset23 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %buf22)
  %cur_len24 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %buf22)
  %remaining_bytes25 = sub i64 %cur_len24, %cur_offset23
  %8 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf22, i64 %remaining_bytes25)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %8) #4
  %9 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf22, i64 4)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %9) #4
  %data31 = getelementptr inbounds %struct.NestedVLA, ptr addrspace(200) %n, i64 1
  %cur_offset32 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data31)
  %cur_len33 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data31)
  %remaining_bytes34 = sub i64 %cur_len33, %cur_offset32
  %10 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data31, i64 %remaining_bytes34)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %10) #4
  ret void
}

declare void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef) local_unnamed_addr addrspace(200) #1

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200)) addrspace(200) #2

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200)) addrspace(200) #2

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200), i64) addrspace(200) #2

; Function Attrs: mustprogress nounwind
define dso_local void @_Z5test2Pv(ptr addrspace(200) noundef %ptr) local_unnamed_addr addrspace(200) #0 {
entry:
  %cur_offset = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull @global_vn)
  %cur_len = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull @global_vn)
  %remaining_bytes = sub i64 %cur_len, %cur_offset
  %0 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull @global_vn, i64 %remaining_bytes)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %0) #4
  %buf = getelementptr inbounds %struct.VLAContainerNoLength, ptr addrspace(200) %ptr, i64 0, i32 1
  %cur_offset1 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %buf)
  %cur_len2 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %buf)
  %remaining_bytes3 = sub i64 %cur_len2, %cur_offset1
  %1 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf, i64 %remaining_bytes3)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %1) #4
  %data = getelementptr inbounds %struct.VLAContainerNoLength, ptr addrspace(200) %ptr, i64 1
  %cur_offset5 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data)
  %cur_len6 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data)
  %remaining_bytes7 = sub i64 %cur_len6, %cur_offset5
  %2 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data, i64 %remaining_bytes7)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %2) #4
  %cur_offset8 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull @global_v0)
  %cur_len9 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull @global_v0)
  %remaining_bytes10 = sub i64 %cur_len9, %cur_offset8
  %3 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull @global_v0, i64 %remaining_bytes10)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %3) #4
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %1) #4
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %2) #4
  %cur_offset20 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull @global_v1)
  %cur_len21 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull @global_v1)
  %remaining_bytes22 = sub i64 %cur_len21, %cur_offset20
  %4 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull @global_v1, i64 %remaining_bytes22)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %4) #4
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %1) #4
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %2) #4
  %5 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull @global_v2, i64 12)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %5) #4
  %6 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %buf, i64 8)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %6) #4
  %7 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data, i64 2)
  tail call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %7) #4
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200) #3

; Function Attrs: mustprogress nounwind
define dso_local noundef signext i32 @_Z14test_local_vlaii(i32 noundef signext %len, i32 noundef signext %index) local_unnamed_addr addrspace(200) #0 {
entry:
  %0 = zext i32 %len to i64
  %vla = alloca i32, i64 %0, align 4, addrspace(200)
  %cur_offset = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) %vla)
  %cur_len = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %vla)
  %remaining_bytes = sub i64 %cur_len, %cur_offset
  %1 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %vla, i64 %remaining_bytes)
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef %1) #4
  %idxprom = sext i32 %index to i64
  %cur_offset4 = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) %1)
  %cur_len5 = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %1)
  %remaining_bytes6 = sub i64 %cur_len5, %cur_offset4
  %2 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %1, i64 %remaining_bytes6)
  %arrayidx = getelementptr inbounds i32, ptr addrspace(200) %2, i64 %idxprom
  %3 = load i32, ptr addrspace(200) %arrayidx, align 4, !tbaa !2
  ret i32 %3
}

; Function Attrs: mustprogress nounwind
define dso_local void @_Z14test_maybe_vlav() local_unnamed_addr addrspace(200) #0 {
entry:
  %mvla0 = alloca %struct.MaybeVLA, align 4, addrspace(200)
  %mvla1 = alloca %struct.MaybeVLA.0, align 4, addrspace(200)
  %mvla2 = alloca %struct.MaybeVLA.1, align 4, addrspace(200)
  call void @llvm.lifetime.start.p200(i64 4, ptr addrspace(200) %mvla0) #4
  %cur_offset = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) %mvla0)
  %cur_len = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %mvla0)
  %remaining_bytes = sub i64 %cur_len, %cur_offset
  %0 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %mvla0, i64 %remaining_bytes)
  call void @_Z18do_stuff_referenceI8MaybeVLAILi0EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(4) %0) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %0) #4
  %data = getelementptr inbounds %struct.MaybeVLA, ptr addrspace(200) %mvla0, i64 0, i32 1
  %cur_offset4 = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data)
  %cur_len5 = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data)
  %remaining_bytes6 = sub i64 %cur_len5, %cur_offset4
  %1 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data, i64 %remaining_bytes6)
  call void @_Z18do_stuff_referenceIA0_cEvRT_(ptr addrspace(200) noundef nonnull align 1 %1) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %1) #4
  call void @llvm.lifetime.start.p200(i64 8, ptr addrspace(200) %mvla1) #4
  %cur_offset11 = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) %mvla1)
  %cur_len12 = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %mvla1)
  %remaining_bytes13 = sub i64 %cur_len12, %cur_offset11
  %2 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %mvla1, i64 %remaining_bytes13)
  call void @_Z18do_stuff_referenceI8MaybeVLAILi1EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(8) %2) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %2) #4
  %data17 = getelementptr inbounds %struct.MaybeVLA.0, ptr addrspace(200) %mvla1, i64 0, i32 1
  %cur_offset18 = call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data17)
  %cur_len19 = call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data17)
  %remaining_bytes20 = sub i64 %cur_len19, %cur_offset18
  %3 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data17, i64 %remaining_bytes20)
  call void @_Z18do_stuff_referenceIA1_cEvRT_(ptr addrspace(200) noundef nonnull align 1 dereferenceable(1) %3) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %3) #4
  call void @llvm.lifetime.start.p200(i64 8, ptr addrspace(200) %mvla2) #4
  %4 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %mvla2, i64 8)
  call void @_Z18do_stuff_referenceI8MaybeVLAILi2EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(8) %4) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %4) #4
  %data25 = getelementptr inbounds %struct.MaybeVLA.1, ptr addrspace(200) %mvla2, i64 0, i32 1
  %5 = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data25, i64 2)
  call void @_Z18do_stuff_referenceIA2_cEvRT_(ptr addrspace(200) noundef nonnull align 1 dereferenceable(2) %5) #4
  call void @_Z16do_stuff_untypedPv(ptr addrspace(200) noundef nonnull %5) #4
  call void @llvm.lifetime.end.p200(i64 8, ptr addrspace(200) %mvla2) #4
  call void @llvm.lifetime.end.p200(i64 8, ptr addrspace(200) %mvla1) #4
  call void @llvm.lifetime.end.p200(i64 4, ptr addrspace(200) %mvla0) #4
  ret void
}

declare void @_Z18do_stuff_referenceI8MaybeVLAILi0EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(4)) local_unnamed_addr addrspace(200) #1

declare void @_Z18do_stuff_referenceIA0_cEvRT_(ptr addrspace(200) noundef nonnull align 1) local_unnamed_addr addrspace(200) #1

declare void @_Z18do_stuff_referenceI8MaybeVLAILi1EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(8)) local_unnamed_addr addrspace(200) #1

declare void @_Z18do_stuff_referenceIA1_cEvRT_(ptr addrspace(200) noundef nonnull align 1 dereferenceable(1)) local_unnamed_addr addrspace(200) #1

declare void @_Z18do_stuff_referenceI8MaybeVLAILi2EEEvRT_(ptr addrspace(200) noundef nonnull align 4 dereferenceable(8)) local_unnamed_addr addrspace(200) #1

declare void @_Z18do_stuff_referenceIA2_cEvRT_(ptr addrspace(200) noundef nonnull align 1 dereferenceable(2)) local_unnamed_addr addrspace(200) #1

; Function Attrs: mustprogress nounwind
define dso_local void @_Z13ls_regressionP5names(ptr addrspace(200) noundef %np) local_unnamed_addr addrspace(200) #0 {
entry:
  %data = getelementptr inbounds %struct.names, ptr addrspace(200) %np, i64 0, i32 1
  %cur_offset = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) nonnull %data)
  %cur_len = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) nonnull %data)
  %remaining_bytes = sub i64 %cur_len, %cur_offset
  %0 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) nonnull %data, i64 %remaining_bytes)
  %cur_offset1 = tail call i64 @llvm.cheri.cap.offset.get.i64(ptr addrspace(200) %0)
  %cur_len2 = tail call i64 @llvm.cheri.cap.length.get.i64(ptr addrspace(200) %0)
  %remaining_bytes3 = sub i64 %cur_len2, %cur_offset1
  %1 = tail call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) %0, i64 %remaining_bytes3)
  store ptr addrspace(200) %1, ptr addrspace(200) %np, align 16, !tbaa !6
  tail call void @_Z10test_decayPc(ptr addrspace(200) noundef %0) #4
  tail call void @_Z10test_decayPc(ptr addrspace(200) noundef %1) #4
  ret void
}

declare void @_Z10test_decayPc(ptr addrspace(200) noundef) local_unnamed_addr addrspace(200) #1

attributes #0 = { mustprogress nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap" }
attributes #1 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cheri128" "target-features"="+cheri128,+chericap" }
attributes #2 = { mustprogress nofree nosync nounwind willreturn memory(none) }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 18.0.0 (https://github.com/CTSRD-CHERI/llvm-project.git 485803f732f03daa24d2ca4f7c43f9767450c548)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"_ZTS5names", !8, i64 0, !4, i64 16}
!8 = !{!"any pointer", !4, i64 0}
