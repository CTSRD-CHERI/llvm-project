#include "llvm/Support/Endian.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/ELFObjectFile.h"
#include <unordered_map>
#include <stdio.h>

using namespace llvm;
using namespace llvm::object;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s {statically linked object}\n", argv[0]);
    return EXIT_FAILURE;
  }
  auto OF = ObjectFile::createObjectFile(argv[1]);
  std::unordered_map<uint64_t, std::pair<uint64_t,bool>> SectionSizes;
  // ObjectFile doesn't allow in-place modification, so we open the file again
  // and write it out.
  FILE *F = fopen(argv[1], "r+");

  for (const SymbolRef &sym : OF->getBinary()->symbols()) {
    uint64_t Size = ELFSymbolRef(sym).getSize();
    if (Size == 0)
      continue;
    uint64_t Start;
    if (sym.getAddress(Start))
      continue;
    SymbolRef::Type type = sym.getType();
    StringRef Name;
    sym.getName(Name);
    SectionSizes.insert({Start, {Size, (type == SymbolRef::ST_Function)}});
  }
  StringRef Data;
  for (const SectionRef &Sec : OF->getBinary()->sections()) {
    StringRef Name;
    if (Sec.getName(Name))
      continue;
    if (Name == "__cap_relocs") {
      Sec.getContents(Data);
      break;
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
    auto SizeAndType = SectionSizes.find(base);
    if (SizeAndType == SectionSizes.end()) {
      fprintf(stderr, "Unable to find size of symbol at 0%llx for pointer at 0x%llx\n",
              (unsigned long long)base,
              static_cast<unsigned long long>(
                  support::endian::read<uint64_t, support::big, 1>(entry)));
      continue;
    }
    uint64_t Size = SizeAndType->second.first;
    bool isFunction = SizeAndType->second.second;
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
  fclose(F);
}
