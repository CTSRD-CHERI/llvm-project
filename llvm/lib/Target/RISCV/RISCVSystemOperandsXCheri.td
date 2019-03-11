//===- RISCVSystemOperandsXCheri.td ------------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the symbolic operands permitted for various kinds of
// CHERI RISC-V system instruction.
//
//===----------------------------------------------------------------------===//

class SpecialCapReg<string name, bits<5> op> {
  string Name = name;
  bits<5> Encoding = op;
}

def SpecialCapRegsList : GenericTable {
  let FilterClass = "SpecialCapReg";
  let Fields = [ "Name", "Encoding" ];

  let PrimaryKey = [ "Encoding" ];
  let PrimaryKeyName = "lookupSpecialCapRegByEncoding";
}

def lookupSpecialCapRegByName : SearchIndex {
  let Table = SpecialCapRegsList;
  let Key = [ "Name" ];
}

//===------------------------------
// Execution Environment
//===------------------------------
def : SpecialCapReg<"pcc", 0x00>;
def : SpecialCapReg<"ddc", 0x01>;

//===------------------------------
// User Trap Setup/Handling
//===------------------------------
def : SpecialCapReg<"utcc", 0x04>;
def : SpecialCapReg<"uscratchc", 0x06>;
def : SpecialCapReg<"uepcc", 0x07>;

//===------------------------------
// Supervisor Trap Setup/Handling
//===------------------------------
def : SpecialCapReg<"stcc", 0x0c>;
def : SpecialCapReg<"sscratchc", 0x0e>;
def : SpecialCapReg<"sepcc", 0x0f>;

//===------------------------------
// Machine Trap Setup/Handling
//===------------------------------
def : SpecialCapReg<"mtcc", 0x1c>;
def : SpecialCapReg<"mscratchc", 0x1e>;
def : SpecialCapReg<"mepcc", 0x1f>;
