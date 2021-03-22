// RUN: llvm-tblgen -gen-instr-info -I %p -I %p/../../include %s | FileCheck %s
include "ConstraintChecking.inc"

def Src1MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1">;
def Src2MustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src2">;
def BothMustBeSealed : TestInstructionWithConstraints<"@traps_if_sealed $src1, @traps_if_sealed $src2">;

// CHECK-DAG: MCOperandInfo [[BOTH_UNSEALED_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, };
// CHECK-DAG: MCOperandInfo [[UNSEALED1_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, };
// CHECK-DAG: MCOperandInfo [[UNSEALED2_OPINFO:OperandInfo[0-9]+]][] = { { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0, MCOI::OPERAND_REGISTER, 0 }, { TestTarget::RegRegClassID, 0|(1<<MCOI::TrapsIfSealedCapability), MCOI::OPERAND_REGISTER, 0 }, };

// CHECK: { 204, {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[BOTH_UNSEALED_OPINFO]] },  // Inst #204 = BothMustBeSealed
// CHECK: { 205, {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[UNSEALED1_OPINFO]] },  // Inst #205 = Src1MustBeSealed
// CHECK: { 206, {{.+}}, 0|(1ULL<<MCID::MayTrapOnSealedInput){{.+}}, [[UNSEALED2_OPINFO]] },  // Inst #206 = Src2MustBeSealed
