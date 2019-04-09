# RUN: yaml2obj %s > %t.o
# RUN: ld.lld --eh-frame-hdr --shared -z notext -o %T/libcxxrt.so.1.full -m elf64btsmip_cheri_fbsd %t.o
# RssUN: %clang -nostdinc -target cheri-unknown-freebsd -msoft-float -mabi=purecap -isystem /local/scratch/alr48/cheri/output/sdk256/sysroot/usr/include -I /local/scratch/alr48/cheri/libcxxrt/src -x c++ /local/scratch/alr48/cheri/llvm-master/tools/lld/test/ELF/cheri/libcxxrt-crash.test -c -o - | /local/scratch/alr48/cheri/build/llvm-256-build/./bin/ld.lld --full-shutdown --eh-frame-hdr --shared -z notext -o /local/scratch/alr48/cheri/build/llvm-256-build/tools/lld/test/ELF/cheri/Output/libcxxrt.so.1.full -m elf64btsmip_cheri_fbsd /dev/stdin
# RsUN: ld.lld --eh-frame-hdr --shared -z notext -o %T/libcxxrt.so.1.full -m elf64btsmip_cheri_fbsd %S/Inputs/exception.pico

# Generated from this source code and running %cheri_purecap_clang -isystem /local/scratch/alr48/cheri/output/sdk256/sysroot/usr/include -I /local/scratch/alr48/cheri/libcxxrt/src -x c++ %s -c -o %t.o
# void a(int);
# void b();
# long c;
# void d() { a(c); }
# void e() throw() { b(); }
#

--- !ELF
FileHeader:
  Class:           ELFCLASS64
  Data:            ELFDATA2MSB
  OSABI:           ELFOSABI_FREEBSD
  Type:            ET_REL
  Machine:         EM_MIPS
  Flags:           [ EF_MIPS_NOREORDER, EF_MIPS_PIC, EF_MIPS_CPIC, EF_MIPS_ABI_CHERIABI, EF_MIPS_MACH_CHERI256, EF_MIPS_ARCH_4 ]
Sections:
  - Name:            .text
    Type:            SHT_PROGBITS
    Flags:           [ SHF_ALLOC, SHF_EXECINSTR ]
    AddressAlign:    0x0000000000000010
    Content:         67BDFFC0EBCBE83BFA2BE80003A0F02549B960023C0100000039082D64210000DC390000480CC9FFDC390000DF390000DC2100004881004748210E40C82100030001200048F160000000000003C0E825DA2BE800CBCBE83B67BD004049008800000000000000000067BDFF00EBCBE8FBEB8BE8F3FA2BE80C03A0F02549B960023C0100000039082D6421000067D900A049AC5E4164190020482C664067D9009C49A15E416419000448210E40DC3900004802C9FFF98BF00649AC1000E82BF05BF82BF00248F1600000000000100000010000000003C0E825DA2BE80CCB8BE8F3CBCBE8FB67BD0100490088000000000000A008250001080064050000D82BF006FA010000DA0BF002E8300002E8ABF01B1000000100000000D82BF006D8610000C82BF05BDC220000480C11FF0020E02548F1600000000000
  - Name:            .rela.text
    Type:            SHT_RELA
    Link:            .symtab
    AddressAlign:    0x0000000000000008
    Info:            .text
    Relocations:
      - Offset:          0x0000000000000014
        Symbol:          _Z1dv
        Type:            R_MIPS_GPREL16
        Type2:           R_MIPS_SUB
        Type3:           R_MIPS_HI16
      - Offset:          0x000000000000001C
        Symbol:          _Z1dv
        Type:            R_MIPS_GPREL16
        Type2:           R_MIPS_SUB
        Type3:           R_MIPS_LO16
      - Offset:          0x0000000000000020
        Symbol:          _Z1ai
        Type:            R_MIPS_GOT_DISP
      - Offset:          0x0000000000000028
        Symbol:          .size.c
        Type:            R_MIPS_GOT_DISP
      - Offset:          0x0000000000000030
        Symbol:          c
        Type:            R_MIPS_GOT_DISP
      - Offset:          0x0000000000000080
        Symbol:          _Z1ev
        Type:            R_MIPS_GPREL16
        Type2:           R_MIPS_SUB
        Type3:           R_MIPS_HI16
      - Offset:          0x0000000000000088
        Symbol:          _Z1ev
        Type:            R_MIPS_GPREL16
        Type2:           R_MIPS_SUB
        Type3:           R_MIPS_LO16
      - Offset:          0x00000000000000AC
        Symbol:          _Z1bv
        Type:            R_MIPS_GOT_DISP
      - Offset:          0x0000000000000124
        Symbol:          __cxa_call_unexpected
        Type:            R_MIPS_CALL16
  - Name:            .mdebug.abi64
    Type:            SHT_PROGBITS
    AddressAlign:    0x0000000000000001
    Content:         ''
  - Name:            .pdr
    Type:            SHT_PROGBITS
    AddressAlign:    0x0000000000000004
    Content:         0000000000000000000000000000000000000000000000400000001E0000001F0000000000000000000000000000000000000000000001000000001E0000001F
  - Name:            .rela.pdr
    Type:            SHT_RELA
    Link:            .symtab
    AddressAlign:    0x0000000000000008
    Info:            .pdr
    Relocations:
      - Offset:          0x0000000000000000
        Symbol:          _Z1dv
        Type:            R_MIPS_32
      - Offset:          0x0000000000000020
        Symbol:          _Z1ev
        Type:            R_MIPS_32
  - Name:            .gcc_except_table
    Type:            SHT_PROGBITS
    Flags:           [ SHF_ALLOC ]
    AddressAlign:    0x0000000000000004
    Content:         FF009E808000031A0000004C000000180000008801000000640000006C00000000007F0000000000
  - Name:            .bss
    Type:            SHT_NOBITS
    Flags:           [ SHF_WRITE, SHF_ALLOC ]
    AddressAlign:    0x0000000000000010
    Size:            0x0000000000000008
  - Name:            .global_sizes
    Type:            SHT_PROGBITS
    Flags:           [ SHF_ALLOC ]
    AddressAlign:    0x0000000000000008
    Content:         '0000000000000000'
  - Name:            .comment
    Type:            SHT_PROGBITS
    Flags:           [ SHF_MERGE, SHF_STRINGS ]
    AddressAlign:    0x0000000000000001
    Content:         00636C616E672076657273696F6E20352E302E302000
  - Name:            .note.GNU-stack
    Type:            SHT_PROGBITS
    AddressAlign:    0x0000000000000001
    Content:         ''
  - Name:            .data
    Type:            SHT_PROGBITS
    Flags:           [ SHF_WRITE, SHF_ALLOC ]
    AddressAlign:    0x0000000000000010
    Content:         ''
  - Name:            .MIPS.options
    Type:            SHT_MIPS_OPTIONS
    Flags:           [ SHF_ALLOC, SHF_MIPS_NOSTRIP ]
    AddressAlign:    0x0000000000000008
    Content:         '01280000000000007200003700000000000000000000000000000000000000000000000000000000'
  - Name:            .MIPS.abiflags
    Type:            SHT_MIPS_ABIFLAGS
    Flags:           [ SHF_ALLOC ]
    AddressAlign:    0x0000000000000008
    ISA:             MIPS4
    FpABI:           FP_SOFT
    GPRSize:         REG_64
    Flags1:          [ ODDSPREG ]
  - Name:            .eh_frame
    Type:            SHT_PROGBITS
    Flags:           [ SHF_ALLOC ]
    AddressAlign:    0x0000000000000008
    Content:         0000001000000000017A520001781F011B0C1D000000001C00000018000000000000006400440E40489E01055908440D1E0000000000001C00000000017A504C520001781F0B000000000000000000001B0C1D00000000180000002400000000000000D0080000000000000000000000
  - Name:            .rela.eh_frame
    Type:            SHT_RELA
    Link:            .symtab
    AddressAlign:    0x0000000000000008
    Info:            .eh_frame
    Relocations:
      - Offset:          0x000000000000001C
        Symbol:          '.Lfoo'
        Type:            R_MIPS_PC32
      - Offset:          0x0000000000000047
        Symbol:          __gxx_personality_v0
        Type:            R_MIPS_64
      - Offset:          0x000000000000005C
        Symbol:          '.Lfoo2'
        Type:            R_MIPS_PC32
      - Offset:          0x0000000000000065
        Symbol:          '.Lfoo3'
        Type:            R_MIPS_64
Symbols:
    - Name:            /local/scratch/alr48/cheri/llvm-master/tools/lld/test/ELF/cheri/libcxxrt-crash.test
      Type:            STT_FILE
      Binding:         STB_LOCAL
    - Section:         .text
      Binding:         STB_LOCAL
      Name:            .Lfoo
    - Section:         .text
      Name:            .Lfoo2
      Binding:         STB_LOCAL
      Value:           0x0000000000000068
    - Name:            GCC_except_table1
      Binding:         STB_LOCAL
      Section:         .gcc_except_table
    - Type:            STT_SECTION
      Binding:         STB_LOCAL
      Name:            .Lfoo3
      Section:         .gcc_except_table
    - Name:            _Z1ai
      Binding:         STB_GLOBAL
    - Name:            _Z1bv
      Binding:         STB_GLOBAL
    - Name:            _Z1dv
      Binding:         STB_GLOBAL
      Type:            STT_FUNC
      Section:         .text
      Size:            0x0000000000000064
    - Name:            _Z1ev
      Binding:         STB_GLOBAL
      Type:            STT_FUNC
      Section:         .text
      Value:           0x0000000000000068
      Size:            0x00000000000000D0
    - Name:            __cxa_call_unexpected
      Binding:         STB_GLOBAL
    - Name:            __gxx_personality_v0
      Binding:         STB_GLOBAL
    - Name:            c
      Binding:         STB_GLOBAL
      Type:            STT_OBJECT
      Section:         .bss
      Size:            0x0000000000000008
    - Name:            .size.c
      Binding:         STB_WEAK
      Type:            STT_OBJECT
      Section:         .global_sizes
      Size:            0x0000000000000008
...


