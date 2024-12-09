// RUN: llvm-tblgen -gen-instr-info -I %p -I %p/../../include %s | FileCheck %s
include "ConstraintChecking.inc"

def Src1MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1">;
def Src2MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src2">;
def BothMustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1, @traps_if_sealed $src2">;


// CHECK: BothMustBeSealed = [[BOTH_INSN_NR:[0-9]+]],
// CHECK: Src1MustBeSealed = [[SRC1_INSN_NR:[0-9]+]],
// CHECK: Src2MustBeSealed = [[SRC2_INSN_NR:[0-9]+]],

// CHECK-DAG: { [[SRC2_INSN_NR]], {{.*}}, [[OPINFO_SRC2:[0-9]+]], 0|(1ULL<<MCID::MayTrapOnSealedInput)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::ExtraSrcRegAllocReq)|(1ULL<<MCID::ExtraDefRegAllocReq), 0x0ULL },  // Inst #[[SRC2_INSN_NR]] = Src2MustBeSealed
// CHECK-DAG: { [[SRC1_INSN_NR]], {{.*}}, [[OPINFO_SRC1:[0-9]+]], 0|(1ULL<<MCID::MayTrapOnSealedInput)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::ExtraSrcRegAllocReq)|(1ULL<<MCID::ExtraDefRegAllocReq), 0x0ULL },  // Inst #[[SRC1_INSN_NR]] = Src1MustBeSealed
// CHECK-DAG: { [[BOTH_INSN_NR]], {{.*}}, [[OPINFO_BOTH:[0-9]+]], 0|(1ULL<<MCID::MayTrapOnSealedInput)|(1ULL<<MCID::UnmodeledSideEffects)|(1ULL<<MCID::ExtraSrcRegAllocReq)|(1ULL<<MCID::ExtraDefRegAllocReq), 0x0ULL },  // Inst #[[BOTH_INSN_NR]] = BothMustBeSealed


// CHECK-DAG: /* [[OPINFO_BOTH]] */ { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 },
// CHECK-DAG: /* [[OPINFO_SRC1]] */ { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 },
// CHECK-DAG: /* [[OPINFO_SRC2]] */ { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 },


