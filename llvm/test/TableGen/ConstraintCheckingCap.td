// RUN: llvm-tblgen -gen-instr-info -I %p -I %p/../../include %s | FileCheck %s
include "ConstraintChecking.inc"

def Src1MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1">;
def Src2MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src2">;
def BothMustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1, @traps_if_sealed $src2">;

// CHECK-DAG: MCOperandInfo [[BOTH_UNSEALED_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, };
// CHECK-DAG: MCOperandInfo [[UNSEALED1_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
// CHECK-DAG: MCOperandInfo [[UNSEALED2_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, };

// CHECK: { [[#SRC2_INSN_NR:]], {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[UNSEALED2_OPINFO]]
// CHECK-SAME: },  // Inst #[[#SRC2_INSN_NR]] = Src2MustBeSealed
// CHECK: { [[#SRC1_INSN_NR:]], {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[UNSEALED1_OPINFO]]
// CHECK-SAME: },  // Inst #[[#SRC1_INSN_NR]] = Src1MustBeSealed
// CHECK: { [[#BOTH_INSN_NR:]], {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[BOTH_UNSEALED_OPINFO]]
// CHECK-SAME: },  // Inst #[[#BOTH_INSN_NR]] = BothMustBeSealed
