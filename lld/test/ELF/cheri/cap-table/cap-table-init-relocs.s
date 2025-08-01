# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -z now -shared -o %t %t.o -local-caprelocs=legacy
# RUN: llvm-readobj -r %t | FileCheck %s --check-prefix ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS
# RUN: llvm-objdump --cap-relocs -h -t %t | FileCheck %s --check-prefix ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS

# TODO: ld.lld -o %t %t -local-caprelocs=elf

.text
.ent __start
.global __start
__start:
clcbi $c1, %captab20(preemptible_var)($c26)
clcbi $c2, %captab20(local_var)($c26)
.end __start
.data

.global preemptible_var
preemptible_var:
.8byte 0x1234
.size preemptible_var, 8


.hidden local_var
local_var:
.8byte 0x5678
.size local_var, 8


# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: Sections:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS: __cap_relocs  00000028 00000000000004b8 DATA
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS: .captable    00000020 00000000000204f0 DATA
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: SYMBOL TABLE:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS:       00000000000204f0 l     O .captable              0000000000000010 preemptible_var@CAPTABLE
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS:       0000000000020500 l     O .captable              0000000000000010 local_var@CAPTABLE
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: CAPABILITY RELOCATION RECORDS:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-NEXT: 0x0000000000020500	Base: local_var (0x0000000000030518)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-EMPTY:

# One relative reloc for the local __cap_reloc location and one R_MIPS_CHERI_CAPABILITY for the preemptible symbol
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS:      Relocations [
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:   Section (7) .rel.dyn {
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:     0x204F0 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE preemptible_var{{$}}
#                                                       ^---- preemptible_var@CAPTABLE
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:   }
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT: ]
