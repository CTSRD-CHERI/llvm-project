from __future__ import print_function
import re
import sys

from . import common

if sys.version_info[0] > 2:
  class string:
    expandtabs = str.expandtabs
else:
  import string

# RegEx: this is where the magic happens.

##### Assembly parser

ASM_FUNCTION_X86_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*(@"?(?P=func)"?| -- Begin function (?P=func))\n(?:\s*\.?Lfunc_begin[^:\n]*:\n)?'
    r'(?:\.L(?P=func)\$local:\n)?'      # drop .L<func>$local:
    r'(?:\s*\.type\s+\.L(?P=func)\$local,@function\n)?'  # drop .type .L<func>$local
    r'(?:[ \t]+.cfi_startproc\n|.seh_proc[^\n]+\n)?'  # drop optional cfi
    r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
    r'^\s*(?:[^:\n]+?:\s*\n\s*\.size|\.cfi_endproc|\.globl|\.comm|\.(?:sub)?section|#+ -- End function)',
    flags=(re.M | re.S))

ASM_FUNCTION_ARM_RE = re.compile(
    r'^(?P<func>[0-9a-zA-Z_$]+):\n' # f: (name of function)
    r'(?:\.L(?P=func)\$local:\n)?'  # drop .L<func>$local:
    r'(?:\s*\.type\s+\.L(?P=func)\$local,@function\n)?'  # drop .type .L<func>$local
    r'\s+\.fnstart\n' # .fnstart
    r'(?P<body>.*?)' # (body of the function)
    r'^.Lfunc_end[0-9]+:', # .Lfunc_end0: or # -- End function
    flags=(re.M | re.S))

ASM_FUNCTION_AARCH64_RE = re.compile(
     r'^_?(?P<func>[^:]+):[ \t]*\/\/[ \t]*@"?(?P=func)"?( (Function|Tail Call))?\n'
     r'(?:[ \t]+.cfi_startproc\n)?'  # drop optional cfi noise
     r'(?P<body>.*?)\n'
     # This list is incomplete
     r'^\s*(\.Lfunc_end[0-9]+|// -- End function)',
     flags=(re.M | re.S))

ASM_FUNCTION_AMDGPU_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*;+[ \t]*@"?(?P=func)"?\n[^:]*?'
    r'(?P<body>.*?)\n' # (body of the function)
    # This list is incomplete
    r'^\s*(\.Lfunc_end[0-9]+:\n|\.section)',
    flags=(re.M | re.S))

ASM_FUNCTION_HEXAGON_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*//[ \t]*@"?(?P=func)"?\n[^:]*?'
    r'(?P<body>.*?)\n' # (body of the function)
    # This list is incomplete
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_M68K_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*;[ \t]*@"?(?P=func)"?\n'
    r'(?P<body>.*?)\s*' # (body of the function)
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_MIPS_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@"?(?P=func)"?\n[^:]*?' # f: (name of func)
    r'(?:\s*\.?Ltmp[^:\n]*:\n)?[^:]*?'        # optional .Ltmp<N> for EH
    r'(?:^[ \t]+\.(frame|f?mask|set).*?\n)+'  # Mips+LLVM standard asm prologue
    r'(?P<body>.*?)\n'                        # (body of the function)
    # Mips+LLVM standard asm epilogue
    r'(?:(^[ \t]+\.set[^\n]*?\n)*^[ \t]+\.end.*?\n)'
    r'(\$|\.L)func_end[0-9]+:\n',             # $func_end0: (mips32 - O32) or
                                              # .Lfunc_end0: (mips64 - NewABI)
    flags=(re.M | re.S))

ASM_FUNCTION_MSP430_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*;+[ \t]*@"?(?P=func)"?\n[^:]*?'
    r'(?P<body>.*?)\n'
    r'(\$|\.L)func_end[0-9]+:\n',             # $func_end0:
    flags=(re.M | re.S))

ASM_FUNCTION_AVR_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*;+[ \t]*@"?(?P=func)"?\n[^:]*?'
    r'(?P<body>.*?)\n'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_PPC_RE = re.compile(
    r'#[ \-\t]*Begin function (?P<func>[^.:]+)\n'
    r'.*?'
    r'^[_.]?(?P=func):(?:[ \t]*#+[ \t]*@"?(?P=func)"?)?\n'
    r'(?:^[^#]*\n)*'
    r'(?P<body>.*?)\n'
    # This list is incomplete
    r'(?:^[ \t]*(?:\.(?:long|quad|v?byte)[ \t]+[^\n]+)\n)*'
    r'(?:\.Lfunc_end|L\.\.(?P=func))[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_RISCV_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@"?(?P=func)"?\n'
    r'(?:\s*\.?L(?P=func)\$eh_alias:\n)?'         # optional .L<func>$eh_alias: due to CHERI EH
    r'(?:\s*\.?L(?P=func)\$jump_table_base:\n)?'  # optional .L<func>$jump_table_base: due to CHERI jump tables
    r'(?:\s*\.?L(?P=func)\$local:\n)?'  # optional .L<func>$local: due to -fno-semantic-interposition
    r'(?:\s*\.type\s+\.?L(?P=func)\$local,@function\n)?'  # optional .type .L<func>$local
    r'(?:\s*\.?Lfunc_begin[^:\n]*:\n)?[^:]*?'
    r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_LANAI_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*!+[ \t]*@"?(?P=func)"?\n'
    r'(?:[ \t]+.cfi_startproc\n)?'  # drop optional cfi noise
    r'(?P<body>.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_SPARC_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*!+[ \t]*@"?(?P=func)"?\n'
    r'(?P<body>.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_SYSTEMZ_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@"?(?P=func)"?\n'
    r'[ \t]+.cfi_startproc\n'
    r'(?P<body>.*?)\n'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_AARCH64_DARWIN_RE = re.compile(
    r'^_(?P<func>[^:]+):[ \t]*;[ \t]@"?(?P=func)"?\n'
    r'([ \t]*.cfi_startproc\n[\s]*)?'
    r'(?P<body>.*?)'
    r'([ \t]*.cfi_endproc\n[\s]*)?'
    r'^[ \t]*;[ \t]--[ \t]End[ \t]function',
    flags=(re.M | re.S))

ASM_FUNCTION_ARM_DARWIN_RE = re.compile(
    r'@[ \t]--[ \t]Begin[ \t]function[ \t](?P<func>[^ \t]+?)\n'
    r'^[ \t]*\.globl[ \t]*_(?P=func)[ \t]*'
    r'(?P<directives>.*?)'
    r'^_(?P=func):\n[ \t]*'
    r'(?P<body>.*?)'
    r'^[ \t]*@[ \t]--[ \t]End[ \t]function',
    flags=(re.M | re.S ))

ASM_FUNCTION_ARM_MACHO_RE = re.compile(
    r'^_(?P<func>[^:]+):[ \t]*\n'
    r'([ \t]*.cfi_startproc\n[ \t]*)?'
    r'(?P<body>.*?)\n'
    r'[ \t]*\.cfi_endproc\n',
    flags=(re.M | re.S))

ASM_FUNCTION_THUMBS_DARWIN_RE = re.compile(
    r'^_(?P<func>[^:]+):\n'
    r'(?P<body>.*?)\n'
    r'[ \t]*\.data_region\n',
    flags=(re.M | re.S))

ASM_FUNCTION_THUMB_DARWIN_RE = re.compile(
    r'^_(?P<func>[^:]+):\n'
    r'(?P<body>.*?)\n'
    r'^[ \t]*@[ \t]--[ \t]End[ \t]function',
    flags=(re.M | re.S))

ASM_FUNCTION_ARM_IOS_RE = re.compile(
    r'^_(?P<func>[^:]+):\n'
    r'(?P<body>.*?)'
    r'^[ \t]*@[ \t]--[ \t]End[ \t]function',
    flags=(re.M | re.S))

ASM_FUNCTION_WASM32_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@"?(?P=func)"?\n'
    r'(?P<body>.*?)\n'
    r'^\s*(\.Lfunc_end[0-9]+:\n|end_function)',
    flags=(re.M | re.S))

ASM_FUNCTION_VE_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@(?P=func)\n'
    r'(?:\s*\.?L(?P=func)\$local:\n)?'  # optional .L<func>$local: due to -fno-semantic-interposition
    r'(?:\s*\.type\s+\.?L(?P=func)\$local,@function\n)?'  # optional .type .L<func>$local
    r'(?:\s*\.?Lfunc_begin[^:\n]*:\n)?[^:]*?'
    r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_CSKY_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@(?P=func)\n(?:\s*\.?Lfunc_begin[^:\n]*:\n)?[^:]*?'
    r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

ASM_FUNCTION_NVPTX_RE = re.compile(
    # function attributes and retval
    # .visible .func (.param .align 16 .b8 func_retval0[32])
    #r'^(\.visible\s+)?\.func\s+(\([^\)]*\)\s*)?'
    r'^(\.(func|visible|weak|entry|noreturn|extern)\s+)+(\([^\)]*\)\s*)?'

    # function name
    r'(?P<func>[^\(\n]+)'

    # function name separator (opening brace)
    r'(?P<func_name_separator>\()'

    # function parameters
    # (
    #   .param .align 16 .b8 callee_St8x4_param_0[32]
    # ) // -- Begin function callee_St8x4
    r'[^\)]*\)(\s*//[^\n]*)?\n'

    # function body
    r'(?P<body>.*?)\n'

    # function body end marker
    r'\s*// -- End function',
    flags=(re.M | re.S))

ASM_FUNCTION_LOONGARCH_RE = re.compile(
    r'^_?(?P<func>[^:]+):[ \t]*#+[ \t]*@"?(?P=func)"?\n'
    r'(?:\s*\.?Lfunc_begin[^:\n]*:\n)?[^:]*?'
    r'(?P<body>^##?[ \t]+[^:]+:.*?)\s*'
    r'.Lfunc_end[0-9]+:\n',
    flags=(re.M | re.S))

SCRUB_X86_SHUFFLES_RE = (
    re.compile(
        r'^(\s*\w+) [^#\n]+#+ ((?:[xyz]mm\d+|mem)( \{%k\d+\}( \{z\})?)? = .*)$',
        flags=re.M))

SCRUB_X86_SHUFFLES_NO_MEM_RE = (
    re.compile(
        r'^(\s*\w+) [^#\n]+#+ ((?:[xyz]mm\d+|mem)( \{%k\d+\}( \{z\})?)? = (?!.*(?:mem)).*)$',
        flags=re.M))

SCRUB_X86_SPILL_RELOAD_RE = (
    re.compile(
        r'-?\d+\(%([er])[sb]p\)(.*(?:Spill|Reload))$',
        flags=re.M))
SCRUB_X86_SP_RE = re.compile(r'\d+\(%(esp|rsp)\)')
SCRUB_X86_RIP_RE = re.compile(r'[.\w]+\(%rip\)')
SCRUB_X86_LCP_RE = re.compile(r'\.?LCPI[0-9]+_[0-9]+')
SCRUB_X86_RET_RE = re.compile(r'ret[l|q]')

def scrub_asm_x86(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)

  # Detect shuffle asm comments and hide the operands in favor of the comments.
  if getattr(args, 'no_x86_scrub_mem_shuffle', True):
    asm = SCRUB_X86_SHUFFLES_NO_MEM_RE.sub(r'\1 {{.*#+}} \2', asm)
  else:
    asm = SCRUB_X86_SHUFFLES_RE.sub(r'\1 {{.*#+}} \2', asm)

  # Detect stack spills and reloads and hide their exact offset and whether
  # they used the stack pointer or frame pointer.
  asm = SCRUB_X86_SPILL_RELOAD_RE.sub(r'{{[-0-9]+}}(%\1{{[sb]}}p)\2', asm)
  if getattr(args, 'x86_scrub_sp', True):
    # Generically match the stack offset of a memory operand.
    asm = SCRUB_X86_SP_RE.sub(r'{{[0-9]+}}(%\1)', asm)
  if getattr(args, 'x86_scrub_rip', False):
    # Generically match a RIP-relative memory operand.
    asm = SCRUB_X86_RIP_RE.sub(r'{{.*}}(%rip)', asm)
  # Generically match a LCP symbol.
  asm = SCRUB_X86_LCP_RE.sub(r'{{\.?LCPI[0-9]+_[0-9]+}}', asm)
  if getattr(args, 'extra_scrub', False):
    # Avoid generating different checks for 32- and 64-bit because of 'retl' vs 'retq'.
    asm = SCRUB_X86_RET_RE.sub(r'ret{{[l|q]}}', asm)
  # Strip kill operands inserted into the asm.
  asm = common.SCRUB_KILL_COMMENT_RE.sub('', asm)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_amdgpu(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_arm_eabi(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip kill operands inserted into the asm.
  asm = common.SCRUB_KILL_COMMENT_RE.sub('', asm)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_hexagon(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_powerpc(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip unimportant comments, but leave the token '#' in place.
  asm = common.SCRUB_LOOP_COMMENT_RE.sub(r'#', asm)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  # Strip the tailing token '#', except the line only has token '#'.
  asm = common.SCRUB_TAILING_COMMENT_TOKEN_RE.sub(r'', asm)
  return asm

def scrub_asm_m68k(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm


def cap_size_multiple(offset, cap_size):
  if offset % cap_size == 0:
    return "[[#mul(CAP_SIZE, " + str(offset // cap_size) + ")]]"
  else:
    return "[[#mul(CAP_SIZE, " + str(offset // cap_size) + ") + " + str(offset % cap_size) + "]]"


def offset_to_cap_expr(match, offset, cap_size, last_frame_size):
  if offset == 0:
    return "0"
  if not last_frame_size:
    print("clc/csc: unknown stackframe size:" + unchanged_match(match))
    return str(offset)

  if offset >= last_frame_size:
    print("clc/csc: offset bigger than", last_frame_size, ":" + unchanged_match(match))
    return str(offset)

  difference = last_frame_size - offset
  if difference % cap_size != 0:
    print("clc/csc: modulo wrong:" + unchanged_match(match))
    return cap_size_multiple(offset, cap_size)

  if difference == cap_size:
    return "[[#STACKFRAME_SIZE - CAP_SIZE]]"
  return "[[#STACKFRAME_SIZE - mul(CAP_SIZE, " + str(difference // cap_size) + ")]]"


def unchanged_match(match):
  return match.string[match.start():match.end()]


def _replace_group(match, group, replacement):
  return match.string[match.start():match.start(group)] + replacement + match.string[match.end(group):match.end()]


def do_clc_csc_sub(match, last_frame_size, last_cap_size_ref=None, cap_size=None):
  offset = int(match.group('offset'))
  if cap_size is None:
    cap_size = int(match.group('cap_size'))
    assert cap_size == 16 or cap_size == 32
    last_cap_size_ref[0] = cap_size
  offset_str = offset_to_cap_expr(match, offset, cap_size, last_frame_size)
  result = _replace_group(match, 'offset', offset_str)
  # print('replacing ', match.string[match.start():match.end()], 'with', result)
  return result


def do_save_load_dword_sub(match, last_frame_size):
  offset = int(match.group('offset'))
  # dwords are stored after capabilities so usually this can be $STACKFRAME_SIZE - n*dword
  if not last_frame_size:
    print("cld/csd: unknown stackframe size:" + unchanged_match(match))
    return unchanged_match(match)
  if offset >= last_frame_size:
    print("cld/csd: offset bigger than", last_frame_size, ":" + unchanged_match(match))
    return unchanged_match(match)
  difference = last_frame_size - offset
  if difference % 8 != 0:
    print("cld/csd: modulo wrong:" + unchanged_match(match))
    return unchanged_match(match)
  result = _replace_group(match, 'offset', "[[#STACKFRAME_SIZE - " + str(difference) + "]]")
  # print('replacing ', match.string[match.start():match.end()], 'with', result)
  return result


def do_cfi_offset_sub(match, last_frame_size, last_cap_size):
  cap_size = 16 if last_cap_size is None else last_cap_size
  offset_str = offset_to_cap_expr(match, int(match.group('offset')), cap_size, last_frame_size)
  return _replace_group(match, 'offset', offset_str)


def do_stackframe_size_sub(match, last_frame_size_ref):
  if match.group("addi_size") is not None:
    last_frame_size_ref[0] = int(match.group("addi_size"))
    return _replace_group(match, 'addi_size', "[[#STACKFRAME_SIZE:]]")
  else:
    last_frame_size_ref[0] = int(match.group("size"))
    return _replace_group(match, 'size', "[[#STACKFRAME_SIZE:]]")


def scrub_asm_mips(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)

  if not getattr(args, 'scrub_stack_indices', False):
    return asm

  # "pass-by-reference"
  last_frame_size_ref = [None]
  stackframe_size_regex = re.compile(r'(daddiu\s+\$1, \$zero, -(?P<addi_size>\d+)\n\s*)?cincoffset \$c11, \$c11, -?(?P<size>(\d+)|(\$1))')
  asm = stackframe_size_regex.sub(lambda m: do_stackframe_size_sub(m, last_frame_size_ref), asm, count=1)
  if not last_frame_size_ref[0]:
    mips_stackframe_size_regex = re.compile(r'daddiu \$sp, \$sp, -(?P<size>\d+)')
    asm = mips_stackframe_size_regex.sub(lambda m: do_stackframe_size_sub(m, last_frame_size_ref), asm, count=1)
  last_frame_size = last_frame_size_ref[0]
  if last_frame_size is not None:
    asm = re.sub(r'.cfi_def_cfa_offset ' + str(last_frame_size), '.cfi_def_cfa_offset [[#STACKFRAME_SIZE]]', asm, count=1)

  # Expand .cfi offsets and clc offset to #CAPS_SIZE for CHERI128/256
  stack_store_cap_re = re.compile(r'(?:csc|clc) \$\w+, \$zero, \-?(?P<offset>\d+)\(\$c11\) *# (?P<cap_size>16|32)\-byte Folded (Spill|Reload)')
  last_cap_size_ref = [None]
  asm = stack_store_cap_re.sub(lambda m: do_clc_csc_sub(m, last_frame_size, last_cap_size_ref), asm)
  last_cap_size = last_cap_size_ref[0]
  stack_store_dword_re = re.compile(r'(?:csd|cld) \$\w+, \$zero, \-?(?P<offset>\d+)\(\$c11\)')
  asm = stack_store_dword_re.sub(lambda m: do_save_load_dword_sub(m, last_frame_size), asm)
  cfi_offset_regex = re.compile(r'\.cfi_offset (?P<reg>[$\w]+), -(?P<offset>\d+)')
  asm = cfi_offset_regex.sub(lambda m: do_cfi_offset_sub(m, last_frame_size, last_cap_size), asm)
  stackframe_inc_return_regex = re.compile(r'cjr \$c17\n *cincoffset \$c11, \$c11, \d+')
  asm = stackframe_inc_return_regex.sub('cjr $c17\n  cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]', asm)
  # Finally try to replace all other stack cincoffsets
  if last_frame_size:
    asm = re.sub("cincoffset\s+\$c11,\s+\$c11, " + str(last_frame_size), "cincoffset $c11, $c11, [[#STACKFRAME_SIZE]]", asm)
    asm = re.sub("daddiu \$1, \$zero, " + str(last_frame_size), "daddiu $1, $zero, [[#STACKFRAME_SIZE]]", asm)
    asm = re.sub("daddiu\s+\$sp,\s+\$sp, " + str(last_frame_size), "daddiu $sp, $sp, [[#STACKFRAME_SIZE]]", asm)
  return asm

def scrub_asm_msp430(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_avr(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_riscv(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_lanai(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_sparc(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_systemz(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_wasm32(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_ve(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_csky(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip kill operands inserted into the asm.
  asm = common.SCRUB_KILL_COMMENT_RE.sub('', asm)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_nvptx(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

def scrub_asm_loongarch(asm, args):
  # Scrub runs of whitespace out of the assembly, but leave the leading
  # whitespace in place.
  asm = common.SCRUB_WHITESPACE_RE.sub(r' ', asm)
  # Expand the tabs used for indentation.
  asm = string.expandtabs(asm, 2)
  # Strip trailing whitespace.
  asm = common.SCRUB_TRAILING_WHITESPACE_RE.sub(r'', asm)
  return asm

# Returns a tuple of a scrub function and a function regex. Scrub function is
# used to alter function body in some way, for example, remove trailing spaces.
# Function regex is used to match function name, body, etc. in raw llc output.
def get_run_handler(triple):
  target_handlers = {
      'i686': (scrub_asm_x86, ASM_FUNCTION_X86_RE),
      'x86': (scrub_asm_x86, ASM_FUNCTION_X86_RE),
      'i386': (scrub_asm_x86, ASM_FUNCTION_X86_RE),
      'arm64_32-apple-ios': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'arm64_32-apple-watchos2.0.0': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'aarch64': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_RE),
      'aarch64-apple-darwin': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'aarch64-apple-ios': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'hexagon': (scrub_asm_hexagon, ASM_FUNCTION_HEXAGON_RE),
      'r600': (scrub_asm_amdgpu, ASM_FUNCTION_AMDGPU_RE),
      'amdgcn': (scrub_asm_amdgpu, ASM_FUNCTION_AMDGPU_RE),
      'arm': (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_RE),
      'arm64': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_RE),
      'arm64e': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'arm64ec': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_RE),
      'arm64-apple-ios': (scrub_asm_arm_eabi, ASM_FUNCTION_AARCH64_DARWIN_RE),
      'armv7-apple-ios' : (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_IOS_RE),
      'armv7-apple-darwin': (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_DARWIN_RE),
      'thumb': (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_RE),
      'thumb-macho': (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_MACHO_RE),
      'thumbv5-macho': (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_MACHO_RE),
      'thumbv7s-apple-darwin' : (scrub_asm_arm_eabi, ASM_FUNCTION_THUMBS_DARWIN_RE),
      'thumbv7-apple-darwin' : (scrub_asm_arm_eabi, ASM_FUNCTION_THUMB_DARWIN_RE),
      'thumbv7-apple-ios' : (scrub_asm_arm_eabi, ASM_FUNCTION_ARM_IOS_RE),
      'm68k': (scrub_asm_m68k, ASM_FUNCTION_M68K_RE),
      'mips': (scrub_asm_mips, ASM_FUNCTION_MIPS_RE),
      'cheri': (scrub_asm_mips, ASM_FUNCTION_MIPS_RE),
      'msp430': (scrub_asm_msp430, ASM_FUNCTION_MSP430_RE),
      'avr': (scrub_asm_avr, ASM_FUNCTION_AVR_RE),
      'ppc32': (scrub_asm_powerpc, ASM_FUNCTION_PPC_RE),
      'powerpc': (scrub_asm_powerpc, ASM_FUNCTION_PPC_RE),
      'riscv32': (scrub_asm_riscv, ASM_FUNCTION_RISCV_RE),
      'riscv64': (scrub_asm_riscv, ASM_FUNCTION_RISCV_RE),
      'lanai': (scrub_asm_lanai, ASM_FUNCTION_LANAI_RE),
      'sparc': (scrub_asm_sparc, ASM_FUNCTION_SPARC_RE),
      's390x': (scrub_asm_systemz, ASM_FUNCTION_SYSTEMZ_RE),
      'wasm32': (scrub_asm_wasm32, ASM_FUNCTION_WASM32_RE),
      've': (scrub_asm_ve, ASM_FUNCTION_VE_RE),
      'csky': (scrub_asm_csky, ASM_FUNCTION_CSKY_RE),
      'nvptx': (scrub_asm_nvptx, ASM_FUNCTION_NVPTX_RE),
      'loongarch32': (scrub_asm_loongarch, ASM_FUNCTION_LOONGARCH_RE),
      'loongarch64': (scrub_asm_loongarch, ASM_FUNCTION_LOONGARCH_RE)
  }
  handler = None
  best_prefix = ''
  for prefix, s in target_handlers.items():
    if triple.startswith(prefix) and len(prefix) > len(best_prefix):
      handler = s
      best_prefix = prefix

  if handler is None:
    raise KeyError('Triple %r is not supported' % (triple))

  return handler

##### Generator of assembly CHECK lines

def add_checks(output_lines, comment_marker, prefix_list, func_dict,
               func_name, global_vars_seen_dict, is_filtered):
  # Label format is based on ASM string.
  check_label_format = '{} %s-LABEL: %s%s%s'.format(comment_marker)
  return common.add_checks(output_lines, comment_marker, prefix_list, func_dict,
                           func_name, check_label_format, True, False,
                           global_vars_seen_dict, is_filtered=is_filtered)
