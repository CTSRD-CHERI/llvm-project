#!/usr/bin/env python3
# PYTHON_ARGCOMPLETE_OK
import argparse
import re
import os
import tempfile
import shutil
import shlex
import subprocess
import sys
import resource
from abc import ABCMeta, abstractmethod
from enum import Enum
from pathlib import Path


try:
    from colors import blue, red, green, bold
except ImportError:
    def blue(s, bg=None, style=None):
        return s
    bold = blue
    red = blue
    green = blue

options = None  # type: Options

def verbose_print(*args, **kwargs):
    global options
    if options.verbose:
        print(*args, **kwargs)


def extremely_verbose_print(*args, **kwargs):
    global options
    if options.extremely_verbose:
        print(*args, **kwargs)


def die(*args):
    sys.exit(red(" ".join(map(str, args)), style="bold"))


def run(cmd: list, **kwargs):
    print(cmd, kwargs)
    subprocess.check_call(list(map(str, cmd)), **kwargs)


class ReduceTool(metaclass=ABCMeta):
    def __init__(self, args: "Options", name: str, tool: Path):
        self.tool = tool
        self.name = name
        self.exit_statement = ""
        self.args = args
        self.run_lines = [] # RUN: lines from the test case
        self.run_cmds = []  # the lines without RUN: suitably quoted for passing to a shell
        self.infile_name = None
        print("Reducing test case using", name)

    def _reduce_script_text(self, input_file: Path):
        result = "#!/bin/sh\n"
        for cmd in self.run_cmds:
            # check for %s should have happened earlier
            assert "%s" in cmd, cmd
            # Undo the lit substitutions
            compiler_cmd = cmd.replace("%clang_cc1 ", str(self.args.clang_cmd) + " -cc1 ")
            compiler_cmd = compiler_cmd.replace("%clang ", str(self.args.clang_cmd) + " ")
            cheri_cc1_triple = " -cc1 -triple cheri-unknown-freebsd "
            compiler_cmd = compiler_cmd.replace("%cheri_cc1 ", str(self.args.clang_cmd) + cheri_cc1_triple)
            compiler_cmd = compiler_cmd.replace("%cheri128_cc1 ", str(self.args.clang_cmd) + cheri_cc1_triple +
                                                "-target-cpu cheri128 ")
            compiler_cmd = compiler_cmd.replace("%cheri256_cc1 ", str(self.args.clang_cmd) + cheri_cc1_triple +
                                                "-target-cpu cheri256 ")
            compiler_cmd = compiler_cmd.replace("%cheri_purecap_cc1 ", str(self.args.clang_cmd) + cheri_cc1_triple +
                                                "-target-abi purecap ")
            # llc substitutions:
            if "llc" in compiler_cmd:
                compiler_cmd = re.sub(r"\bllc\b", " " + str(self.args.llc_cmd) + " ", compiler_cmd)
            compiler_cmd = compiler_cmd.replace("%cheri128_llc ", str(self.args.llc_cmd) +
                                                " -mtriple=cheri-unknown-freebsd -mcpu=cheri128")
            compiler_cmd = compiler_cmd.replace("%cheri256_llc ", str(self.args.llc_cmd) +
                                                " -mtriple=cheri-unknown-freebsd -mcpu=cheri256")
            compiler_cmd = compiler_cmd.replace("%cheri_llc ", str(self.args.llc_cmd) +
                                                " -mtriple=cheri-unknown-freebsd")
            # opt substitutions
            if "opt" in compiler_cmd:
                compiler_cmd = re.sub(r"\opt\b", " " + str(self.args.opt_cmd) + " ", compiler_cmd)
            compiler_cmd = compiler_cmd.replace("%cheri_opt ", str(self.args.opt_cmd) +
                                                " -mtriple=cheri-unknown-freebsd")

            # ignore all the piping to FileCheck parts of the command
            if "|" in compiler_cmd:
                compiler_cmd = compiler_cmd[0:compiler_cmd.find("|")]
            compiler_cmd = compiler_cmd.replace("%s", self.input_file_arg(input_file))
            grep_msg = ""
            crash_flag = "--crash"
            if self.args.llvm_error:
                grep_msg += "2>&1 | grep 'LLVM ERROR:' "
                crash_flag = ""  # it doesn't actually crash now but returns 1
                if not self.args.crash_message:
                    self.args.crash_message = "Cannot select"
            if self.args.crash_message:
                grep_msg += "2>&1 | grep " + shlex.quote(self.args.crash_message)
            # exit once the first command crashes
            result += "{not_cmd} {crash_flag} {command} {grep_msg} {exit_stmt}\n".format(
                not_cmd=self.args.not_cmd, crash_flag=crash_flag,
                command=compiler_cmd, grep_msg=grep_msg,
                exit_stmt=self.exit_statement)
        return result

    def _create_reduce_script(self, tmpdir: Path, input_file: Path):
        reduce_script = Path(tmpdir, "reduce_script.sh").absolute()
        reduce_script_text = self._reduce_script_text(input_file)
        reduce_script.write_text(reduce_script_text)
        print("Reduce script:\n", bold(reduce_script_text), sep="")
        reduce_script.chmod(0o755)
        if not self.is_reduce_script_interesting(reduce_script, input_file):
            die("Reduce script is not interesting!")
        return reduce_script

    def create_test_case(self, input_text: str, test_case: Path):
        processed_run_lines = []
        # TODO: try to remove more flags from the RUN: line!
        for run_line in self.run_lines:
            # convert %clang_cc1 -target-cpu cheri to %cheri_cc1 / %cheri_purecap_cc1
            run_line = run_line.replace("-Werror=implicit-int", "")  # important for creduce but not for the test
            if "%clang_cc1" in run_line:
                target_cpu_re = r"-target-cpu\s+cheri[^\s]*\s*"
                triple_cheri_freebsd_re = r"-triple\s+cheri-unknown-freebsd\d*\s+"
                if re.search(target_cpu_re, run_line) or re.search(triple_cheri_freebsd_re, run_line):
                    run_line = re.sub(target_cpu_re, "", run_line)  # remove
                    run_line = re.sub(triple_cheri_freebsd_re, "", run_line)  # remove
                    run_line = run_line.replace("%clang_cc1", "%cheri_cc1")
                    run_line = run_line.replace("-mllvm -cheri128", "")
                    target_abi_re = r"-target-abi\s+purecap\s*"
                    if re.search(target_abi_re, run_line) is not None:
                        run_line = re.sub(target_abi_re, "", run_line)  # remove
                        assert "%cheri_cc1" in run_line
                        run_line = run_line.replace("%cheri_cc1", "%cheri_purecap_cc1")
            processed_run_lines.append(run_line)
        result = "\n".join(processed_run_lines) + "\n" + input_text
        with test_case.open("w", encoding="utf-8") as f:
            f.write(result)
            f.flush()
        print("\nResulting test case ", test_case, sep="")
        verbose_print(result)

    def preprocess(self, input_file: Path) -> Path:
        return input_file

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        return False

    @abstractmethod
    def reduce(self, input_file: Path, extra_args: list, tempdir: Path):
        raise NotImplemented()

    @abstractmethod
    def input_file_arg(self, input_file: Path) -> str:
        raise NotImplemented()


class RunBugpoint(ReduceTool):
    def __init__(self, args: "Options"):
        super().__init__(args, "bugpoint", tool=args.bugpoint_cmd)
        # bugpoint wants a non-zero exit code on failure
        self.exit_statement = "&& exit 1; exit 0"

    def reduce(self, input_file, extra_args, tempdir):
        bugpoint = [self.tool, "-opt-command=" + str(self.args.opt_cmd), "-output-prefix=" + input_file.name]
        if self.args.verbose:
            bugpoint.append("-verbose-errors")
        expected_output_file = Path.cwd() / (input_file.name + "-reduced-simplified.bc")
        if expected_output_file.exists():
            print("bugpoint output file already exists: ", bold(expected_output_file))
            if input("Delete it and continue? [Y/n]").lower().startswith("n"):
                die("Can't continue")
            else:
                expected_output_file.unlink()

        # use a custom script to check for matching crash message:
        if self.args.crash_message:
            # check that the reduce script is interesting:
            # http://blog.llvm.org/2015/11/reduce-your-testcases-with-bugpoint-and.html
            # ./bin/bugpoint -compile-custom -compile-command=./check.sh -opt-command=./bin/opt my_test_case.ll
            reduce_script = self._create_reduce_script(tempdir, input_file.absolute())
            print("Checking whether reduce script works")
            test_result = subprocess.run([str(reduce_script.absolute()), str(input_file)])
            if test_result.returncode == 0:
                die("Interestingness test failed for bugpoint. Does the command really crash? Script was",
                    reduce_script.read_text())
            bugpoint += ["-compile-custom", "-compile-command=" + str(reduce_script.absolute()), input_file]
        else:
            bugpoint += ["-run-llc-ia", input_file]
            tool_args = shlex.split(self.run_cmds[0])[1:]
            # filter the tool args
            bugpoint += ["--tool-args", "--"]
            skip_next = False
            for arg in tool_args:
                if skip_next:
                    skip_next = False
                    continue
                elif "%s" in arg:
                    continue
                elif arg.strip() == "-o":
                    skip_next = True
                    continue
                else:
                    bugpoint.append(arg)
        bugpoint += extra_args
        print("About to run", bugpoint)
        print("Working directory:", os.getcwd())
        try:
            env = os.environ.copy()
            env["PATH"] = str(self.args.bindir) + ":" + env["PATH"]
            try:
                run(bugpoint, env=env)
            except KeyboardInterrupt:
                print(red("\nCTRL+C detected, stopping bugpoint.", style="bold"))
        finally:
            print("Output files are in:", os.getcwd())
            # TODO: generate a test case from the output files?
        if expected_output_file.exists():
            print("Attempting to convert generated bitcode file to a test case...")
            dis = subprocess.run([str(self.args.llvm_dis_cmd), "-o", "-", str(expected_output_file)], stdout=subprocess.PIPE)
            self.create_test_case(dis.stdout.decode("utf-8"), input_file.with_suffix(".test" + input_file.suffix))

    def input_file_arg(self, input_file: Path):
        # bugpoint expects a script that takes the input files as arguments:
        return '"$@"'

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        proc = subprocess.run([str(reduce_script), str(input_file)])
        return proc.returncode != 0


class RunCreduce(ReduceTool):
    def __init__(self, args: "Options"):
        super().__init__(args, "creduce", tool=args.creduce_cmd)
        self.exit_statement = "&& exit 0"

    def preprocess(self, input_file: Path):
        # creduce wastes a lot of time trying to remove #includes and dead cases generated by -frewrite-includes
        if self.args.no_initial_reduce:
            return input_file
        with tempfile.NamedTemporaryFile() as tmp:
            shutil.copy(str(input_file), tmp.name)
            original_size = input_file.stat().st_size
            with input_file.open("w", encoding="utf-8") as reduced_file:
                self._do_initial_reduce(Path(tmp.name), reduced_file)
            new_size = input_file.stat().st_size
            percent_reduction = 100 - 100.0 * (new_size / original_size)
            print("Intial preprocessing: {} bytes -> {} bytes ({}% reduction)".format(
                  original_size, new_size, percent_reduction))
        return input_file

    def reduce(self, input_file: Path, extra_args, tempdir):
        reduce_script = self._create_reduce_script(tempdir, input_file.absolute())
        creduce = ["time", str(self.tool), str(reduce_script), str(input_file), "--timing"] + extra_args
        # This is way too verbose
        if self.args.extremely_verbose:
            creduce.append("--print-diff")
        print("About to run", creduce)
        try:
            run(creduce, cwd=tempdir)
        except KeyboardInterrupt:
            print(red("\nCTRL+C detected, stopping creduce.", style="bold"))
        # write the output test file:
        print("\nDONE!")
        self.create_test_case(input_file.read_text(encoding="utf-8"),
                              input_file.with_suffix(".test" + input_file.suffix))

    def input_file_arg(self, input_file: Path):
        # creduce creates an input file in the test directory with the same name as the original input
        return input_file.name

    def _do_initial_reduce(self, input_path, out_file):
        # The initial remove #includes pass takes a long time -> remove all the includes that are inside a #if 0
        # This is especially true for C++ because there are so many #included files in preprocessed input
        with input_path.open("r", errors="replace", encoding="utf-8") as input_file:
            # line_regex = re.compile(r'^#\s+\d+\s+".*".*')
            start_rewrite_includes = re.compile(r"^\s*#if\s+0\s+/\* expanded by -frewrite-includes \*/\s*")
            end_rewrite_includes = re.compile(r"^\s*#endif\s+/\* expanded by -frewrite-includes \*/\s*")
            in_rewrite_includes = False
            max_rewrite_includes_lines = 10
            skipped_rewrite_includes = 0
            for line in input_file.readlines():
                if re.match(start_rewrite_includes, line):
                    extremely_verbose_print("Starting -frewrite-includes-block:", line.rstrip())
                    assert not in_rewrite_includes
                    assert skipped_rewrite_includes == 0
                    in_rewrite_includes = True
                    continue
                elif re.match(end_rewrite_includes, line):
                    extremely_verbose_print("Ending -frewrite-includes-block, skipped", skipped_rewrite_includes, "lines")
                    assert in_rewrite_includes
                    in_rewrite_includes = False
                    skipped_rewrite_includes = 0
                    continue
                elif in_rewrite_includes:
                    if skipped_rewrite_includes > max_rewrite_includes_lines:
                        die("Error in initial reduction, rerun with --no-initial-reduce")
                    extremely_verbose_print("Skipping line inside -frewrite-includes:", line.rstrip())
                    skipped_rewrite_includes += 1
                    continue
                elif line.lstrip().startswith("//"):
                    continue  # skip line comments
                # This appears to break creduce sometimes:
                # elif re.match(line_regex, line):
                #     extremely_verbose_print("Removing # line directive:", line.rstrip())
                #     continue
                else:
                    out_file.write(line)
            out_file.flush()
        if self.args.extremely_verbose:
            verbose_print("Initial reduction:")
            subprocess.call(["diff", "-u", str(input_path), out_file.name])
            pass

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        if self.args.verbose:
            return self.__is_reduce_script_interesting(reduce_script, input_file)
        else:
            return True  # creduce checks anyway, this just wastes time

    @staticmethod
    def __is_reduce_script_interesting(reduce_script: Path, input_file: Path) -> bool:
        with tempfile.TemporaryDirectory() as tmpdir:
            shutil.copy(str(input_file), str(Path(tmpdir, input_file.name)))
            proc = subprocess.run([str(reduce_script), str(input_file)], cwd=tmpdir)
            return proc.returncode == 0


class SkipReducing(ReduceTool):
    def __init__(self, args: "Options"):
        super().__init__(args, "noop", tool=Path("/dev/null"))

    def reduce(self, input_file, extra_args, tempdir):
        self.create_test_case("Some strange reduced test case\n", input_file.with_suffix(".test" + input_file.suffix))

    def input_file_arg(self, input_file: Path) -> str:
        raise NotImplemented()

class Options(object):
    def __init__(self, args: argparse.Namespace):
        self.verbose = args.verbose
        self.extremely_verbose = args.extremely_verbose
        self.bindir = Path(args.bindir)
        self.args = args
        self.no_initial_reduce = args.no_initial_reduce  # type: bool
        self.crash_message = args.crash_message  # type: str
        self.llvm_error = args.llvm_error  # type: bool

    @property
    def clang_cmd(self):
        return self._get_command("clang")

    @property
    def opt_cmd(self):
        return self._get_command("opt")

    @property
    def not_cmd(self):
        return self._get_command("not")

    @property
    def llc_cmd(self):
        return self._get_command("llc")

    @property
    def llvm_dis_cmd(self):
        return self._get_command("llvm-dis")

    @property
    def bugpoint_cmd(self):
        return self._get_command("bugpoint")

    @property
    def creduce_cmd(self):
        creduce_path = self.args.creduce_cmd or shutil.which("creduce")
        if not creduce_path:
            die("Could not find `creduce` in $PATH. Add it to $PATH or pass --creduce-cmd")
        return Path(creduce_path)

    def _get_command(self, name):
        result = Path(getattr(self.args, name + "_cmd", None) or Path(self.bindir, name))
        if not result.exists():
            die("Invalid `" + name + "` binary`", result)
        return result


class Reducer(object):
    def __init__(self, parser: argparse.ArgumentParser):
        self.args, self.reduce_args = parser.parse_known_args()
        if self.args.extremely_verbose:
            self.args.verbose = True
        global options
        options = Options(self.args)
        self.options = options
        self.testcase = Path(self.args.testcase)
        self.run_lines = [] # RUN: lines from the test case
        self.run_cmds = []  # the lines without RUN: suitably quoted for passing to a shell
        self.reduce_tool = None  # type: ReduceTool

    # returns the real input file
    def parse_RUN_lines(self, infile: Path) -> Path:
        is_crash_reproducer = infile.suffix == ".sh"
        if is_crash_reproducer:
            verbose_print("Input file is a crash reproducer script")
        verbose_print("Finding test command(s) in", infile)
        with infile.open("r", errors="replace", encoding="utf-8") as f:
            if is_crash_reproducer:
                real_infile = self._parse_crash_reproducer(infile, f)
            else:
                real_infile = infile
                self._parse_test_case(f)
        if len(self.run_cmds) < 1:
            die("Could not find any RUN: lines in", infile)
        return real_infile

    def _parse_crash_reproducer(self, infile, f) -> Path:
        real_in_file = None
        for line in f.readlines():
            if line.strip().startswith("#"):
                continue
            command = shlex.split(line)
            if "clang" not in command[0]:
                die("Executed program should contain 'clang', but was", command[0])
            source_file_name = command[-1]
            source_file = infile.with_name(source_file_name)
            if not source_file.exists():
                die("Reproducer input file", source_file, "does not exist!")
            real_in_file = source_file
            verbose_print("Real input file is", real_in_file)
            command[-1] = "%s"
            # try to remove all unnecessary command line arguments
            command, real_in_file = self._simplify_crash_command(command, real_in_file.absolute())
            command.append("%s")
            compiler_cmd = " ".join(shlex.quote(s) for s in command)
            verbose_print("Test command is", bold(compiler_cmd))
            self.run_cmds.append(compiler_cmd)
            if real_in_file.suffix == ".ll":
                comment_start = ";"
            elif real_in_file.suffix in (".S", ".s"):
                comment_start = "#"
            else:
                comment_start = "//"
            self.run_lines.append(comment_start + " RUN: " + compiler_cmd)
        if not real_in_file:
            die("Could not compute input file for crash reproducer")
        return real_in_file

    def _check_crash(self, command, infile, proc_info: dict=None):
        full_cmd = [str(self.options.not_cmd), "--crash"] + command + [str(infile)]
        verbose_print(blue("\nRunning" + " ".join(map(shlex.quote, full_cmd))))
        if self.args.reduce_tool == "noop":
            if proc_info is not None:
                proc_info["stderr"] = b"Assertion `noop' failed."
            print(green(" yes"))
            return True
        proc = subprocess.run(full_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        # treat fatal llvm errors (cannot select, etc) as crashes too:
        is_llvm_error = b"LLVM ERROR:" in proc.stderr
        if proc_info is not None:  # To get the initial error message:
            proc_info["stdout"] = proc.stdout
            proc_info["stderr"] = proc.stderr
            proc_info["returncode"] = proc.returncode
        if proc.returncode == 0 or is_llvm_error:
            if not self.options.crash_message or (self.options.crash_message in proc.stderr.decode("utf-8")):
                print(green(" yes"))
                return True
            else:
                print(red(" yes, but with a different crash message!"))
                verbose_print("Expected crash message '", bold(self.options.crash_message), "' not found in:\n",
                              proc.stderr.decode("utf-8"), sep="")
                return False
        print(red(" no"))
        return False

    @staticmethod
    def _filter_args(args, *, noargs_opts_to_remove=list(), noargs_opts_to_remove_startswith=list(),
                     one_arg_opts_to_remove=list(), one_arg_opts_to_remove_if=dict()):
        result = []
        skip_next = False

        def should_remove_arg(option, value):
            for a, predicate in one_arg_opts_to_remove_if.items():
                if option == a:
                    print("Testing predicate", predicate, "for arg", option, "on", value)
                    if predicate(value):
                        return True
            return False

        for i, arg in enumerate(args):
            if skip_next:
                skip_next = False
                continue
            if any(arg == a for a in noargs_opts_to_remove) or any(arg.startswith(a) for a in noargs_opts_to_remove_startswith):
                continue
            if any(arg == a for a in one_arg_opts_to_remove):
                skip_next = True
                continue
            if (i + 1) < len(args) and should_remove_arg(arg, args[i + 1]):
                skip_next = True
                continue
            # none of the filters matches -> append to the result
            result.append(arg)
        return result

    def _try_remove_args(self, command: list, infile: Path, message: str, **kwargs):
        new_command = self._filter_args(command, **kwargs)
        print(message, end="", flush=True)
        if self._check_crash(new_command, infile):
            return new_command
        return command

    @staticmethod
    def _infer_crash_message(stderr: bytes):
        if not stderr:
            return None
        regexes = [re.compile(s) for s in (
            r"Assertion `(.+)' failed.",  # Linux assert()
            r"Assertion failed: \(.+\),",  # FreeBSD/Mac assert()
            r"UNREACHABLE executed( at .+)?!",  # llvm_unreachable()
            # generic code gen crashes (at least creduce will keep the function name):
            r"LLVM IR generation of declaration '(.+)'",
            r"Generating code for declaration '(.+)'",
            # error in backend:
            r"fatal error: error in backend:(.+)"
        )]
        for line in stderr.decode("utf-8").splitlines():
            # Check for failed assertions:
            for r in regexes:
                match = r.search(line)
                if match:
                    return match.group(0)
        return None

    def _simplify_crash_command(self, command: list, infile: Path) -> tuple:
        new_command = command.copy()
        new_command[0] = str(self.options.clang_cmd)
        assert new_command[-1] == "%s"
        del new_command[-1]
        # output to stdout
        if "-o" not in new_command:
            print("Adding '-o -' to the compiler invocation")
            new_command += ["-o", "-"]
        full_cmd = new_command.copy()
        print("Checking whether reproducer crashes with ", self.options.clang_cmd, ":", sep="", end="", flush=True)
        crash_info = dict()
        if not self._check_crash(new_command, infile, crash_info):
            die("Crash reproducer no longer crashes?")

        if not self.options.crash_message:
            print("Attempting to infer crash message from process output")
            inferred_msg = self._infer_crash_message(crash_info["stderr"])
            if inferred_msg:
                print("Inferred crash message as '" + green(inferred_msg) + "'")
                if not input("Use this message? [Y/n]").lower().startswith("n"):
                    self.options.crash_message = inferred_msg
            else:
                print("Could not infer crash message, stderr was:\n\n")
                print(crash_info["stderr"].decode("utf-8"))
                print("\n\n")
        if not self.options.crash_message:
            print("Could not infer crash message from crash reproducer.")
            print(red("WARNING: Reducing without specifying the crash message will probably result"
                      " in the wrong test case being generated."))
            if not input("Are you sure you want to continue? [y/N]").lower().startswith("y"):
                sys.exit()
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -coverage-notes-file crashes:",
            one_arg_opts_to_remove=["-coverage-notes-file"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without debug info crashes:",
            noargs_opts_to_remove=["-dwarf-column-info"],
            noargs_opts_to_remove_startswith=["-debug-info-kind=", "-dwarf-version=", "-debugger-tuning="],
        )
        # try emitting llvm-ir (i.e. frontend bug):
        print("Checking whether -O0 -emit-llvm crashes:", end="", flush=True)
        generate_ir_cmd = new_command + ["-O0", "-emit-llvm"]
        if "-emit-obj" in generate_ir_cmd:
            generate_ir_cmd.remove("-emit-obj")
        if self._check_crash(generate_ir_cmd, infile):
            new_command = generate_ir_cmd
            print("Must be a", blue("frontend crash.", style="bold"), "Will need to use creduce for test case reduction")
            return self._simplify_frontend_crash_cmd(new_command, infile)
        else:
            print("Must be a ", blue("backend crash", style="bold"), ", ", end="", sep="")
            if self.args.reduce_tool == "creduce":
                print("but reducing with creduce requested. Will not try to convert to a bugpoint test case")
                return self._simplify_frontend_crash_cmd(new_command, infile)
            else:
                print("will try to use bugpoint.")
                return self._simplify_backend_crash_cmd(new_command, infile, full_cmd)

    def _simplify_frontend_crash_cmd(self, new_command: list, infile: Path):
        print("Checking whether compiling without warnings crashes:", end="", flush=True)
        no_warnings_cmd = self._filter_args(new_command, noargs_opts_to_remove=["-w"],
                                            noargs_opts_to_remove_startswith=["-W"])
        no_warnings_cmd.append("-w")  # disable all warnigns
        no_warnings_cmd.append("-Werror=implicit-int")
        if self._check_crash(no_warnings_cmd, infile):
            new_command = no_warnings_cmd[:-2]
        new_command.append("-Werror=implicit-int")

        # check if floating point args are relevant
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without floating point arguments crashes:",
            noargs_opts_to_remove=["-mdisable-fp-elim", "-msoft-float"],
            one_arg_opts_to_remove=["-mfloat-abi"],
            one_arg_opts_to_remove_if={"-target-feature": lambda a: a == "+soft-float"}
        )

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without PIC flags crashes:",
            one_arg_opts_to_remove=["-mrelocation-model", "-pic-level"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without thread model flags crashes:",
            one_arg_opts_to_remove=["-mthread-model"],
            noargs_opts_to_remove_startswith=["-ftls-model=initial-exec"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -target-feature flags crashes:",
            one_arg_opts_to_remove=["-target-feature"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without various MIPS flags crashes:",
            noargs_opts_to_remove=["-cheri-linker"],
            one_arg_opts_to_remove_if={"-mllvm": lambda a: a.startswith("-mips-ssection-threshold=") or a == "-mxgot"}
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -mrelax-all crashes:",
            noargs_opts_to_remove=["-mrelax-all"],
        )

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -D flags crashes:",
            noargs_opts_to_remove=["-sys-header-deps"],
            one_arg_opts_to_remove=["-D"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -x flag crashes:",
            one_arg_opts_to_remove=["-x"]
        )

        if "-disable-llvm-verifier" in new_command:
            new_command = self._try_remove_args(
                new_command, infile, "Checking whether compiling without -disable-llvm-verifier crashes:",
                noargs_opts_to_remove=["-disable-llvm-verifier"])

        # try to remove some arguments that should not be needed
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether misc diagnostic options can be removed:",
            noargs_opts_to_remove=["-disable-free", "-discard-value-names", "-masm-verbose",
                                   "-mconstructor-aliases"],
            noargs_opts_to_remove_startswith=["-fdiagnostics-", "-fobjc-runtime="],
            one_arg_opts_to_remove=["-main-file-name", "-ferror-limit", "-fmessage-length"]
        )

        # TODO: try removing individual -mllvm options such as mxgot, etc.?
        # add the placeholders for the RUN: line
        new_command[0] = "%clang"
        if new_command[1] == "-cc1":
            del new_command[1]
            new_command[0] = "%clang_cc1"
        return new_command, infile

    def _simplify_backend_crash_cmd(self, new_command: list, infile: Path, full_cmd: list):
        # TODO: convert it to a llc commandline and use bugpoint
        assert "-emit-llvm" not in full_cmd
        assert "-o" in full_cmd
        command = full_cmd.copy()
        irfile = infile.with_name(infile.name.partition(".")[0] + "-bugpoint.ll")
        command[command.index("-o") + 1] = str(irfile.absolute())
        if "-discard-value-names" in command:
            command.remove("-discard-value-names")
        command.append("-emit-llvm")
        print("Generating IR file", irfile)
        try:
            verbose_print(command + ["-O0", str(infile)])
            subprocess.check_call(command + ["-O0", str(infile)])
        except subprocess.CalledProcessError:
            print("Failed to generate IR from", infile, "will have to reduce using creduce")
            return self._simplify_frontend_crash_cmd(new_command, infile)
        if not irfile.exists():
            die("IR file was not generated?")
        llc_args = [str(self.options.llc_cmd), "-O3", "-o", "/dev/null"]  # TODO: -o -?
        cpu_flag = None  # -mcpu= only allowed once!
        pass_once_flags = set()
        skip_next = False
        for i, arg in enumerate(command):
            if skip_next:
                skip_next = False
                continue
            if arg == "-triple" or arg == "-target":
                # assume well formed command line
                llc_args.append("-mtriple=" + command[i + 1])
            # forward all the llvm args
            elif arg == "-mllvm":
                llvm_flag = command[i + 1]
                if llvm_flag == "-cheri128":
                    cpu_flag = "-mcpu=cheri128"
                    llc_args.append("-mattr=+cheri128")
                else:
                    pass_once_flags.add(llvm_flag)
                skip_next = True
            elif arg == "-target-abi":
                llc_args.append("-target-abi")
                llc_args.append(command[i + 1])
                skip_next = True
            elif arg == "-target-cpu":
                cpu_flag = "-mcpu=" + command[i + 1]
                skip_next = True
            elif arg == "-target-feature":
                llc_args.append("-mattr=" + command[i + 1])
                skip_next = True
            elif arg == "-mrelocation-model":
                llc_args.append("-relocation-model=" + command[i + 1])
                skip_next = True
            elif arg == "-mthread-model":
                llc_args.append("-thread-model=" + command[i + 1])
                skip_next = True
            elif arg == "-msoft-float":
                llc_args.append("-float-abi=soft")
            elif arg.startswith("-vectorize"):
                llc_args.append(arg)
            elif arg == "-mxgot":
                pass_once_flags.add(arg)  # some bugs only happen if mxgot is also passed
        if cpu_flag:
            llc_args.append(cpu_flag)
        llc_args.extend(pass_once_flags)
        print("Checking whether compiling IR file with llc crashes:", end="", flush=True)
        llc_info = dict()
        if self._check_crash(llc_args, irfile, llc_info):
            print("Crash found with llc -> using bugpoint which is faster than creduce.")
            self.reduce_tool = RunBugpoint(self.options)
            llc_args[0] = "llc"
            return llc_args, irfile
        print("Compiling IR file with llc did not reproduce crash. Stderr was:", llc_info["stderr"].decode("utf-8"))
        print("Checking whether compiling IR file with opt crashes:", end="", flush=True)
        opt_args = llc_args.copy()
        opt_args[0] = str(self.options.opt_cmd)
        # -O flag can only be passed once and is already included in llc_args
        assert "-O3" in opt_args, opt_args
        opt_args.append("-S")
        opt_info = dict()
        if self._check_crash(opt_args, irfile, opt_info):
            print("Crash found with LLC -> using bugpoint which is faster than creduce.")
            self.reduce_tool = RunBugpoint(self.options)
            opt_args[0] = "opt"
            return opt_args, irfile
        print("Compiling IR file with opt did not reproduce crash. Stderr was:", opt_info["stderr"].decode("utf-8"))

        print("Checking whether compiling IR file with clang crashes:", end="", flush=True)
        clang_info = dict()
        bugpoint_clang_cmd = self._filter_args(full_cmd, noargs_opts_to_remove_startswith=["-xc", "-W", "-std="],
                                               one_arg_opts_to_remove=["-D", "-x", "-main-file-name"])
        bugpoint_clang_cmd.extend(["-x", "ir"])
        if self._check_crash(bugpoint_clang_cmd, irfile, clang_info):
            print("Crash found compiling IR with clang -> using bugpoint which is faster than creduce.")
            self.reduce_tool = RunBugpoint(self.options)
            full_cmd[0] = "%clang"
            return bugpoint_clang_cmd, irfile
        print("Compiling IR file with clang did not reproduce crash. Stderr was:", clang_info["stderr"].decode("utf-8"))
        print(red("No crash found compiling the IR! Possibly crash only happens when invoking clang -> using creduce."))
        self.reduce_tool = RunCreduce(self.options)
        return self._simplify_frontend_crash_cmd(new_command, infile)

    def _parse_test_case(self, f):
        # test case: just search for RUN: lines
        for line in f.readlines():
            match = re.match(r".*\s+RUN: (.+)", line)
            if match:
                self.run_lines.append(line)
                command = match.group(1).strip()
                self.run_cmds.append(command)
                if "%s" not in command:
                    die("RUN: line does not contain %s -> cannot create replacement invocation")

    def run(self):
        # scan test case for RUN: lines
        infile = self.parse_RUN_lines(self.testcase)

        if self.reduce_tool is None:
            if self.args.reduce_tool is None:
                self.args.reduce_tool = "bugpoint" if infile.suffix in (".ll", ".bc") else "creduce"
            if self.args.reduce_tool == "bugpoint":
                self.reduce_tool = RunBugpoint(self.options)
            elif self.args.reduce_tool == "noop":  # for debugging purposes
                self.reduce_tool = SkipReducing(self.options)
            else:
                assert self.args.reduce_tool == "creduce"
                self.reduce_tool = RunCreduce(self.options)

        # TODO: move this to ctor
        self.reduce_tool.run_cmds = self.run_cmds
        self.reduce_tool.run_lines = self.run_lines

        if self.args.output_file:
            reduce_input = Path(self.args.output_file).absolute()
        else:
            reduce_input = infile.with_name(infile.name.partition(".")[0] + "-reduce" + infile.suffix).absolute()
        shutil.copy(str(infile), str(reduce_input))
        reduce_input = self.reduce_tool.preprocess(reduce_input)
        with tempfile.TemporaryDirectory() as tmpdir:
            # run("ulimit -S -c 0".split())
            self.reduce_tool.reduce(input_file=reduce_input, extra_args=self.reduce_args, tempdir=tmpdir)

def main():
    default_bindir = "@CMAKE_BINARY_DIR@/bin"
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("--bindir", default=default_bindir,
                        help="Path to clang build directory. Default is " + default_bindir)
    parser.add_argument("--not-cmd", help="Path to `not` tool. Default is $BINDIR/not")
    parser.add_argument("--clang-cmd", help="Path to `clang` tool. Default is $BINDIR/clang")
    parser.add_argument("--llc-cmd", help="Path to `llc` tool. Default is $BINDIR/llc")
    parser.add_argument("--opt-cmd", help="Path to `opt` tool. Default is $BINDIR/opt")
    parser.add_argument("--llvm-dis-cmd", help="Path to `llvm-dis` tool. Default is $BINDIR/llvm-dis")
    parser.add_argument("--bugpoint-cmd", help="Path to `bugpoint` tool. Default is $BINDIR/bugpoint")
    parser.add_argument("--creduce-cmd", help="Path to `creduce` tool. Default is `creduce`")
    parser.add_argument("--output-file", help="The name of the output file")
    parser.add_argument("--verbose", action="store_true", help="Print more debug output")
    parser.add_argument("--extremely-verbose", action="store_true", help="Print tons of debug output")
    parser.add_argument("--llvm-error", action="store_true", help="Reduce a LLVM ERROR: message instead of a crash")
    # TODO: infer this automatically from the crash reproducer?
    parser.add_argument("--crash-message", help="If set the crash must contain this message to be accepted for reduction."
                                                " This is useful if creduce ends up generating another crash bug that is not the one being debugged.")
    parser.add_argument("--reduce-tool", help="The tool to use for test case reduction. "
                                              "Defaults to `bugpoint` if input file is a .ll or .bc file and `creduce` otherwise.",
                        choices=["bugpoint", "creduce", "noop"])
    parser.add_argument("--no-initial-reduce", help="Pass the original input file to creduce without "
                        "removing #if 0 regions. Generally this will speed up but in very rare corner "
                        "cases it might cause the test case to no longer crash.", action="store_true")
    parser.add_argument("testcase", help="The file to reduce (must be a testcase with a RUN: line that crashes or a .sh file from a clang crash")
    # bash completion for arguments:
    try:
        import argcomplete
        argcomplete.autocomplete(parser)
    except ImportError:
        pass
    # Disable coredumps to avoid filling up disk space:
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    Reducer(parser).run()


if __name__ == "__main__":
    main()
