; This test caused assertion failures in MIPS DAG->DAG Pattern Instruction Selection
; REQUIRES: asserts
; RUN: %cheri128_llc -target-abi n64 %s -o - | FileCheck %s
%struct.arg_1000_long = type { [1000 x i64] }
%struct.arg_1000___intcap_t = type { [1000 x i8 addrspace(200)*] }

@global_1000_long_struct = common global %struct.arg_1000_long zeroinitializer, align 8
@global_1000___intcap_t_struct = common global %struct.arg_1000___intcap_t zeroinitializer, align 16

; Function Attrs: nounwind
define void @call_1000_long_byval() local_unnamed_addr #0 {
entry:
  ; tail call void @other_func(i8* bitcast (%struct.arg_1000_long* @global_1000_long_struct to i8*)) #2
  tail call void @take_1000_long_byval(%struct.arg_1000_long* byval(%struct.arg_1000_long) nonnull align 8 @global_1000_long_struct) #2
  ; CHECK: jal memcpy
  ret void
}

declare void @other_func(i8*) local_unnamed_addr #1

declare void @take_1000_long_byval(%struct.arg_1000_long* byval(%struct.arg_1000_long) align 8) local_unnamed_addr #1

; Function Attrs: nounwind
define void @call_1000___intcap_t_byval() local_unnamed_addr #0 {
entry:
  ; tail call void @other_func(i8* bitcast (%struct.arg_1000___intcap_t* @global_1000___intcap_t_struct to i8*)) #2
  tail call void @take_1000___intcap_t_byval(%struct.arg_1000___intcap_t* byval(%struct.arg_1000___intcap_t) nonnull align 16 @global_1000___intcap_t_struct) #2
  ; CHECK: jal memcpy
  ret void
}

declare void @take_1000___intcap_t_byval(%struct.arg_1000___intcap_t* byval(%struct.arg_1000___intcap_t) align 16) local_unnamed_addr #1

attributes #0 = { nounwind }
attributes #1 = { nounwind }
attributes #2 = { nounwind }
