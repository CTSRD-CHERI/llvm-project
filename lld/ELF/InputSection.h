//===- InputSection.h -------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_INPUT_SECTION_H
#define LLD_ELF_INPUT_SECTION_H

#include "Relocations.h"
#include "lld/Common/CommonLinkerContext.h"
#include "lld/Common/LLVM.h"
#include "lld/Common/Memory.h"
#include "llvm/ADT/CachedHashString.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Compiler.h"

namespace lld {
namespace elf {

class InputFile;
class Symbol;

class Defined;
struct Partition;
class SyntheticSection;
template <class ELFT> class ObjFile;
class OutputSection;

LLVM_LIBRARY_VISIBILITY extern std::vector<Partition> partitions;

// Returned by InputSectionBase::relsOrRelas. At least one member is empty.
template <class ELFT> struct RelsOrRelas {
  ArrayRef<typename ELFT::Rel> rels;
  ArrayRef<typename ELFT::Rela> relas;
  bool areRelocsRel() const { return rels.size(); }
};

// This is the base class of all sections that lld handles. Some are sections in
// input files, some are sections in the produced output file and some exist
// just as a convenience for implementing special ways of combining some
// sections.
class SectionBase {
public:
  enum Kind { Regular, Synthetic, EHFrame, Merge, Output };

  Kind kind() const { return (Kind)sectionKind; }

  uint8_t sectionKind : 3;

  // The next two bit fields are only used by InputSectionBase, but we
  // put them here so the struct packs better.

  uint8_t bss : 1;

  // Set for sections that should not be folded by ICF.
  uint8_t keepUnique : 1;

  uint8_t partition = 1;
  uint32_t type;
  StringRef name;

  // The 1-indexed partition that this section is assigned to by the garbage
  // collector, or 0 if this section is dead. Normally there is only one
  // partition, so this will either be 0 or 1.
  elf::Partition &getPartition() const;

  // These corresponds to the fields in Elf_Shdr.
  uint64_t flags;
  uint32_t addralign;
  uint32_t entsize;
  uint32_t link;
  uint32_t info;

  OutputSection *getOutputSection();
  const OutputSection *getOutputSection() const {
    return const_cast<SectionBase *>(this)->getOutputSection();
  }

  // Translate an offset in the input section to an offset in the output
  // section.
  uint64_t getOffset(uint64_t offset) const;

  uint64_t getVA(uint64_t offset = 0) const;

  bool isLive() const { return partition != 0; }
  void markLive() { partition = 1; }
  void markDead() { partition = 0; }

protected:
  constexpr SectionBase(Kind sectionKind, StringRef name, uint64_t flags,
                        uint32_t entsize, uint32_t addralign, uint32_t type,
                        uint32_t info, uint32_t link)
      : sectionKind(sectionKind), bss(false), keepUnique(false), type(type),
        name(name), flags(flags), addralign(addralign), entsize(entsize),
        link(link), info(info) {}
};

struct RISCVRelaxAux;

// This corresponds to a section of an input file.
class InputSectionBase : public SectionBase {
public:
  template <class ELFT>
  InputSectionBase(ObjFile<ELFT> &file, const typename ELFT::Shdr &header,
                   StringRef name, Kind sectionKind);

  InputSectionBase(InputFile *file, uint64_t flags, uint32_t type,
                   uint64_t entsize, uint32_t link, uint32_t info,
                   uint32_t addralign, ArrayRef<uint8_t> data, StringRef name,
                   Kind sectionKind);

  static bool classof(const SectionBase *s) { return s->kind() != Output; }

  // The file which contains this section. Its dynamic type is always
  // ObjFile<ELFT>, but in order to avoid ELFT, we use InputFile as
  // its static type.
  InputFile *file;

  // Input sections are part of an output section. Special sections
  // like .eh_frame and merge sections are first combined into a
  // synthetic section that is then added to an output section. In all
  // cases this points one level up.
  SectionBase *parent = nullptr;

  // Section index of the relocation section if exists.
  uint32_t relSecIdx = 0;

  template <class ELFT> ObjFile<ELFT> *getFile() const {
    return cast_or_null<ObjFile<ELFT>>(file);
  }

  // Used by --optimize-bb-jumps and RISC-V linker relaxation temporarily to
  // indicate the number of bytes which is not counted in the size. This should
  // be reset to zero after uses.
  uint32_t bytesDropped = 0;

  mutable bool compressed = false;

  // Whether the section needs to be padded with a NOP filler due to
  // deleteFallThruJmpInsn.
  bool nopFiller = false;

  void drop_back(unsigned num) {
    assert(bytesDropped + num < 256);
    bytesDropped += num;
  }

  void push_back(uint64_t num) {
    assert(bytesDropped >= num);
    bytesDropped -= num;
  }

  mutable const uint8_t *content_;
  uint64_t size;

  void trim() {
    if (bytesDropped) {
      size -= bytesDropped;
      bytesDropped = 0;
    }
  }

  ArrayRef<uint8_t> content() const {
    return ArrayRef<uint8_t>(content_, size);
  }
  ArrayRef<uint8_t> contentMaybeDecompress() const {
    if (compressed)
      decompress();
    return content();
  }

  // The next member in the section group if this section is in a group. This is
  // used by --gc-sections.
  InputSectionBase *nextInSectionGroup = nullptr;

  template <class ELFT> RelsOrRelas<ELFT> relsOrRelas() const;

  // InputSections that are dependent on us (reverse dependency for GC)
  llvm::TinyPtrVector<InputSection *> dependentSections;

  // Returns the size of this section (even if this is a common or BSS.)
  size_t getSize() const;

  InputSection *getLinkOrderDep() const;

  // Get the function symbol that encloses this offset from within the
  // section.
  Defined *getEnclosingFunction(uint64_t offset) const;
  Defined *getEnclosingObject(uint64_t offset) const;
  template <unsigned SymbolType>
  Defined *getEnclosingSymbol(uint64_t offset) const;

  // Returns a source location string. Used to construct an error message.
  std::string getLocation(uint64_t offset) const;
  std::string getSrcMsg(const Symbol &sym, uint64_t offset) const;
  std::string getObjMsg(uint64_t offset) const;

  // Each section knows how to relocate itself. These functions apply
  // relocations, assuming that Buf points to this section's copy in
  // the mmap'ed output buffer.
  template <class ELFT> void relocate(uint8_t *buf, uint8_t *bufEnd);
  static uint64_t getRelocTargetVA(const InputFile *File, RelType Type,
                                   int64_t A, uint64_t P, const Symbol &Sym,
                                   RelExpr Expr, const InputSectionBase *isec,
                                   uint64_t offset);

  // The native ELF reloc data type is not very convenient to handle.
  // So we convert ELF reloc records to our own records in Relocations.cpp.
  // This vector contains such "cooked" relocations.
  SmallVector<Relocation, 0> relocations;

  void addReloc(const Relocation &r) {
    if (r.expr == R_ABS_CAP)
      addRelocCap(r);
    else
      relocations.push_back(r);
  }
  MutableArrayRef<Relocation> relocs() { return relocations; }
  ArrayRef<Relocation> relocs() const { return relocations; }

  union {
    // These are modifiers to jump instructions that are necessary when basic
    // block sections are enabled.  Basic block sections creates opportunities
    // to relax jump instructions at basic block boundaries after reordering the
    // basic blocks.
    JumpInstrMod *jumpInstrMod = nullptr;

    // Auxiliary information for RISC-V linker relaxation. RISC-V does not use
    // jumpInstrMod.
    RISCVRelaxAux *relaxAux;

    // The compressed content size when `compressed` is true.
    size_t compressedSize;
  };

  // A function compiled with -fsplit-stack calling a function
  // compiled without -fsplit-stack needs its prologue adjusted. Find
  // such functions and adjust their prologues.  This is very similar
  // to relocation. See https://gcc.gnu.org/wiki/SplitStacks for more
  // information.
  template <typename ELFT>
  void adjustSplitStackFunctionPrologues(uint8_t *buf, uint8_t *end);


  template <typename T> llvm::ArrayRef<T> getDataAs() const {
    size_t s = content().size();
    assert(s % sizeof(T) == 0);
    return llvm::ArrayRef<T>((const T *)content().data(), s / sizeof(T));
  }

protected:
  template <typename ELFT>
  void parseCompressedHeader();
  void decompress() const;

private:
  void addRelocCap(const Relocation &r);
};

// SectionPiece represents a piece of splittable section contents.
// We allocate a lot of these and binary search on them. This means that they
// have to be as compact as possible, which is why we don't store the size (can
// be found by looking at the next one).
struct SectionPiece {
  SectionPiece() = default;
  SectionPiece(size_t off, uint32_t hash, bool live)
      : inputOff(off), live(live), hash(hash >> 1) {}

  uint32_t inputOff;
  uint32_t live : 1;
  uint32_t hash : 31;
  uint64_t outputOff = 0;
};

static_assert(sizeof(SectionPiece) == 16, "SectionPiece is too big");

// This corresponds to a SHF_MERGE section of an input file.
class MergeInputSection : public InputSectionBase {
public:
  template <class ELFT>
  MergeInputSection(ObjFile<ELFT> &f, const typename ELFT::Shdr &header,
                    StringRef name);
  MergeInputSection(uint64_t flags, uint32_t type, uint64_t entsize,
                    ArrayRef<uint8_t> data, StringRef name);

  static bool classof(const SectionBase *s) { return s->kind() == Merge; }
  void splitIntoPieces();

  // Translate an offset in the input section to an offset in the parent
  // MergeSyntheticSection.
  uint64_t getParentOffset(uint64_t offset) const;

  // Splittable sections are handled as a sequence of data
  // rather than a single large blob of data.
  SmallVector<SectionPiece, 0> pieces;

  // Returns I'th piece's data. This function is very hot when
  // string merging is enabled, so we want to inline.
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  llvm::CachedHashStringRef getData(size_t i) const {
    size_t begin = pieces[i].inputOff;
    size_t end =
        (pieces.size() - 1 == i) ? content().size() : pieces[i + 1].inputOff;
    return {toStringRef(content().slice(begin, end - begin)), pieces[i].hash};
  }

  // Returns the SectionPiece at a given input section offset.
  SectionPiece &getSectionPiece(uint64_t offset);
  const SectionPiece &getSectionPiece(uint64_t offset) const {
    return const_cast<MergeInputSection *>(this)->getSectionPiece(offset);
  }

  SyntheticSection *getParent() const {
    return cast_or_null<SyntheticSection>(parent);
  }

private:
  void splitStrings(StringRef s, size_t size);
  void splitNonStrings(ArrayRef<uint8_t> a, size_t size);
};

struct EhSectionPiece {
  EhSectionPiece(size_t off, InputSectionBase *sec, uint32_t size,
                 unsigned firstRelocation)
      : inputOff(off), sec(sec), size(size), firstRelocation(firstRelocation) {}

  ArrayRef<uint8_t> data() const {
    return {sec->content().data() + this->inputOff, size};
  }

  size_t inputOff;
  ssize_t outputOff = -1;
  InputSectionBase *sec;
  uint32_t size;
  unsigned firstRelocation;
};

// This corresponds to a .eh_frame section of an input file.
class EhInputSection : public InputSectionBase {
public:
  template <class ELFT>
  EhInputSection(ObjFile<ELFT> &f, const typename ELFT::Shdr &header,
                 StringRef name);
  static bool classof(const SectionBase *s) { return s->kind() == EHFrame; }
  template <class ELFT> void split();
  template <class ELFT, class RelTy> void split(ArrayRef<RelTy> rels);

  // Splittable sections are handled as a sequence of data
  // rather than a single large blob of data.
  SmallVector<EhSectionPiece, 0> cies, fdes;

  SyntheticSection *getParent() const;
  uint64_t getParentOffset(uint64_t offset) const;
};

// This is a section that is added directly to an output section
// instead of needing special combination via a synthetic section. This
// includes all input sections with the exceptions of SHF_MERGE and
// .eh_frame. It also includes the synthetic sections themselves.
class InputSection : public InputSectionBase {
public:
  InputSection(InputFile *f, uint64_t flags, uint32_t type, uint32_t addralign,
               ArrayRef<uint8_t> data, StringRef name, Kind k = Regular);
  template <class ELFT>
  InputSection(ObjFile<ELFT> &f, const typename ELFT::Shdr &header,
               StringRef name);

  static bool classof(const SectionBase *s) {
    return s->kind() == SectionBase::Regular ||
           s->kind() == SectionBase::Synthetic;
  }

  // Write this section to a mmap'ed file, assuming Buf is pointing to
  // beginning of the output section.
  template <class ELFT> void writeTo(uint8_t *buf);

  OutputSection *getParent() const {
    return reinterpret_cast<OutputSection *>(parent);
  }

  // This variable has two usages. Initially, it represents an index in the
  // OutputSection's InputSection list, and is used when ordering SHF_LINK_ORDER
  // sections. After assignAddresses is called, it represents the offset from
  // the beginning of the output section this section was assigned to.
  uint64_t outSecOff = 0;

  InputSectionBase *getRelocatedSection() const;

  template <class ELFT, class RelTy>
  void relocateNonAlloc(uint8_t *buf, llvm::ArrayRef<RelTy> rels);

  // Points to the canonical section. If ICF folds two sections, repl pointer of
  // one section points to the other.
  InputSection *repl = this;

  // Used by ICF.
  uint32_t eqClass[2] = {0, 0};

  // Called by ICF to merge two input sections.
  void replace(InputSection *other);

  static InputSection discarded;

private:
  template <class ELFT, class RelTy>
  void copyRelocations(uint8_t *buf, llvm::ArrayRef<RelTy> rels);

  template <class ELFT> void copyShtGroup(uint8_t *buf);
};

static_assert(sizeof(InputSection) <= 168, "InputSection is too big");

class SyntheticSection : public InputSection {
public:
  SyntheticSection(uint64_t flags, uint32_t type, uint32_t addralign,
                   StringRef name)
      : InputSection(nullptr, flags, type, addralign, {}, name,
                     InputSectionBase::Synthetic) {}

  virtual ~SyntheticSection() = default;
  virtual size_t getSize() const = 0;
  virtual bool updateAllocSize() { return false; }
  // If the section has the SHF_ALLOC flag and the size may be changed if
  // thunks are added, update the section size.
  virtual bool isNeeded() const { return true; }
  virtual void finalizeContents() {}
  virtual void writeTo(uint8_t *buf) = 0;

  static bool classof(const SectionBase *sec) {
    return sec->kind() == InputSectionBase::Synthetic;
  }
};

inline bool isDebugSection(const InputSectionBase &sec) {
  return (sec.flags & llvm::ELF::SHF_ALLOC) == 0 &&
         sec.name.starts_with(".debug");
}

// The set of TOC entries (.toc + addend) for which we should not apply
// toc-indirect to toc-relative relaxation. const Symbol * refers to the
// STT_SECTION symbol associated to the .toc input section.
extern llvm::DenseSet<std::pair<const Symbol *, uint64_t>> ppc64noTocRelax;

} // namespace elf

std::string toString(const elf::InputSectionBase *);
} // namespace lld

#endif
