//===- Writer.h -------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_WRITER_H
#define LLD_ELF_WRITER_H

#include "Config.h"
#include "llvm/ADT/StringRef.h"
#include <cstdint>

namespace lld::elf {
class InputFile;
class OutputSection;
void copySectionsIntoPartitions();
template <class ELFT> void createSyntheticSections();
template <class ELFT> void writeResult();

// This describes a program header entry.
// Each contains type, access flags and range of output sections that will be
// placed in it.
struct PhdrEntry {
  PhdrEntry(unsigned type, unsigned flags)
      : p_align(type == llvm::ELF::PT_LOAD ? config->maxPageSize : 0),
        p_type(type), p_flags(flags) {}
  void add(OutputSection *sec);

  uint64_t p_paddr = 0;
  uint64_t p_vaddr = 0;
  uint64_t p_memsz = 0;
  uint64_t p_filesz = 0;
  uint64_t p_offset = 0;
  uint32_t p_align = 0;
  uint32_t p_type = 0;
  uint32_t p_flags = 0;

  OutputSection *firstSec = nullptr;
  OutputSection *lastSec = nullptr;
  bool hasLMA = false;

  uint64_t lmaOffset = 0;
};

void addReservedSymbols();

bool isCheriAbi(const InputFile *f);

template <class ELFT> uint32_t calcMipsEFlags();

uint8_t getMipsFpAbiFlag(uint8_t oldFlag, llvm::StringRef oldFile,
                         uint8_t newFlag, llvm::StringRef newFile);
uint8_t getMipsIsaExt(uint64_t oldExt, llvm::StringRef oldFile, uint64_t newExt,
                      llvm::StringRef newFile);
void checkMipsShlibCompatible(InputFile *f, uint64_t shlibCheriFlags,
                              uint64_t targetCheriFlags);
bool isRelroSection(const OutputSection *sec);

bool isMipsN32Abi(const InputFile *f);
bool isMicroMips();
bool isMipsR6();

bool hasDynamicLinker();
} // namespace lld::elf

#endif
