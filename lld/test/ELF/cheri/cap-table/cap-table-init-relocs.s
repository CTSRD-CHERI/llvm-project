# RUN: %cheri128_purecap_llvm-mc -filetype=obj %s -o %t.o
# RUN: ld.lld -shared -o %t %t.o -preemptible-caprelocs=legacy -local-caprelocs=legacy
# RUN: llvm-readobj -r %t | FileCheck %s -check-prefix LEGACY-BOTH-ELF-RELOCS
# RUN: llvm-objdump -C -h -t %t | FileCheck %s -check-prefix LEGACY-BOTH-CAPRELOCS
# RUN: ld.lld -shared -o %t %t.o -preemptible-caprelocs=elf -local-caprelocs=legacy
# RUN: llvm-readobj -r %t | FileCheck %s -check-prefix ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS
# RUN: llvm-objdump -C -h -t %t | FileCheck %s -check-prefix ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS

# TODO: ld.lld -o %t %t -preemptible-caprelocs=elf -local-caprelocs=cbuildcap

# Not sure it makes sense to maintain the -local-caprelocs=elf codepath
# RUN: ld.lld -shared -o %t %t.o -preemptible-caprelocs=elf -local-caprelocs=elf
# RUN: llvm-readobj -r %t | FileCheck %s -check-prefix ELF-BOTH-ELF-RELOCS
# RUN: llvm-objdump -C -h -t %t | FileCheck %s -check-prefix ELF-BOTH-CAPRELOCS

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


# LEGACY-BOTH-CAPRELOCS-LABEL: CAPABILITY RELOCATION RECORDS:
# LEGACY-BOTH-CAPRELOCS-NEXT: 0x0000000000030000	Base:  (0x0000000000000000)	Offset: 0x0000000000000000	Length: 0x0000000000000000	Permissions: 0x00000000
# LEGACY-BOTH-CAPRELOCS-NEXT: 0x0000000000030010	Base: local_var (0x0000000000020008)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000{{$}}
# LEGACY-BOTH-CAPRELOCS-SAME: {{[[:space:]]$}}
# LEGACY-BOTH-CAPRELOCS-LABEL: Sections:
# LEGACY-BOTH-CAPRELOCS: 11 .cap_table    00000020 0000000000030000 DATA
# LEGACY-BOTH-CAPRELOCS: 12 __cap_relocs  00000050 0000000000030020 DATA
# LEGACY-BOTH-CAPRELOCS-LABEL: SYMBOL TABLE:
# LEGACY-BOTH-CAPRELOCS:       0000000000030000 l       .cap_table              00000010 preemptible_var@CAPTABLE
# LEGACY-BOTH-CAPRELOCS:       0000000000030010 l       .cap_table              00000010 local_var@CAPTABLE

# two relative relocs for the location and one for the local var base. And then the ABSPTR/SIZE for the preemptible one
# LEGACY-BOTH-ELF-RELOCS:      Relocations [
# LEGACY-BOTH-ELF-RELOCS-NEXT:   Section (7) .rel.dyn {
# LEGACY-BOTH-ELF-RELOCS-NEXT:     0x30020 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
# LEGACY-BOTH-ELF-RELOCS-NEXT:     0x30048 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
# LEGACY-BOTH-ELF-RELOCS-NEXT:     0x30050 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
# LEGACY-BOTH-ELF-RELOCS-NEXT:     0x30028 R_MIPS_CHERI_ABSPTR/R_MIPS_64/R_MIPS_NONE preemptible_var 0x0 (real addend unknown)
# LEGACY-BOTH-ELF-RELOCS-NEXT:     0x30038 R_MIPS_CHERI_SIZE/R_MIPS_64/R_MIPS_NONE preemptible_var 0x0 (real addend unknown)
# LEGACY-BOTH-ELF-RELOCS-NEXT:   }
# LEGACY-BOTH-ELF-RELOCS-NEXT: ]





# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: CAPABILITY RELOCATION RECORDS:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-NEXT: 0x0000000000030010	Base: local_var (0x0000000000020008)	Offset: 0x0000000000000000	Length: 0x0000000000000008	Permissions: 0x00000000{{$}}
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-SAME: {{[[:space:]]$}}
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: Sections:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS: 11 .cap_table    00000020 0000000000030000 DATA
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS: 12 __cap_relocs  00000028 0000000000030020 DATA
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS-LABEL: SYMBOL TABLE:
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS:       0000000000030000 l       .cap_table              00000010 preemptible_var@CAPTABLE
# ELF-PREEMPTIBLE-LEGACY-LOCAL-CAPRELOCS:       0000000000030010 l       .cap_table              00000010 local_var@CAPTABLE

# One relative reloc for the local __cap_reloc location and one R_MIPS_CHERI_CAPABILITY for the preemptible symbol
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS:      Relocations [
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:   Section (7) .rel.dyn {
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:     0x30020 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
#                                                     ^----- location field in __cap_reloc[0]
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:     0x30028 R_MIPS_REL32/R_MIPS_64/R_MIPS_NONE - 0x0 (real addend unknown)
#                                                     ^----- base field in __cap_reloc[0]
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:     0x30000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE preemptible_var 0x0 (real addend unknown)
#                                                       ^---- preemptible_var@CAPTABLE
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT:   }
# ELF-PREEMPTIBLE-LEGACY-LOCAL-ELF-RELOCS-NEXT: ]


# Finally if we use ELF relocs for local symbols as well we have no __cap_relocs, only two elf relocs:
# ELF-BOTH-CAPRELOCS: CAPABILITY RELOCATION RECORDS:{{$}}
# ELF-BOTH-CAPRELOCS-SAME: {{[[:space:]]$}}
# ELF-BOTH-CAPRELOCS-LABEL: Sections:
# ELF-BOTH-CAPRELOCS-NOT: __cap_relocs
# ELF-BOTH-CAPRELOCS: 11 .cap_table    00000020 0000000000030000 DATA
# ELF-BOTH-CAPRELOCS-NOT: __cap_relocs
# ELF-BOTH-CAPRELOCS-LABEL: SYMBOL TABLE:
# ELF-BOTH-CAPRELOCS:       0000000000030000 l       .cap_table              00000010 preemptible_var@CAPTABLE
# ELF-BOTH-CAPRELOCS:       0000000000030010 l       .cap_table              00000010 local_var@CAPTABLE

# ELF-BOTH-ELF-RELOCS:      Relocations [
# ELF-BOTH-ELF-RELOCS-NEXT:   Section (7) .rel.dyn {
# ELF-BOTH-ELF-RELOCS-NEXT:     0x30010 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE local_var 0x0 (real addend unknown)
#                                    ^---- local_var@CAPTABLE
# ELF-BOTH-ELF-RELOCS-NEXT:     0x30000 R_MIPS_CHERI_CAPABILITY/R_MIPS_NONE/R_MIPS_NONE preemptible_var 0x0 (real addend unknown)
#                                   ^---- preemptible_var@CAPTABLE
# ELF-BOTH-ELF-RELOCS-NEXT:   }
# ELF-BOTH-ELF-RELOCS-NEXT: ]
