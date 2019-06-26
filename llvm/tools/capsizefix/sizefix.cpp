#include "llvm/Support/Endian.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/ELFObjectFile.h"
#include <unordered_map>
#include <stdio.h>

using namespace llvm;
using namespace llvm::object;

static const std::string SizePrefix = ".size.";

int main(int argc, char *argv[]) {
  const char* InFile = nullptr;
  bool Verbose = false;
  if (argc == 2) {
    InFile = argv[1];
  } else if (argc == 3 && StringRef("--verbose") == argv[1]) {
    Verbose = true;
    InFile = argv[2];
  } else {
    fprintf(stderr, "Usage: %s [--verbose] {statically linked object}\n", argv[0]);
    return EXIT_FAILURE;
  }
  auto OF = ObjectFile::createObjectFile(InFile);
  // OF is an Expected<T> which requires mandatory checking for errors. This is
  // done by invoking the bool conversion operator.
  if (!OF) {
    fprintf(stderr, "Cannot open %s: %s\n", InFile, toString(OF.takeError()).c_str());
    return EXIT_FAILURE;
  }

  std::unordered_map<uint64_t, std::pair<uint64_t,bool>> SymbolSizes;
  std::vector<std::tuple<uint64_t, uint64_t, bool>> Sections;
  // ObjectFile doesn't allow in-place modification, so we open the file again
  // and write it out.
  FILE *F = fopen(InFile, "r+");
  if (!F) {
    fprintf(stderr, "Cannot open %s\n", InFile);
    return EXIT_FAILURE;
  }
  StringMap<uint64_t> SizeForName;
  std::vector<SymbolRef> SizeSymbols;
  SectionRef SizesSection;

  StringRef Data;
  for (const SectionRef &Sec : OF->getBinary()->sections()) {
    StringRef Name;
    if (!Sec.getName(Name)) {
      if (Name == "__cap_relocs") {
        Data = *Sec.getContents();
        continue;
      } else if (Name == ".global_sizes") {
        SizesSection = Sec;
        continue;
      }
    Sections.push_back(std::make_tuple(Sec.getAddress(), Sec.getSize(),
                Sec.isText()));
    }
  }

  for (const SymbolRef &sym : OF->getBinary()->symbols()) {
    uint64_t Size = ELFSymbolRef(sym).getSize();
    if (Size == 0)
      continue;
    Expected<StringRef> Name = sym.getName();
    if (Name) {
      if (Name->startswith(SizePrefix) && SizesSection.containsSymbol(sym)) {
        SizeSymbols.push_back(sym);
        continue;
      } else
        SizeForName[*Name] = Size;
    }
    Expected<uint64_t> Start = sym.getAddress();
    if (!Start)
      continue;
    Expected<SymbolRef::Type> SymbolType = sym.getType();
    if (SymbolType) {
      SymbolRef::Type type = SymbolType.get();
      SymbolSizes.insert({Start.get(), {Size, (type == SymbolRef::ST_Function)}});
    }
  }
  const size_t entry_size = 40;
  MemoryBufferRef MB = OF->getBinary()->getMemoryBufferRef();
  for (int i = 0, e = Data.size() / entry_size; i < e; i++) {
    const char *entry = Data.data() + (entry_size * i);
    uint64_t base = support::endian::read<uint64_t, support::big, 1>(entry + 8);
    if (!((entry >= MB.getBufferStart()) && (entry <= MB.getBufferEnd()))) {
      fprintf(stderr,
              "Unexpected location for capability relocations section!\n");
      return EXIT_FAILURE;
    }
    auto SizeAndType = SymbolSizes.find(base);
    uint64_t Size = 0;
    bool isFunction = false;
    if (base == 0)
      Size = -1;
    else if (SizeAndType == SymbolSizes.end()) {
      for (auto &Sec : Sections) {
        if (std::get<0>(Sec) < base &&
            (std::get<0>(Sec) + std::get<1>(Sec)) > base) {
          Size = std::get<1>(Sec);
          isFunction = std::get<2>(Sec);
#ifndef NDEBUG
          if (!isFunction && Verbose)
            fprintf(stderr, "Unable to find size of symbol at 0%llx for pointer at 0x%llx\n"
                    "Using section size (%llu bytes) instead\n",
                    (unsigned long long)base,
                    static_cast<unsigned long long>(
                        support::endian::read<uint64_t, support::big, 1>(entry)),
                    static_cast<unsigned long long>(Size));
#endif
          break; // First match wins
        }
      }
      if (Size == 0) {
        if (Verbose)
          fprintf(stderr, "Unable to find size of symbol at 0%llx for pointer at 0x%llx\n",
                 (unsigned long long)base,
                  static_cast<unsigned long long>(
                      support::endian::read<uint64_t, support::big, 1>(entry)));
        continue;
      }
    } else {
      Size = SizeAndType->second.first;
      isFunction = SizeAndType->second.second;
    }
    uint64_t Perms = 0;
    if (isFunction)
        Perms |= (1ULL<<63);
    uint64_t BigSize =
        support::endian::byte_swap<uint64_t, support::big>(Size);
    uint64_t BigPerms =
        support::endian::byte_swap<uint64_t, support::big>(Perms);
    // This is an ugly hack.  object ought to allow modification
    fseek(F, entry - MB.getBufferStart() + 24, SEEK_SET);
    fwrite(&BigSize, sizeof(BigSize), 1, F);
    fwrite(&BigPerms, sizeof(BigPerms), 1, F);
  }
  if (SizesSection != SectionRef()) {
    Data = *SizesSection.getContents();
    uint64_t SectionOffset = Data.data() - MB.getBufferStart();
    for (SymbolRef &sym : SizeSymbols) {
      Expected<uint64_t> Start = sym.getAddress();
      if (!Start)
        continue;
      uint64_t offset = *Start - SizesSection.getAddress();
      Expected<StringRef> SymName = sym.getName();
      if (!SymName)
        continue;
      std::string Name = SymName->str();
      Name = Name.substr(SizePrefix.size(), Name.length() - SizePrefix.size());
      auto SizeIt = SizeForName.find(Name);
      uint64_t Size;
      if (SizeIt == SizeForName.end()) {
        fprintf(stderr, "Unable to find size for symbol %s\n", Name.c_str());
        Size = 0;
      } else
        Size = SizeIt->second;
#ifndef NDEBUG
      if (Verbose) {
        fprintf(stderr, "Writing size %llu for symbol %s\n", (unsigned long long)Size, Name.c_str());
      }
#endif
      fseek(F, SectionOffset + offset, SEEK_SET);
      uint64_t BigSize =
          support::endian::byte_swap<uint64_t, support::big>(Size);
      fwrite(&BigSize, sizeof(BigSize), 1, F);
    }
  }
  fclose(F);
}
