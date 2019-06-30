# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -pie %t.o -o %t.exe
# RUN: llvm-readobj -h %t.exe | FileCheck %s

.text
.global __start
.ent __start
__start:
nop
.end __start


# CHECK:      Format: ELF64-mips
# CHECK-NEXT: Arch: cheri
# CHECK-NEXT: AddressSize: 64bit
# CHECK-NEXT: LoadName:
# CHECK-NEXT: ElfHeader {
# CHECK-NEXT:   Ident {
# CHECK-NEXT:     Magic: (7F 45 4C 46)
# CHECK-NEXT:     Class: 64-bit (0x2)
# CHECK-NEXT:     DataEncoding: BigEndian (0x2)
# CHECK-NEXT:     FileVersion: 1
# CHECK-NEXT:     OS/ABI: FreeBSD (0x9)
# CHECK-NEXT:     ABIVersion: 2
# CHECK-NEXT:     Unused: (00 00 00 00 00 00 00)
# CHECK-NEXT:   }
# CHECK-NEXT:   Type: SharedObject (0x3)
# CHECK-NEXT:   Machine: EM_MIPS (0x8)
# CHECK-NEXT:   Version: 1
# CHECK:        Flags [ (0x30C1C006)
# CHECK-NEXT:     EF_MIPS_ABI_CHERIABI (0xC000)
# CHECK-NEXT:     EF_MIPS_ARCH_4 (0x30000000)
# CHECK-NEXT:     EF_MIPS_CPIC (0x4)
# CHECK-NEXT:     EF_MIPS_MACH_CHERI128 (0xC10000)
# CHECK-NEXT:     EF_MIPS_PIC (0x2)
# CHECK-NEXT:   ]
