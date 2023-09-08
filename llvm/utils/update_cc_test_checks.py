#!/usr/bin/env python3
'''A utility to update LLVM IR CHECK lines in C/C++ FileCheck test files.

Example RUN lines in .c/.cc test files:

// RUN: %clang -emit-llvm -S %s -o - -O2 | FileCheck %s
// RUN: %clangxx -emit-llvm -S %s -o - -O2 | FileCheck -check-prefix=CHECK-A %s

Usage:

% utils/update_cc_test_checks.py --llvm-bin=release/bin test/a.cc
% utils/update_cc_test_checks.py --clang=release/bin/clang /tmp/c/a.cc
'''

from __future__ import print_function

import argparse
import collections
import distutils.spawn
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

from UpdateTestChecks import common

sys.path.append(os.path.join(os.path.dirname(__file__), "lit"))
from lit.TestRunner import ShellEnvironment, executeShCmd, ShellCommandResult
from lit.ShUtil import ShParser, Pipeline, Command

SUBST = {
    '%clang': [],
    '%clang_cc1': ['-cc1'],
    '%clangxx': ['--driver-mode=g++'],
    '%cheri_cc1': ['-cc1', "-triple=mips64-unknown-freebsd", "-target-cpu", "cheri128", "-cheri-size", "128"],
    '%cheri_clang': ['-target', 'mips64-unknown-freebsd', '-cheri'],
    '%cheri128_cc1': ['-cc1', "-triple=mips64-unknown-freebsd", "-target-cpu", "cheri128", "-cheri-size", "128"],
    '%cheri_purecap_clang': ['-target', 'mips64-unknown-freebsd', '-mabi=purecap'],
    '%cheri_purecap_cc1': ['-cc1', "-triple=mips64-unknown-freebsd", "-target-abi", "purecap", "-target-cpu", "cheri128", "-cheri-size", "128"],
    '%cheri128_purecap_cc1': ['-cc1', "-triple=mips64-unknown-freebsd", "-target-abi", "purecap", "-target-cpu", "cheri128", "-cheri-size", "128"],
    '%riscv32_cheri_cc1': ['-cc1', "-triple=riscv32-unknown-freebsd", "-target-feature", "+xcheri"],
    '%riscv64_cheri_cc1': ['-cc1', "-triple=riscv64-unknown-freebsd", "-target-feature", "+xcheri"],
    '%riscv32_cheri_clang': ['-target', 'riscv32-unknown-freebsd', '-march=rv32imafdcxcheri'],
    '%riscv64_cheri_clang': ['-target', 'riscv64-unknown-freebsd', '-march=rv64imafdcxcheri'],
    '%riscv32_cheri_purecap_cc1': ['-cc1', "-triple=riscv32-unknown-freebsd", "-target-feature", "+xcheri", "-target-abi", "il32pc64", '-target-feature', '+cap-mode'],
    '%riscv64_cheri_purecap_cc1': ['-cc1', "-triple=riscv64-unknown-freebsd", "-target-feature", "+xcheri", "-target-abi", "l64pc128", '-target-feature', '+cap-mode'],
    '%riscv32_cheri_purecap_clang': ['-target', 'riscv32-unknown-freebsd', '-march=rv32imafdcxcheri', '-mabi=il32pc64'],
    '%riscv64_cheri_purecap_clang': ['-target', 'riscv64-unknown-freebsd', '-march=rv64imafdcxcheri', '-mabi=l64pc128'],
}

def get_line2func_list(clang_cmd: Command):
  ret = collections.defaultdict(list)
  # Use clang's JSON AST dump to get the mangled name
  json_dump_args = clang_cmd.args + ['-fsyntax-only', '-o', '-']
  if '-cc1' not in json_dump_args:
    # For tests that invoke %clang instead if %clang_cc1 we have to use
    # -Xclang -ast-dump=json instead:
    json_dump_args.append('-Xclang')
  json_dump_args.append('-ast-dump=json')
  common.debug('Running', ' '.join(json_dump_args))

  popen = subprocess.Popen(json_dump_args, stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE, universal_newlines=True)
  stdout, stderr = popen.communicate()
  if popen.returncode != 0:
    sys.stderr.write('Failed to run ' + ' '.join(json_dump_args) + '\n')
    sys.stderr.write(stderr)
    sys.stderr.write(stdout)
    sys.exit(2)

  # Parse the clang JSON and add all children of type FunctionDecl.
  # TODO: Should we add checks for global variables being emitted?
  def parse_clang_ast_json(node, loc, search):
    node_kind = node['kind']
    # Recurse for the following nodes that can contain nested function decls:
    if node_kind in ('NamespaceDecl', 'LinkageSpecDecl', 'TranslationUnitDecl',
                     'CXXRecordDecl', 'ClassTemplateSpecializationDecl'):
      # Specializations must use the loc from the specialization, not the
      # template, and search for the class's spelling as the specialization
      # does not mention the method names in the source.
      if node_kind == 'ClassTemplateSpecializationDecl':
        inner_loc = node['loc']
        inner_search = node['name']
      else:
        inner_loc = None
        inner_search = None
      if 'inner' in node:
        for inner in node['inner']:
          parse_clang_ast_json(inner, inner_loc, inner_search)
    # Otherwise we ignore everything except functions:
    if node_kind not in ('FunctionDecl', 'CXXMethodDecl', 'CXXConstructorDecl',
                         'CXXDestructorDecl', 'CXXConversionDecl'):
      return
    if loc is None:
      loc = node['loc']
    if node.get('isImplicit') is True and node.get('storageClass') == 'extern':
      common.debug('Skipping builtin function:', node['name'], '@', loc)
      return
    common.debug('Found function:', node['kind'], node['name'], '@', loc)
    line = loc.get('line')
    # If there is no line it is probably a builtin function -> skip
    if line is None:
      common.debug('Skipping function without line number:', node['name'], '@', loc)
      return

    # If there is no 'inner' object, it is a function declaration and we can
    # skip it. However, function declarations may also contain an 'inner' list,
    # but in that case it will only contains ParmVarDecls. If we find an entry
    # that is not a ParmVarDecl, we know that this is a function definition.
    has_body = False
    if 'inner' in node:
      for i in node['inner']:
        if i.get('kind', 'ParmVarDecl') != 'ParmVarDecl':
          has_body = True
          break
    if not has_body:
      common.debug('Skipping function without body:', node['name'], '@', loc)
      return
    spell = node['name']
    if search is None:
      search = spell
    mangled = node.get('mangledName', spell)
    ret[int(line)-1].append((spell, mangled, search))

  ast = json.loads(stdout)
  if ast['kind'] != 'TranslationUnitDecl':
    common.error('Clang AST dump JSON format changed?')
    sys.exit(2)
  parse_clang_ast_json(ast, None, None)

  for line, funcs in sorted(ret.items()):
    for func in funcs:
      common.debug('line {}: found function {}'.format(line+1, func), file=sys.stderr)
  if not ret:
    common.warn('Did not find any functions using', ' '.join(json_dump_args))
  return ret


def str_to_commandline(value):
  if not value:
    return []
  return shlex.split(value)


def infer_dependent_args(args):
  if not args.clang:
    if not args.llvm_bin:
      args.clang = 'clang'
    else:
      args.clang = os.path.join(args.llvm_bin, 'clang')
  if not args.opt:
    if not args.llvm_bin:
      args.opt = 'opt'
    else:
      args.opt = os.path.join(args.llvm_bin, 'opt')


def config():
  parser = argparse.ArgumentParser(
      description=__doc__,
      formatter_class=argparse.RawTextHelpFormatter)
  parser.add_argument('--llvm-bin', help='llvm $prefix/bin path')
  parser.add_argument('--clang',
                      help='"clang" executable, defaults to $llvm_bin/clang')
  parser.add_argument('--clang-args', default=[], type=str_to_commandline,
                      help='Space-separated extra args to clang, e.g. --clang-args=-v')
  parser.add_argument('--opt',
                      help='"opt" executable, defaults to $llvm_bin/opt')
  parser.add_argument(
      '--functions', nargs='+', help='A list of function name regexes. '
      'If specified, update CHECK lines for functions matching at least one regex')
  parser.add_argument(
      '--x86_extra_scrub', action='store_true',
      help='Use more regex for x86 matching to reduce diffs between various subtargets')
  parser.add_argument('--function-signature', action='store_true',
                      help='Keep function signature information around for the check line')
  parser.add_argument('--check-attributes', action='store_true',
                      help='Check "Function Attributes" for functions')
  parser.add_argument('--check-globals', action='store_true',
                      help='Check global entries (global variables, metadata, attribute sets, ...) for functions')
  parser.add_argument('tests', nargs='+')
  args = common.parse_commandline_args(parser)
  infer_dependent_args(args)

  if not distutils.spawn.find_executable(args.clang):
    print('Please specify --llvm-bin or --clang', file=sys.stderr)
    sys.exit(1)

  # Determine the builtin includes directory so that we can update tests that
  # depend on the builtin headers. See get_clang_builtin_include_dir() and
  # use_clang() in llvm/utils/lit/lit/llvm/config.py.
  try:
    builtin_include_dir = subprocess.check_output(
      [args.clang, '-print-file-name=include']).decode().strip()
    SUBST['%clang_cc1'] = ['-cc1', '-internal-isystem', builtin_include_dir,
                           '-nostdsysteminc']
  except subprocess.CalledProcessError:
    common.warn('Could not determine clang builtins directory, some tests '
                'might not update correctly.')

  if not distutils.spawn.find_executable(args.opt):
    # Many uses of this tool will not need an opt binary, because it's only
    # needed for updating a test that runs clang | opt | FileCheck. So we
    # defer this error message until we find that opt is actually needed.
    args.opt = None

  return args, parser


def execute_pipeline(commands: Pipeline) -> 'tuple[int, list[ShellCommandResult]]':
  shenv = ShellEnvironment(os.getcwd(), os.environ)
  results: 'list[ShellCommandResult]' = []
  exitCode, _ = executeShCmd(commands, shenv, results)
  return exitCode, results

def get_function_body(builder, args, filename, clang_pipeline: 'list[Command]',
                      prefixes):
  # TODO Clean up duplication of asm/common build_function_body_dictionary
  # Invoke external tool and extract function bodies.
  exitCode, results = execute_pipeline(Pipeline(clang_pipeline))
  raw_tool_output = results[-1].stdout.replace('\r\n', '\n')
  if '-emit-llvm' in clang_pipeline[0].args:
    builder.process_run_line(
            common.OPT_FUNCTION_RE, common.scrub_body, raw_tool_output,
            prefixes, False)
    builder.processed_prefixes(prefixes)
  else:
    print('The clang command line should include -emit-llvm as asm tests '
          'are discouraged in Clang testsuite.', file=sys.stderr)
    sys.exit(1)


def exec_run_line(commands: Pipeline):
  exitcode, results = execute_pipeline(commands)
  if exitcode != 0:
    sys.stderr.write('Failed to run ' + str(commands) + '\n')
    for result in results:
      sys.stderr.write(result.stderr)
      sys.stderr.write(result.stdout)
    sys.exit(3)

def main():
  initial_args, parser = config()
  script_name = os.path.basename(__file__)

  for ti in common.itertests(initial_args.tests, parser, 'utils/' + script_name,
                             comment_prefix='//', argparse_callback=infer_dependent_args):
    # Build a list of filechecked and non-filechecked RUN lines.
    run_list = []
    line2func_list = collections.defaultdict(list)

    subs = [
      ('%s', ti.path),
      ('%t', tempfile.NamedTemporaryFile().name),
      ('%S', os.path.dirname(ti.path)),
    ]

    for l in ti.run_lines:
      pipeline = ShParser(l, win32Escapes=False, pipefail=False).parse()
      if not isinstance(pipeline, Pipeline):
        # Could be a sequence separated by &&/||/; but we don't handle that yet.
        print('WARNING: RUN: line is too complex for this script: ', pipeline,
              file=sys.stderr)
        continue
      triple_in_cmd = None
      clang_cmd: Command = pipeline.commands[0]
      m = common.TRIPLE_ARG_RE.search(' '.join(clang_cmd.args))
      if m:
        triple_in_cmd = m.groups()[0]

      # Do lit-like substitutions on the command and redirects.
      for cmd in pipeline.commands:
        if cmd.args[0] == 'opt':
          if ti.args.opt is None:
            sys.exit(ti.path + ' needs to run opt. '
                     'Please specify --llvm-bin or --opt')
          cmd.args[0] = ti.args.opt
        cmd.args = [common.applySubstitutions(i, subs) for i in cmd.args]
        for i, redirect in enumerate(cmd.redirects):
          cmd.redirects[i] = redirect[0], common.applySubstitutions(redirect[1], subs)

      # Parse executable args.
      # Execute non-clang runline.
      if clang_cmd.args[0] not in SUBST:
        # Ignore FileCheck-only 'RUN: lines'
        if pipeline.commands[0].args[0] == 'FileCheck':
          print('NOTE: Skipping FileCheck-only RUN line: ', pipeline, file=sys.stderr)
          continue
        run_list.append((None, pipeline, None))
        continue
      # This is a clang runline, apply %clang substitution rule, do lit-like substitutions,
      # and append args.clang_args
      clang_cmd.args[0:1] = SUBST[clang_cmd.args[0]]
      print(clang_cmd)
      clang_cmd.args.insert(0, ti.args.clang)
      clang_cmd.args += ti.args.clang_args
      # Remove all -verify arguments since they could cause the IR generation to fail
      clang_cmd.args = [x for x in clang_cmd.args if not x.startswith("-verify")]

      # Extract -check-prefix in FileCheck args
      filecheck_cmd = ' '.join(pipeline.commands[-1].args)
      common.verify_filecheck_prefixes(filecheck_cmd)
      if not filecheck_cmd.startswith('FileCheck ') and not filecheck_cmd.startswith('%cheri_FileCheck '):
        # Execute non-filechecked clang runline.
        print('WARNING: Executing but ignoring non-filechecked RUN line: ' + l, file=sys.stderr)
        run_list.append((None, pipeline, None))
        continue
      if '-ast-dump' in clang_cmd.args:
        print('WARNING: Executing but ignoring -ast-dump RUN line: ' + l, file=sys.stderr)
        run_list.append((None, pipeline, None))
        continue
      if '-fsyntax-only' in clang_cmd.args:
        print('WARNING: Executing but ignoring -fsyntax-only RUN line: ' + l, file=sys.stderr)
        run_list.append((None, pipeline, None))
        continue
      if '-emit-llvm' not in clang_cmd.args:
        print('WARNING: Executing but ignoring assembly output RUN line: ' + l, file=sys.stderr)
        run_list.append((None, pipeline, None))
        continue

      check_prefixes = [item for m in common.CHECK_PREFIX_RE.finditer(filecheck_cmd)
                               for item in m.group(1).split(',')]
      if not check_prefixes:
        check_prefixes = ['CHECK']
      run_list.append((check_prefixes, pipeline, triple_in_cmd))

    # Execute clang, generate LLVM IR, and extract functions.

    # Store only filechecked runlines.
    filecheck_run_list = [i for i in run_list if i[0]]
    builder = common.FunctionTestBuilder(
      run_list=filecheck_run_list,
      flags=ti.args,
      scrubber_args=[],
      path=ti.path)

    for prefixes, pipeline, triple_in_cmd in run_list:
      assert isinstance(pipeline, Pipeline)
      # Execute non-filechecked runline.
      if not prefixes:
        print('NOTE: Executing non-FileChecked RUN line: ', pipeline, file=sys.stderr)
        exec_run_line(pipeline)
        continue

      clang_cmd = pipeline.commands[0:-1]
      common.debug('Extracted clang cmd: clang {}'.format(clang_cmd))
      common.debug('Extracted FileCheck prefixes: {}'.format(prefixes))

      get_function_body(builder, ti.args, ti.path, clang_cmd, prefixes)

      # Invoke clang -Xclang -ast-dump=json to get mapping from start lines to
      # mangled names. Forward all clang args for now.
      for k, v in get_line2func_list(pipeline.commands[0]).items():
        line2func_list[k].extend(v)

    func_dict = builder.finish_and_get_func_dict()
    global_vars_seen_dict = {}
    prefix_set = set([prefix for p in filecheck_run_list for prefix in p[0]])
    output_lines = []
    has_checked_pre_function_globals = False

    include_generated_funcs = common.find_arg_in_test(ti,
                                                      lambda args: ti.args.include_generated_funcs,
                                                      '--include-generated-funcs',
                                                      True)

    if include_generated_funcs:
      # Generate the appropriate checks for each function.  We need to emit
      # these in the order according to the generated output so that CHECK-LABEL
      # works properly.  func_order provides that.

      # It turns out that when clang generates functions (for example, with
      # -fopenmp), it can sometimes cause functions to be re-ordered in the
      # output, even functions that exist in the source file.  Therefore we
      # can't insert check lines before each source function and instead have to
      # put them at the end.  So the first thing to do is dump out the source
      # lines.
      common.dump_input_lines(output_lines, ti, prefix_set, '//')

      # Now generate all the checks.
      def check_generator(my_output_lines, prefixes, func):
        if '-emit-llvm' in pipeline.commands[0].args:
          return common.add_ir_checks(my_output_lines, '//',
                                      prefixes,
                                      func_dict, func, False,
                                      ti.args.function_signature,
                                      global_vars_seen_dict,
                                      is_filtered=builder.is_filtered())
        else:
          return asm.add_checks(my_output_lines, '//',
                                prefixes,
                                func_dict, func, global_vars_seen_dict,
                                is_filtered=builder.is_filtered())

      if ti.args.check_globals:
        common.add_global_checks(builder.global_var_dict(), '//', run_list,
                                 output_lines, global_vars_seen_dict, True,
                                 True)
      common.add_checks_at_end(output_lines, filecheck_run_list, builder.func_order(),
                               '//', lambda my_output_lines, prefixes, func:
                               check_generator(my_output_lines,
                                               prefixes, func))
    else:
      # Normal mode.  Put checks before each source function.
      for line_info in ti.iterlines(output_lines):
        idx = line_info.line_number
        line = line_info.line
        args = line_info.args
        include_line = True
        m = common.CHECK_RE.match(line)
        if m and m.group(1) in prefix_set:
          continue  # Don't append the existing CHECK lines
        # Skip special separator comments added by commmon.add_global_checks.
        if line.strip() == '//' + common.SEPARATOR:
          continue
        if idx in line2func_list:
          added = set()
          for spell, mangled, search in line2func_list[idx]:
            # One line may contain multiple function declarations.
            # Skip if the mangled name has been added before.
            # The line number may come from an included file, we simply require
            # the search string (normally the function's spelling name, but is
            # the class's spelling name for class specializations) to appear on
            # the line to exclude functions from other files.
            if mangled in added or search not in line:
              continue
            if args.functions is None or any(re.search(regex, spell) for regex in args.functions):
              last_line = output_lines[-1].strip()
              while last_line == '//':
                # Remove the comment line since we will generate a new  comment
                # line as part of common.add_ir_checks()
                output_lines.pop()
                last_line = output_lines[-1].strip()
              if ti.args.check_globals and not has_checked_pre_function_globals:
                common.add_global_checks(builder.global_var_dict(), '//',
                                         run_list, output_lines,
                                         global_vars_seen_dict, True, True)
                has_checked_pre_function_globals = True
              if added:
                output_lines.append('//')
              added.add(mangled)
              common.add_ir_checks(output_lines, '//', filecheck_run_list, func_dict, mangled,
                                   False, args.function_signature, global_vars_seen_dict,
                                   is_filtered=builder.is_filtered())
              if line.rstrip('\n') == '//':
                include_line = False

        if include_line:
          output_lines.append(line.rstrip('\n'))

    if ti.args.check_globals:
      common.add_global_checks(builder.global_var_dict(), '//', run_list,
                               output_lines, global_vars_seen_dict, True, False)
    common.debug('Writing %d lines to %s...' % (len(output_lines), ti.path))
    with open(ti.path, 'wb') as f:
      f.writelines(['{}\n'.format(l).encode('utf-8') for l in output_lines])

  return 0


if __name__ == '__main__':
  sys.exit(main())
