//===- RISCVSystemOperandsZCheriPurecap.td -----------------*- tablegen -*-===//
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

//===----------------------------------------------------------------------===//
// CHERI CSR (control and status register read/write) instruction options.
//===----------------------------------------------------------------------===//


class CheriSysReg<string name, bits<12> op> {
    string Name = name;
    bits<12> Encoding = op;
}

def CheriSysRegsList : GenericTable {
    let FilterClass = "CheriSysReg";
    let Fields = [
        "Name", "Encoding"
    ];
    let PrimaryKey = [ "Encoding" ];
    let PrimaryKeyName = "lookupCheriSysRegByEncoding";
}

def lookupCheriSysRegByName : SearchIndex {
    let Table = CheriSysRegsList;
    let Key = [ "Name" ];
}

//===--------------------------
// Debug-mode Cheri CSRs
//===--------------------------
def  : CheriSysReg<"dpcc", 0x7b1>;
def  : CheriSysReg<"dscratch0c", 0x7b2>;
def  : CheriSysReg<"dscratch1c", 0x7b3>;

//===--------------------------
// Machine-mode Cheri CSRs
//===--------------------------
def : CheriSysReg<"mtvecc", 0x305>;
def : CheriSysReg<"mscratchc", 0x340>;
def : CheriSysReg<"mepcc", 0x341>;

//===--------------------------
// Supervisor-mode Cheri CSRs
//===--------------------------
def : CheriSysReg<"stvecc", 0x105>;
def : CheriSysReg<"sscratchc", 0x140>;
def : CheriSysReg<"sepcc", 0x141>;

//===--------------------------
// User-mode Cheri CSRs
//===--------------------------
def : CheriSysReg<"jvtc", 0x017>;

//===--------------------------
// Zcherihybrid CSRs
//===--------------------------
def : CheriSysReg<"dddc", 0x7bc>;
def : CheriSysReg<"mtdc", 0x74c>;
def : CheriSysReg<"stdc", 0x163>;
def CSR_DDC : CheriSysReg<"ddc" , 0x416>;

//===--------------------------
// Debug CSRs
//===--------------------------
def : CheriSysReg<"dinfc", 0x7bd>;
def : CheriSysReg<"utidc", 0xc80>;
def : CheriSysReg<"stidc", 0x580>;
