#!/usr/bin/env python3
# PYTHON_ARGCOMPLETE_OK
import argparse
import datetime
import re
import os
import tempfile
import shutil
import shlex
import subprocess
import sys
import resource
import typing
from abc import ABCMeta, abstractmethod
from enum import Enum
from pathlib import Path

lit_path = Path(__file__).parent.parent.parent / "llvm/utils/lit"
if not lit_path.exists():
    sys.exit("Cannot find lit in expected path " + str(lit_path))
sys.path.insert(1, str(lit_path))
from lit.llvm.config import LLVMConfig, FindTool, ToolSubst
import lit.TestRunner

try:
    from colors import blue, red, green, bold
except ImportError:
    print("Install the ansicolors package for coloured output.")

    # noinspection PyUnusedLocal
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


def quote_cmd(cmd: typing.List[str]):
    return " ".join(shlex.quote(s) for s in cmd)


def die(*args):
    sys.exit(red(" ".join(map(str, args)), style="bold"))


def run(cmd: list, **kwargs):
    print(cmd, kwargs)
    subprocess.check_call(list(map(str, cmd)), **kwargs)


class ErrorKind(Enum):
    CRASH = tuple()
    INFINITE_LOOP = (b"INFINITE LOOP:",)
    FATAL_ERROR = (b"fatal error:", b"LLVM ERROR:", b"*** Bad machine code:")
    AddressSanitizer_ERROR = (b"ERROR: AddressSanitizer:",)


class LitSubstitutionHandler(object):
    # noinspection PyMethodMayBeStatic
    class _FakeLitConfig(object):
        def __init__(self, _: "Options"):
            self.params = dict()
            self.quiet = True

        def note(self, msg):
            print(blue(msg))

        def fatal(self, msg):
            sys.exit(msg)

    class _FakeLitParams(object):
        def __init__(self, args: "Options"):
            self.available_features = set()
            self.substitutions = []
            self.llvm_tools_dir = str(args.bindir)
            self.environment = os.environ.copy()
            self.name = "reduce-crash"
            # Don't matter but are needed for clang substitutions
            self.target_triple = "x86_64-unknown-linux-gnu"
            self.host_triple = "x86_64-unknown-linux-gnu"

    def __init__(self, args: "Options"):
        llvm_config = LLVMConfig(LitSubstitutionHandler._FakeLitConfig(args),
                                 LitSubstitutionHandler._FakeLitParams(args))
        llvm_config.use_default_substitutions()
        # Not really required but makes debugging tests easier
        llvm_config.use_clang()
        llvm_config.add_cheri_tool_substitutions(["llc", "opt", "llvm-mc"])
        llvm_tools = [
            'dsymutil', 'lli', 'lli-child-target', 'llvm-ar', 'llvm-as',
            'llvm-bcanalyzer', 'llvm-config', 'llvm-cov', 'llvm-cxxdump', 'llvm-cvtres',
            'llvm-diff', 'llvm-dis', 'llvm-dwarfdump', 'llvm-exegesis', 'llvm-extract',
            'llvm-isel-fuzzer', 'llvm-ifs', 'llvm-install-name-tool',
            'llvm-jitlink', 'llvm-opt-fuzzer', 'llvm-lib',
            'llvm-link', 'llvm-lto', 'llvm-lto2', 'llvm-mc', 'llvm-mca',
            'llvm-modextract', 'llvm-nm', 'llvm-objcopy', 'llvm-objdump',
            'llvm-pdbutil', 'llvm-profdata', 'llvm-ranlib', 'llvm-rc', 'llvm-readelf',
            'llvm-readobj', 'llvm-rtdyld', 'llvm-size', 'llvm-split', 'llvm-strings',
            'llvm-strip', 'llvm-tblgen', 'llvm-undname', 'llvm-c-test', 'llvm-cxxfilt',
            'llvm-xray', 'yaml2obj', 'obj2yaml', 'yaml-bench', 'verify-uselistorder',
            'bugpoint', 'llc', 'llvm-symbolizer', 'opt', 'sancov', 'sanstats'
        ]
        llvm_config.add_tool_substitutions(llvm_tools)
        self.substitutions = llvm_config.config.substitutions
        if args.extremely_verbose:
            import pprint
            pprint.pprint(self.substitutions)

    def expand_lit_subtitutions(self, cmd: str) -> str:
        result = lit.TestRunner.applySubstitutions([cmd], self.substitutions)
        assert len(result) == 1
        print(blue(cmd), "->", red(result))
        return result[0]

    # TODO: reverse apply:


def add_lit_substitutions(args: "Options", run_line: str) -> str:
    for path, replacement in ((args.clang_cmd, "%clang"), (args.opt_cmd, "opt"), (args.llc_cmd, "llc")):
        if str(path) in run_line:
            run_line = run_line.replace(str(path), replacement)
            break
    run_line = re.sub(r"%clang\s+-cc1", "%clang_cc1", run_line)
    # convert %clang_cc1 -target-cpu cheri to %cheri_cc1 / %cheri_purecap_cc1
    run_line = run_line.replace("-Werror=implicit-int", "")  # important for creduce but not for the test
    if "%clang_cc1" in run_line:
        target_cpu_re = r"-target-cpu\s+cheri[^\s]*\s*"
        triple_cheri_freebsd_re = re.compile(
            r"-triple\s+((?:cheri|mips64c128|mips64c256)-unknown-freebsd\d*(-purecap)?)*\s+")
        found_cheri_triple = None
        triple_match = re.search(triple_cheri_freebsd_re, run_line)
        print(triple_match)
        if triple_match:
            found_cheri_triple = triple_match.group(1)
        if re.search(target_cpu_re, run_line) or found_cheri_triple:
            run_line = re.sub(target_cpu_re, "", run_line)  # remove
            run_line = re.sub(triple_cheri_freebsd_re, "", run_line)  # remove
            run_line = run_line.replace("%clang_cc1", "%cheri_cc1")
            run_line = run_line.replace("-mllvm -cheri128", "")
            run_line = re.sub(r"-cheri-size \d+ ", "", run_line)  # remove
            run_line = re.sub(r"-target-cpu mips4 ", "", run_line)  # remove
            target_abi_re = re.compile(r"-target-abi\s+purecap\s*")
            if re.search(target_abi_re, run_line) is not None or "-purecap" in found_cheri_triple:
                run_line = re.sub(target_abi_re, "", run_line)  # remove
                assert "%cheri_cc1" in run_line
                run_line = run_line.replace("%cheri_cc1", "%cheri_purecap_cc1")
    if "llc " in run_line:
        # TODO: convert the 128/256 variants?
        triple_cheri_freebsd_re = re.compile(
            r"-mtriple=+((?:cheri|mips64c128|mips64c256)-unknown-freebsd\d*(-purecap)?)*\s+")
        found_cheri_triple = None
        triple_match = re.search(triple_cheri_freebsd_re, run_line)
        if triple_match:
            found_cheri_triple = triple_match.group(1)
            run_line = re.sub(triple_cheri_freebsd_re, "", run_line)  # remove triple

            target_abi_re = re.compile(r"-target-abi\s+purecap\s*")
            if re.search(target_abi_re, run_line) is not None or "-purecap" in found_cheri_triple:
                # purecap
                run_line = re.sub(target_abi_re, "", run_line)  # remove
                run_line = re.sub(r"\s-relocation-model=pic", "", run_line)  # remove
                run_line = re.sub(r"llc\s+", "%cheri_purecap_llc ", run_line)  # remove triple
            else:
                # hybrid
                run_line = re.sub(r"llc\s+", "%cheri_llc ", run_line)  # remove triple

            # remove 128 vs 256:
            run_line = re.sub(r" -cheri-size \d+", "", run_line)  # remove
            run_line = re.sub(r" -mattr=\+cheri\d+", "", run_line)  # remove
            run_line = re.sub(r" -mcpu=\+cheri\d+", "", run_line)  # remove
    if "opt " in run_line:
        run_line = re.sub(r"opt\s+-mtriple=cheri-unknown-freebsd", "%cheri_opt", run_line)
    return run_line


# to test the lit substitutions
# class fake_args:
#     clang_cmd = "/path/to/clang"
#     llc_cmd = "/path/to/llc"
#     opt_cmd = "/path/to/opt"
#
# print(add_lit_substitutions(fake_args(), "%clang_cc1 -Wfatal-errors %s"))
#
# sys.exit()

class ReduceTool(metaclass=ABCMeta):
    def __init__(self, args: "Options", name: str, tool: Path) -> None:
        self.tool = tool
        self.name = name
        self.exit_statement = ""
        self.args = args
        self.infile_name = None
        self.not_interesting_exit_code = None  # type: int
        self.interesting_exit_code = None  # type: int
        print("Reducing test case using", name)

    def _reduce_script_text(self, input_file: Path, run_cmds: typing.List[typing.List[str]]):
        verbose_print("Generating reduce script for the following commands:", run_cmds)
        # Handling timeouts in a shell script is awful -> just generate a python script instead
        result = """#!/usr/bin/env python3
import subprocess
import os
import signal
import sys

# https://stackoverflow.com/questions/4789837/how-to-terminate-a-python-subprocess-launched-with-shell-true/4791612#4791612
def run_cmd(cmd, timeout):
    with subprocess.Popen(cmd, shell=True, preexec_fn=os.setsid) as process:
        try:
            stdout, stderr = process.communicate(timeout=timeout)
            retcode = process.poll()
            return subprocess.CompletedProcess(process.args, retcode, stdout, stderr)
        except subprocess.TimeoutExpired:
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            process.kill()
            raise subprocess.TimeoutExpired(process.args, timeout)
        except:
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            process.kill()
            process.wait()
            raise
"""
        timeout_arg = self.args.timeout if self.args.timeout else "None"
        for cmd in run_cmds:
            # check for %s should have happened earlier
            assert "%s" in cmd, cmd
            compiler_cmd = quote_cmd(cmd).replace("%s", self.input_file_arg(input_file))
            assert compiler_cmd.startswith("/"), "Command must use absolute path: " + compiler_cmd
            grep_msg = ""
            crash_flag = "--crash" if self.args.expected_error_kind in (None, ErrorKind.CRASH) else ""
            if self.args.crash_message:
                grep_msg += "2>&1 | grep -F " + shlex.quote(self.args.crash_message)
            # exit once the first command crashes
            timeout_exitcode = self.not_interesting_exit_code
            if self.args.expected_error_kind == ErrorKind.INFINITE_LOOP:
                timeout_exitcode = self.interesting_exit_code
            result += """
try:
    command = r'''{not_cmd} {crash_flag} {command} {grep_msg} '''
    result = run_cmd(command, timeout={timeout_arg})
    if result.returncode != 0:
        sys.exit({not_interesting})
except subprocess.TimeoutExpired:
    print("TIMED OUT", file=sys.stderr)
    sys.exit({timeout_exitcode})
except Exception as e:
    print("SOME OTHER ERROR:", e)
    sys.exit({not_interesting})

""".format(timeout_arg=timeout_arg, not_interesting=self.not_interesting_exit_code, timeout_exitcode=timeout_exitcode,
           not_cmd=self.args.not_cmd, crash_flag=crash_flag, command=compiler_cmd, grep_msg=grep_msg)

        return result + "sys.exit(" + str(self.interesting_exit_code) + ")"

    def _create_reduce_script(self, tmpdir: Path, input_file: Path, run_cmds):
        reduce_script = Path(tmpdir, "reduce_script.sh").absolute()
        reduce_script_text = self._reduce_script_text(input_file, run_cmds)
        reduce_script.write_text(reduce_script_text)
        print("Reduce script:\n", bold(reduce_script_text), sep="")
        reduce_script.chmod(0o755)
        if not self.is_reduce_script_interesting(reduce_script, input_file):
            die("Reduce script is not interesting!")
        return reduce_script

    def create_test_case(self, input_text: str, test_case: Path,
                         run_lines: typing.List[str]):
        processed_run_lines = []
        # TODO: try to remove more flags from the RUN: line!
        for run_line in run_lines:
            verbose_print("Adding run line: ", run_line)
            with_lit_subs = add_lit_substitutions(self.args, run_line)
            verbose_print("Substituted line: ", with_lit_subs)
            processed_run_lines.append(with_lit_subs)
        result = "\n".join(processed_run_lines) + "\n" + input_text
        with test_case.open("w", encoding="utf-8") as f:
            f.write(result)
            f.flush()
        print("\nResulting test case ", test_case, sep="")
        verbose_print(result)

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        raise NotImplemented()

    @abstractmethod
    def reduce(self, input_file: Path, extra_args: list, tempdir: Path,
               run_cmds: typing.List[typing.List[str]],
               run_lines: typing.List[str]):
        raise NotImplemented()

    @abstractmethod
    def input_file_arg(self, input_file: Path) -> str:
        raise NotImplemented()


class RunBugpoint(ReduceTool):
    def __init__(self, args: "Options") -> None:
        super().__init__(args, "bugpoint", tool=args.bugpoint_cmd)
        # bugpoint wants a non-zero exit code on interesting exit code
        self.interesting_exit_code = 1  # type: int
        self.not_interesting_exit_code = 0  # type: int

    def reduce(self, input_file, extra_args, tempdir,
               run_cmds: typing.List[typing.List[str]],
               run_lines: typing.List[str]):
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
        # This is also needed when reducing infinite loops since otherwise bugpoint will just freeze
        if self.args.crash_message or self.args.expected_error_kind == ErrorKind.INFINITE_LOOP:
            # check that the reduce script is interesting:
            # http://blog.llvm.org/2015/11/reduce-your-testcases-with-bugpoint-and.html
            # ./bin/bugpoint -compile-custom -compile-command=./check.sh -opt-command=./bin/opt my_test_case.ll
            reduce_script = self._create_reduce_script(tempdir, input_file.absolute(), run_cmds)
            print("Checking whether reduce script works")
            test_result = subprocess.run([str(reduce_script.absolute()), str(input_file)])
            if test_result.returncode == 0:
                die("Interestingness test failed for bugpoint. Does the command really crash? Script was",
                    reduce_script.read_text())
            bugpoint += ["-compile-custom", "-compile-command=" + str(reduce_script.absolute()), input_file]
        else:
            bugpoint += ["-run-llc-ia", input_file]
            tool_args = run_cmds[0][1:]
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
            subprocess.run([str(self.args.llvm_dis_cmd), "-o", "-", str(expected_output_file)],
                           stdout=subprocess.PIPE, check=True)
            # Rename instructions to avoid stupidly long names generated by bugpoint:
            renamed = subprocess.run([str(self.args.opt_cmd), "-S", "-o", "-", "--instnamer", "--metarenamer",
                                      "--name-anon-globals", str(expected_output_file)], stdout=subprocess.PIPE)
            self.create_test_case(renamed.stdout.decode("utf-8"), input_file.with_suffix(".test" + input_file.suffix),
                                  run_lines)

    def input_file_arg(self, input_file: Path):
        # bugpoint expects a script that takes the input files as arguments:
        return "''' + ' '.join(sys.argv[1:]) + '''"

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        proc = subprocess.run([str(reduce_script), str(input_file)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return proc.returncode == self.interesting_exit_code


class RunLLVMReduce(ReduceTool):
    def __init__(self, args: "Options") -> None:
        super().__init__(args, "llvm-reduce", tool=args.llvm_reduce_cmd)
        # bugpoint wants a non-zero exit code on interesting exit code
        self.interesting_exit_code = 0  # type: int
        self.not_interesting_exit_code = 1  # type: int

    def reduce(self, input_file, extra_args, tempdir, run_cmds: typing.List[typing.List[str]],
               run_lines: typing.List[str]):
        expected_output_file = Path.cwd() / (input_file.name + "-reduced.ll")
        if expected_output_file.exists():
            print("bugpoint output file already exists: ", bold(expected_output_file))
            if input("Delete it and continue? [Y/n]").lower().startswith("n"):
                die("Can't continue")
            else:
                expected_output_file.unlink()
        # This is also needed when reducing infinite loops since otherwise bugpoint will just freeze
        reduce_script = self._create_reduce_script(tempdir, input_file.absolute(), run_cmds)
        llvm_reduce = [self.tool, "--test=" + str(reduce_script.absolute()),
                       "--output=" + str(expected_output_file), input_file]
        llvm_reduce += extra_args
        print("About to run", llvm_reduce)
        print("Working directory:", os.getcwd())
        try:
            env = os.environ.copy()
            env["PATH"] = str(self.args.bindir) + ":" + env["PATH"]
            try:
                run(llvm_reduce, env=env)
            except KeyboardInterrupt:
                print(red("\nCTRL+C detected, stopping llvm-reduce.", style="bold"))
        finally:
            print("Output files are in:", os.getcwd())
            # TODO: generate a test case from the output files?
        if expected_output_file.exists():
            # print("Renaming functions in test...")
            # renamed = subprocess.run([str(self.args.opt_cmd), "-S", "-o", "-", "--instnamer", "--metarenamer",
            #                           "--name-anon-globals", str(expected_output_file)], stdout=subprocess.PIPE)
            # self.create_test_case(renamed.stdout.decode("utf-8"),
            #                       input_file.with_suffix(".test" + input_file.suffix), run_lines)
            self.create_test_case(expected_output_file.read_text("utf-8"),
                                  input_file.with_suffix(".test" + input_file.suffix), run_lines)

    def input_file_arg(self, input_file: Path):
        # llvm-reduce expects a script that takes the input files as arguments:
        return "''' + ' '.join(sys.argv[1:]) + '''"

    def is_reduce_script_interesting(self, reduce_script: Path, input_file: Path) -> bool:
        proc = subprocess.run([str(reduce_script), str(input_file)])
        return proc.returncode == self.interesting_exit_code


class RunCreduce(ReduceTool):
    def __init__(self, args: "Options") -> None:
        super().__init__(args, "creduce", tool=args.creduce_cmd)
        self.exit_statement = "&& exit 0"
        # creduce wants a zero exit code on interesting test cases
        self.interesting_exit_code = 0
        self.not_interesting_exit_code = 1

    def reduce(self, input_file: Path, extra_args: "list[str]", tempdir: Path,
               run_cmds: "list[list[str]]", run_lines: "list[str]"):
        reduce_script = self._create_reduce_script(
            tempdir, input_file.absolute(), run_cmds)
        # Work around https://github.com/csmith-project/creduce/issues/195.
        # Note: CVise also requires it to be in the same directory since it
        # does not allow an absolute path to the test case
        reduced_input_file = Path(tempdir, input_file.name)
        shutil.copy(str(input_file), str(reduced_input_file))
        creduce = ["time", str(self.tool), str(reduce_script),
                   reduced_input_file.name] + extra_args
        # This is way too verbose
        if self.args.extremely_verbose:
            creduce.append("--print-diff")
        print("About to run", creduce)
        try:
            run(creduce, cwd=tempdir)
        except KeyboardInterrupt:
            print(red("\nCTRL+C detected, stopping creduce.", style="bold"))
        print("\nDONE!")
        # write the output test file:
        reduced_testcase = reduced_input_file.read_text(encoding="utf-8")
        if not reduced_testcase:
            print(red("Reduction resulted in empty testcase!"))
        output = input_file.with_suffix(".test" + input_file.suffix)
        self.create_test_case(reduced_testcase, output, run_lines)

    def input_file_arg(self, input_file: Path):
        # creduce creates an input file in the test directory with the same
        # name as the original input.
        return input_file.name

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
    def __init__(self, args: "Options") -> None:
        super().__init__(args, "noop", tool=Path("/dev/null"))

    def reduce(self, input_file, extra_args, tempdir,
               run_cmds: typing.List[typing.List[str]],
               run_lines: typing.List[str]):
        self.create_test_case("Some strange reduced test case\n",
                              input_file.with_suffix(".test" + input_file.suffix), run_lines)

    def input_file_arg(self, input_file: Path) -> str:
        raise NotImplemented()


class Options(object):
    # noinspection PyUnresolvedReferences
    def __init__(self, args: argparse.Namespace) -> None:
        self.verbose = args.verbose  # type: bool
        self.timeout = args.timeout  # type: int
        self.extremely_verbose = args.extremely_verbose  # type: bool
        self.bindir = Path(args.bindir)
        self.args = args
        self.no_initial_reduce = args.no_initial_reduce  # type: bool
        self.crash_message = args.crash_message  # type: str
        self.llvm_error = args.llvm_error  # type: bool
        # could also be an LLVM error or Address Sanitizer error that returns a non-crash exit code
        self.expected_error_kind = None  # type: ErrorKind
        if self.llvm_error:
            self.expected_error_kind = ErrorKind.FATAL_ERROR
        if args.infinite_loop:
            if not self.timeout:
                self.timeout = 30
            self.expected_error_kind = ErrorKind.INFINITE_LOOP

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
    def llvm_reduce_cmd(self):
        return self._get_command("llvm-reduce")

    @property
    def creduce_cmd(self):
        # noinspection PyUnresolvedReferences
        creduce_path = self.args.creduce_cmd
        if not creduce_path:
             creduce_path = shutil.which("cvise")
        if not creduce_path:
             creduce_path = shutil.which("creduce")
        if not creduce_path:
            die("Could not find `creduce` or `cvise` in $PATH. Add it to $PATH or pass --creduce-cmd")
        return Path(creduce_path)

    def _get_command(self, name):
        result = Path(getattr(self.args, name + "_cmd", None) or Path(self.bindir, name))
        if not result.exists():
            die("Invalid `" + name + "` binary`", result)
        return result


class Reducer(object):
    def __init__(self, parser: argparse.ArgumentParser) -> None:
        self.args, self.reduce_args = parser.parse_known_args()
        if self.args.extremely_verbose:
            self.args.verbose = True
        global options
        options = Options(self.args)
        self.options = options
        self.subst_handler = LitSubstitutionHandler(options)
        self.testcase = Path(self.args.testcase)
        # RUN: lines to add to the test case
        self.run_lines = []  # type: typing.List[str]
        # the lines without RUN: suitably quoted for passing to a shell
        self.run_cmds = []  # type: typing.List[typing.List[str]]
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
                real_infile = self._parse_test_case(f, infile)
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
            source_file_index = -1
            source_file_name = command[source_file_index]
            source_file = infile.with_name(source_file_name)
            while source_file_name.startswith("-"):
                print("WARNING: crash reproducer command line probably does not end with the input file",
                      "name: got", blue(source_file_name), "which is probably not a file!")
                source_file_index = source_file_index - 1
                source_file_name = command[source_file_index]
                source_file = infile.with_name(source_file_name)
                if not source_file.exists():
                    continue

            if not source_file.exists():
                die("Reproducer input file", source_file, "does not exist!")
            real_in_file = source_file
            verbose_print("Real input file is", real_in_file)
            command[source_file_index] = "%s"
            # output to stdout
            if "-o" not in command:
                print("Adding '-o -' to the compiler invocation")
                command += ["-o", "-"]
            # try to remove all unnecessary command line arguments
            command[0] = str(self.options.clang_cmd)  # replace command with the clang binary
            command, real_in_file = self.simplify_crash_command(command, real_in_file.absolute())
            assert Path(command[0]).is_absolute(), "Command must be absolute: " + command[0]
            quoted_cmd = quote_cmd(command)
            verbose_print("Test command is", bold(quoted_cmd))
            self.run_cmds.append(command)
            if real_in_file.suffix == ".ll":
                comment_start = ";"
            elif real_in_file.suffix in (".S", ".s"):
                comment_start = "#"
            else:
                comment_start = "//"
            self.run_lines.append(comment_start + " RUN: " + quoted_cmd)
        if not real_in_file:
            die("Could not compute input file for crash reproducer")
        return real_in_file

    def _check_crash(self, command, infile, proc_info: subprocess.CompletedProcess = None,
                     force_print_cmd=False) -> typing.Optional[ErrorKind]:
        # command = ["/tmp/crash"]
        full_cmd = command + [str(infile)]
        assert "%s" not in full_cmd, full_cmd

        if force_print_cmd:
            print("\nRunning", blue(quote_cmd(full_cmd)))
        else:
            verbose_print("\nRunning", blue(quote_cmd(full_cmd)))
        if self.args.reduce_tool == "noop":
            if proc_info is not None:
                proc_info.stderr = b"Assertion `noop' failed."
            print(green(" yes"))
            return ErrorKind.CRASH
        infinite_loop_timeout = self.options.timeout if self.options.timeout else 30
        try:
            proc = subprocess.run(full_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                                  timeout=infinite_loop_timeout)
            error_kind = None
            if proc.returncode < 0 or proc.returncode == 254:
                error_kind = ErrorKind.CRASH
            else:
                verbose_print("Exit code", proc.returncode, "was not a crash, checking stderr for known error.")
                verbose_print("stderr:", proc.stderr)
                # treat fatal llvm errors (cannot select, etc)as crashes too
                # Also ASAN errors just return 1 instead of a negative exit code so we have to grep the message
                for kind in ErrorKind:
                    if kind.value:
                        verbose_print("Checking for", kind.value)
                        if any(msg in proc.stderr for msg in kind.value):
                            verbose_print("Crash was", kind)
                            error_kind = kind
                            break
        except subprocess.TimeoutExpired as e:
            proc = subprocess.CompletedProcess(e.args, -1, e.stdout, e.stderr)
            error_kind = ErrorKind.INFINITE_LOOP
            verbose_print("Running took longer than", infinite_loop_timeout, "seconds -> assuming infinite loop")

        if proc_info is not None:  # To get the initial error message:
            proc_info.stdout = proc.stdout
            proc_info.stderr = proc.stderr
            proc_info.returncode = proc.returncode
            proc_info.args = proc.args
        if error_kind:
            if self.options.expected_error_kind and self.options.expected_error_kind != error_kind:
                print(red(" yes, but got " + error_kind.name + " instead of " + self.options.expected_error_kind.name))
                return None
            crash_message_found = not self.options.crash_message or (
                        self.options.crash_message in proc.stderr.decode("utf-8"))
            if error_kind == ErrorKind.INFINITE_LOOP or crash_message_found:
                print(green(" yes"))
                return error_kind
            else:
                print(red(" yes, but with a different crash message!"))
                print("Note: Expected crash message '", bold(self.options.crash_message), "' not found in:\n",
                      proc.stderr.decode("utf-8"), sep="")
                return None
        print(red(" no"))
        return None

    @staticmethod
    def _filter_args(args, *, noargs_opts_to_remove=None, noargs_opts_to_remove_startswith: typing.List[str] = None,
                     one_arg_opts_to_remove: typing.List[str] = None,
                     one_arg_opts_to_remove_if: typing.Dict[str, typing.Callable[[str], bool]] = None):
        if one_arg_opts_to_remove is None:
            one_arg_opts_to_remove = list()
        if noargs_opts_to_remove_startswith is None:
            noargs_opts_to_remove_startswith = list()
        if noargs_opts_to_remove is None:
            noargs_opts_to_remove = list()
        if one_arg_opts_to_remove_if is None:
            one_arg_opts_to_remove_if = dict()
        result = []
        skip_next = False

        def should_remove_arg(option, value):
            for a, predicate in one_arg_opts_to_remove_if.items():
                if option == a:
                    verbose_print("Testing predicate", predicate, "for arg", option, "on", value)
                    if predicate(value):
                        return True
            return False

        for i, arg in enumerate(args):
            if skip_next:
                skip_next = False
                continue
            if any(arg == a for a in noargs_opts_to_remove) or any(
                    arg.startswith(a) for a in noargs_opts_to_remove_startswith):
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

    def _try_remove_args(self, command: list, infile: Path, message: str, *, extra_args: list = None, **kwargs):
        new_command = self._filter_args(command, **kwargs)
        print(message, end="", flush=True)
        if extra_args:
            new_command += extra_args
        if new_command == command:
            print(green(" none of those flags are in the command line"))
            return command
        if self._check_crash(new_command, infile):
            return new_command
        return command

    @staticmethod
    def _infer_crash_message(stderr: bytes):
        print("Inferring crash message from", stderr.decode("utf-8"))
        if not stderr:
            return None
        simple_regexes = [re.compile(s) for s in (
            r"Assertion `(.+)' failed.",  # Linux assert()
            r"Assertion failed: \(.+\),",  # FreeBSD/Mac assert()
            r"UNREACHABLE executed( at .+)?!",  # llvm_unreachable()
            # generic code gen crashes (at least creduce will keep the function name):
            r"LLVM IR generation of declaration '(.+)'",
            r"Generating code for declaration '(.+)'",
            r"LLVM ERROR: Cannot select: (.+)",
            r"LLVM ERROR: Cannot select:",
            r"LLVM ERROR: (.+)",
            r"\*\*\* Bad machine code: (.+) \*\*\*",
        )]
        regexes = [(r, 0) for r in simple_regexes]
        # For this crash message we only want group 1
        # TODO: add another grep for the program counter
        regexes.insert(0, (
            re.compile(r"ERROR: (AddressSanitizer: .+ on address) 0x[0-9a-fA-F]+ (at pc 0x[0-9a-fA-F]+)"), 1))
        # only get the kind of the cannot select from the message (without the number for tNN)
        regexes.insert(0, (re.compile(r"LLVM ERROR: Cannot select: (t\w+): (.+)", ), 2))
        # This message is different when invoked as llc: it prints LLVM ERROR instead
        # so we only capture the actual message
        regexes.insert(0, (re.compile(r"fatal error: error in backend:(.+)"), 1))
        # same without colour diagnostics:
        regexes.insert(0, (re.compile("\x1b\\[0m\x1b\\[0;1;31mfatal error: \x1b\\[0merror in backend:(.+)"), 1))

        # Any other error in backed
        regexes.append((re.compile(r"error in backend:(.+)"), 1))
        # final fallback message: generic fatal error:
        regexes.append((re.compile(r"fatal error:(.+)"), 0))

        for line in stderr.decode("utf-8").splitlines():
            # Check for failed assertions:
            for r, index in regexes:
                match = r.search(line)
                if match:
                    message = match.group(index)
                    if "\x1b" in message:
                        message = message[:message.rfind("\x1b")]
                    print("Inferred crash message bytes: ", message.encode("utf-8"))
                    return message
        return None

    def simplify_crash_command(self, command: list, infile: Path) -> tuple:
        new_command = command.copy()
        assert new_command.count("%s") == 1, new_command
        new_command.remove("%s")

        # Remove -mllvm options that no longer exist
        def is_old_llvm_option(opt: str):
            # -cheri-cap-table=true/false was replace with -mllvm -cheri-cap-table-abi=
            if opt == "-cheri-cap-table" or opt.startswith("-cheri-cap-table="):
                return True
            return False

        new_command = self._filter_args(new_command, one_arg_opts_to_remove_if={"-mllvm": is_old_llvm_option})

        print("Checking whether reproducer crashes with ", new_command[0], ":", sep="", end="", flush=True)
        crash_info = subprocess.CompletedProcess(None, None)
        self.options.expected_error_kind = self._check_crash(new_command, infile, crash_info, force_print_cmd=True)
        if not self.options.expected_error_kind:
            die("Crash reproducer no longer crashes?")

        verbose_print("Reducing a", self.options.expected_error_kind, "with message =", self.options.crash_message)
        verbose_print("Stderr was", crash_info.stderr)
        if not self.options.crash_message:
            if self.options.expected_error_kind == ErrorKind.INFINITE_LOOP:
                self.options.crash_message = "INFINITE LOOP WHILE RUNNING, THIS GREP SHOULD NEVER MATCH!"
            else:
                print("Attempting to infer crash message from process output")
                inferred_msg = self._infer_crash_message(crash_info.stderr)
                if inferred_msg:
                    print("Inferred crash message as '" + green(inferred_msg) + "'")
                    if not input("Use this message? [Y/n]").lower().startswith("n"):
                        self.options.crash_message = inferred_msg
                else:
                    print("Could not infer crash message, stderr was:\n\n")
                    print(crash_info.stderr.decode("utf-8"))
                    print("\n\n")
        if not self.options.crash_message:
            print("Could not infer crash message from crash reproducer.")
            print(red("WARNING: Reducing without specifying the crash message will probably result"
                      " in the wrong test case being generated."))
            if not input("Are you sure you want to continue? [y/N]").lower().startswith("y"):
                sys.exit()

        if new_command[0] == str(self.options.clang_cmd):
            new_command, infile = self._simplify_clang_crash_command(new_command, infile)
        elif new_command[0] == str(self.options.llc_cmd):
            # TODO: should be able to simplify llc crashes (e.g. by adding -O0, -verify-machineinstrs, etc)
            pass
        new_command.append("%s")  # ensure that the command contains %s at the end
        return new_command, infile

    @staticmethod
    def list_with_flag_at_end(orig: list, flag: str) -> list:
        result = list(orig)
        while flag in result:
            result.remove(flag)
        result.append(flag)
        return result

    def _simplify_clang_crash_command(self, new_command: list, infile: Path) -> tuple:
        assert new_command[0] == str(self.options.clang_cmd)
        assert "-o" in new_command
        assert "%s" not in new_command
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether replacing optimization level with -O0 crashes:",
            noargs_opts_to_remove_startswith=["-O"],
            extra_args=["-O0"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -coverage-notes-file crashes:",
            one_arg_opts_to_remove=["-coverage-notes-file"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without debug info crashes:",
            noargs_opts_to_remove=["-dwarf-column-info", "-munwind-tables", "-ggnu-pubnames"],
            one_arg_opts_to_remove=["-split-dwarf-file", "-split-dwarf-output"],
            noargs_opts_to_remove_startswith=["-debug-info-kind=", "-dwarf-version=", "-debugger-tuning=",
                                              "-fdebug-prefix-map=", "-fdebug-compilation-dir=",
                                              "-fcoverage-compilation-dir="],
        )
        # try emitting llvm-ir (i.e. frontend bug):
        print("Checking whether -emit-llvm crashes:", end="", flush=True)
        generate_ir_cmd = self.list_with_flag_at_end(new_command, "-emit-llvm")
        if "-cc1" in generate_ir_cmd:
            # Don't add the optnone attribute to the generated IR function
            generate_ir_cmd = self.list_with_flag_at_end(generate_ir_cmd, "-disable-O0-optnone")
        while "-emit-obj" in generate_ir_cmd:
            generate_ir_cmd.remove("-emit-obj")
        if self._check_crash(generate_ir_cmd, infile, force_print_cmd=True):
            print("Crashed while generating IR, trying again with -O1")
            generate_ir_cmd = self.list_with_flag_at_end(generate_ir_cmd, "-O1")
            if self._check_crash(generate_ir_cmd, infile, force_print_cmd=True):
                print("Crashed while generating IR at -O1, trying again with -O0")
                generate_ir_cmd = self.list_with_flag_at_end(generate_ir_cmd, "-O0")
                if self._check_crash(generate_ir_cmd, infile, force_print_cmd=True):
                    print("Crashed while generating IR -> must be a", blue("frontend crash.", style="bold"),
                          "Will need to use creduce for test case reduction")
                    # Try to remove the flags that were added:
                    new_command = generate_ir_cmd
                    new_command = self._try_remove_args(
                        new_command, infile, "Checking if it also crashes at -O0:",
                        noargs_opts_to_remove=["-disable-O0-optnone"],
                        noargs_opts_to_remove_startswith=["-O"],
                        extra_args=["-O0"]
                    )
                    return self._simplify_frontend_crash_cmd(new_command, infile)
        # Generating IR worked, try reducing the IR testcase
        print("Must be a ", blue("backend crash", style="bold"), ", ", end="", sep="")
        if self.args.reduce_tool == "creduce":
            print("but reducing with creduce requested. Will not try to convert to a bugpoint test case")
            return self._simplify_frontend_crash_cmd(new_command, infile)
        else:
            print("will try to use bugpoint/llvm-reduce.")
            return self._simplify_backend_crash_cmd(infile, new_command, generate_ir_cmd)

    def _shrink_preprocessed_source(self, input_path, out_file):
        # The initial remove #includes pass takes a long time -> remove all the includes that are inside a #if 0
        # This is especially true for C++ because there are so many #included files in preprocessed input
        with input_path.open("r", errors="replace", encoding="utf-8") as input_file:
            line_regex = re.compile(r'^#\s+\d+\s+".*".*')
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
                    extremely_verbose_print("Ending -frewrite-includes-block, skipped", skipped_rewrite_includes,
                                            "lines")
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
                elif re.match(line_regex, line):
                    extremely_verbose_print("Removing # line directive:", line.rstrip())
                    continue
                else:
                    out_file.write(line)
            out_file.flush()
        if self.args.extremely_verbose:
            verbose_print("Initial reduction:")
            subprocess.call(["diff", "-u", str(input_path), out_file.name])
            pass

    def _simplify_frontend_crash_cmd(self, new_command: list, infile: Path):
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without warnings crashes:",
            noargs_opts_to_remove=["-w"], noargs_opts_to_remove_startswith=["-W"], extra_args=["-w"])
        # Try to make implicit int an error to generate more sensible test output
        # If we don't add this we get really obscure code that doesn't look like it should compile
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling with -Werror=implicit-int crashes:",
            noargs_opts_to_remove=["-w"], extra_args=["-Wimplicit-int", "-Werror=implicit-int"])
        # speed up test case reduction by aborting the compilation on the first error
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling with -Wfatal-errors crashes:",
            noargs_opts_to_remove=["-w"], extra_args=["-Wfatal-errors"]
        )

        # Removing all the #ifdefs and #defines that get added by the #included headers can speed up reduction a lot
        print("Generating preprocessed source")
        try:
            preprocessed = infile.with_name(infile.stem + "-pp" + infile.suffix)
            base_pp_command = self._filter_args(new_command, one_arg_opts_to_remove=["-o"],
                                                noargs_opts_to_remove=["-S", "-emit-llvm"])
            base_pp_command += ["-E", "-o", str(preprocessed), str(infile)]
            no_line_pp_command = base_pp_command + ["-P"]
            verbose_print(no_line_pp_command)
            subprocess.check_call(no_line_pp_command)
            assert preprocessed.exists()
            print("Checking if preprocessed source (without line numbers)", preprocessed, "crashes: ", end="",
                  flush=True)
            if self._check_crash(new_command, preprocessed):
                infile = preprocessed
            else:
                print(red("Compiling preprocessed source (without line numbers)", preprocessed,
                          "no longer crashes, not using it. Will try with line numbers"))
                verbose_print(base_pp_command)
                subprocess.check_call(base_pp_command)
                assert preprocessed.exists()
                print("Checking if preprocessed source", preprocessed, "crashes: ", end="", flush=True)
                if self._check_crash(new_command, preprocessed):
                    infile = preprocessed
                else:
                    print(red("Compiling preprocessed source (with line numbers)", preprocessed,
                              "no longer crashes, not using it."))
        except Exception as e:
            print("Failed to preprocess", infile, "-> will use the unprocessed source ", e)

        # creduce wastes a lot of time trying to remove #includes and dead cases generated
        #  by -frewrite-includes (if the preprocessed source no longer crashes)
        # We also try to remove the #line directives
        try:
            smaller = infile.with_name(infile.stem + "-smaller" + infile.suffix)
            with smaller.open("w", encoding="utf-8") as reduced_file:
                original_size = infile.stat().st_size
                self._shrink_preprocessed_source(infile, reduced_file)
                reduced_file.flush()
                new_size = smaller.stat().st_size
                percent_reduction = 100 - 100.0 * (new_size / original_size)
                print("Initial preprocessing: {} bytes -> {} bytes ({}% reduction)".format(
                    original_size, new_size, percent_reduction))
            if self._check_crash(new_command, smaller):
                infile = smaller
            else:
                print(red("Compiling processed preprocessed source", smaller,
                          "no longer crashes, not using it."))
        except Exception as e:
            print("Failed to shrink", infile, "-> will use the unprocessed source", e)

        if "-emit-obj" in new_command:
            new_command = self._try_remove_args(
                new_command, infile, "Checking whether emitting ASM instead of object crashes:",
                noargs_opts_to_remove=["-emit-obj"], extra_args=["-S"])

        # check if floating point args are relevant
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without floating point arguments crashes:",
            noargs_opts_to_remove=["-msoft-float"],
            one_arg_opts_to_remove=["-mfloat-abi"],
            one_arg_opts_to_remove_if={"-target-feature": lambda a: a == "+soft-float"}
        )
        # check if math args are relevant
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without math arguments crashes:",
            noargs_opts_to_remove=["-fno-rounding-math", "-fwrapv"])
        # check if frame pointer args are relevant
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without frame pointer argument crashes:",
            noargs_opts_to_remove=["-mdisable-fp-elim"],
            noargs_opts_to_remove_startswith=["-mframe-pointer="]
        )

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without PIC flags crashes:",
            one_arg_opts_to_remove=["-mrelocation-model", "-pic-level"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without thread model flags crashes:",
            one_arg_opts_to_remove=["-mthread-model"],
            noargs_opts_to_remove_startswith=["-ftls-model="],
        )
        previous = list(new_command)
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -target-feature flags crashes:",
            one_arg_opts_to_remove=["-target-feature"],
        )
        if previous == new_command and "-target-feature" in new_command:
            print(red("Some of the -target-feature flags are required, trying to reduce"))
            target_features = []
            for i, arg in enumerate(new_command):
                if arg == "-target-feature" and i + 1 < len(new_command):
                    target_features.append(new_command[i + 1])
            for feature in target_features:
                new_command = self._try_remove_args(
                    new_command, infile, f"Checking whether compiling without '-target-feature {feature}' crashes:",
                    one_arg_opts_to_remove_if={"-target-feature": lambda a: a == feature}
                )

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without various MIPS flags crashes:",
            one_arg_opts_to_remove_if={
                "-mllvm": lambda a: a.startswith("-mips-ssection-threshold=") or a == "-mxgot" or a == "-mgpopt"}
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without various CHERI flags crashes:",
            one_arg_opts_to_remove_if={
                "-mllvm": lambda a: a.startswith("-cheri-cap-table-abi=") or a.startswith("-mxcaptable")}
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -mrelax-all crashes:",
            noargs_opts_to_remove=["-mrelax-all", "--mrelax-relocations"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -D flags crashes:",
            noargs_opts_to_remove=["-sys-header-deps"],
            one_arg_opts_to_remove=["-D"],
            noargs_opts_to_remove_startswith=["-D"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without include flags crashes:",
            noargs_opts_to_remove=["-nostdsysteminc"],
            one_arg_opts_to_remove=["-I", "-isystem", "-internal-isystem"],
            noargs_opts_to_remove_startswith=["-I", "-isystem"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without include flags crashes:",
            noargs_opts_to_remove=["-nostdsysteminc", "-nobuiltininc"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without function/data sections crashes:",
            noargs_opts_to_remove=["-ffunction-sections", "-fdata-sections"],
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -ffp-contract= crashes:",
            noargs_opts_to_remove_startswith=["-ffp-contract="]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -x flag crashes:",
            one_arg_opts_to_remove=["-x"]
        )
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether compiling without -std= flag crashes:",
            noargs_opts_to_remove_startswith=["-std="]
        )

        if "-disable-llvm-verifier" in new_command:
            new_command = self._try_remove_args(
                new_command, infile, "Checking whether compiling without -disable-llvm-verifier crashes:",
                noargs_opts_to_remove=["-disable-llvm-verifier"])

        if "-fcxx-exceptions" in new_command or "-fexceptions" in new_command:
            new_command = self._try_remove_args(
                new_command, infile, "Checking whether compiling without exceptions crashes:",
                noargs_opts_to_remove=["-fexceptions", "-fcxx-exceptions"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether misc C++ options can be removed:",
            noargs_opts_to_remove=["-fno-rtti", "-mconstructor-aliases", "-nostdinc++"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether misc optimization options can be removed:",
            noargs_opts_to_remove=["-vectorize-loops", "-vectorize-slp"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether addrsig/init-array options can be removed:",
            noargs_opts_to_remove=["-fuse-init-array", "-faddrsig"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -ffreestanding can be removed:",
            noargs_opts_to_remove=["-ffreestanding"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -f(no-)signed-char can be removed:",
            noargs_opts_to_remove=["-fno-signed-char", "-fsigned-char"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether TLS/relocation model options can be removed:",
            noargs_opts_to_remove_startswith=["-ftls-model="],
            one_arg_opts_to_remove=["-mrelocation-model"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -fgnuc-version= can be removed:",
            noargs_opts_to_remove_startswith=["-fgnuc-version="])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -target-cpu option can be removed:",
            one_arg_opts_to_remove=["-target-cpu"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -target-abi option can be removed:",
            one_arg_opts_to_remove=["-target-abi"])

        new_command = self._try_remove_args(
            new_command, infile, "Checking whether -msmall-data-limit option can be removed:",
            one_arg_opts_to_remove=["-msmall-data-limit"])

        # try to remove some arguments that should not be needed
        new_command = self._try_remove_args(
            new_command, infile, "Checking whether misc diagnostic options can be removed:",
            noargs_opts_to_remove=["-disable-free", "-discard-value-names", "-masm-verbose",
                                   "-fdeprecated-macro", "-fcolor-diagnostics"],
            noargs_opts_to_remove_startswith=["-fdiagnostics-", "-fobjc-runtime="],
            one_arg_opts_to_remove=["-main-file-name", "-ferror-limit", "-fmessage-length", "-fvisibility",
                                    "-target-linker-version"]
        )
        return new_command, infile

    def _generate_matching_llc_command(self, command: list) -> list:
        llc_args = [str(self.options.llc_cmd), "-o", "/dev/null"]  # TODO: -o -?
        cpu_flag = None  # -mcpu= only allowed once!
        pass_once_flags = set()
        # Some crash messages only happen with verify-machineinstrs:
        pass_once_flags.add("-verify-machineinstrs")
        skip_next = False
        optimization_flag = "-O2"
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
            elif arg.startswith("-O"):
                if arg == "-Os":
                    arg = "-O2"  # llc doesn't understand -Os
                optimization_flag = arg
        if cpu_flag:
            llc_args.append(cpu_flag)
        llc_args.append(optimization_flag)
        llc_args.extend(pass_once_flags)
        return llc_args

    def _simplify_backend_crash_cmd(self, infile: Path, clang_cc1_command: list, generate_ir_command: list):
        ir_pass_crashing = "-emit-llvm" in clang_cc1_command
        assert "-o" in clang_cc1_command
        generate_ir_command = generate_ir_command.copy()
        irfile = infile.with_name(infile.name.partition(".")[0] + "-bugpoint.ll")
        generate_ir_command[generate_ir_command.index("-o") + 1] = str(irfile.absolute())
        if "-discard-value-names" in generate_ir_command:
            generate_ir_command.remove("-discard-value-names")
        print("Generating IR file", irfile)
        try:
            verbose_print(generate_ir_command + [str(infile)])
            subprocess.check_call(generate_ir_command + [str(infile)])
        except subprocess.CalledProcessError:
            print("Failed to generate IR from", infile, "will have to reduce using creduce")
            return self._simplify_frontend_crash_cmd(clang_cc1_command, infile)
        if not irfile.exists():
            die("IR file was not generated?")
        llc_args = self._generate_matching_llc_command(clang_cc1_command)
        # Only makes sense to compile with llc if the crash happened while generating machine code
        if not ir_pass_crashing:
            print("Checking whether compiling IR file with llc crashes:", end="", flush=True)
            llc_info = subprocess.CompletedProcess(b"", -1)
            if self._check_crash(llc_args, irfile, llc_info, force_print_cmd=True):
                print("Crash found with llc -> using llvm-reduce/bugpoint which is faster than creduce.")
                self.reduce_tool = self.get_llvm_ir_reduce_tool()
                return llc_args, irfile
            if self._check_crash(llc_args + ["-filetype=obj"], irfile, llc_info, force_print_cmd=True):
                print("Crash found with llc -filetype=obj -> using llvm-reduce/bugpoint which is faster than creduce.")
                self.reduce_tool = self.get_llvm_ir_reduce_tool()
                return llc_args + ["-filetype=obj"], irfile
            print("Compiling IR file with llc did not reproduce crash. Stderr was:", llc_info.stderr.decode("utf-8"))
        opt_args = llc_args.copy()
        opt_args[0] = str(self.options.opt_cmd)
        opt_args.append("-S")
        opt_info = subprocess.CompletedProcess(b"", -1)
        print("Checking whether compiling IR file with opt crashes:", end="", flush=True)
        if self._check_crash(opt_args, irfile, opt_info, force_print_cmd=True):
            print("Crash found with opt -> using llvm-reduce/bugpoint which is faster than creduce.")
            self.reduce_tool = self.get_llvm_ir_reduce_tool()
            return opt_args, irfile
        print("Compiling IR file with opt did not reproduce crash. Stderr was:", opt_info.stderr.decode("utf-8"))

        print("Checking whether compiling IR file with clang crashes:", end="", flush=True)
        clang_info = subprocess.CompletedProcess(b"", -1)
        bugpoint_clang_cmd = self._filter_args(clang_cc1_command,
                                               noargs_opts_to_remove_startswith=["-W", "-std="],
                                               one_arg_opts_to_remove=["-D", "-x", "-xc", "-xc++", "-main-file-name"])
        bugpoint_clang_cmd.extend(["-x", "ir"])
        if self._check_crash(bugpoint_clang_cmd, irfile, clang_info, force_print_cmd=True):
            print("Crash found compiling IR with clang -> using llvm-reduce/bugpoint which is faster than creduce.")
            self.reduce_tool = self.get_llvm_ir_reduce_tool()
            return bugpoint_clang_cmd, irfile
        print("Compiling IR file with clang did not reproduce crash. Stderr was:", clang_info.stderr.decode("utf-8"))
        print(red("No crash found compiling the IR! Possibly crash only happens when invoking clang -> using creduce."))
        self.reduce_tool = RunCreduce(self.options)
        return self._simplify_frontend_crash_cmd(clang_cc1_command, infile)

    def _parse_test_case(self, f, infile: Path) -> Path:
        # test case: just search for RUN: lines
        real_input_file = None
        for line in f.readlines():
            match = re.match(r".*\s+RUN: (.+)", line)
            if line.endswith("\\"):
                die("RUN lines with continuations not handled yet")
            if match:
                command = match.group(1).strip()
                if "%s" not in command:
                    die("RUN: line does not contain %s -> cannot create replacement invocation")
                if "2>&1" in line:
                    die("Cannot handle 2>&1 in RUN lines yet")
                verbose_print("Found RUN: ", command)
                command = self.subst_handler.expand_lit_subtitutions(command)
                verbose_print("After expansion:", command)
                # We can only simplify the command line for clang right now
                command, new_infile = self.simplify_crash_command(shlex.split(command), infile.absolute())
                if real_input_file is None:
                    real_input_file = new_infile
                elif real_input_file != new_infile:
                    die("Cannot handle RUN lines using different input files")
                verbose_print("Final command:", command)
                self.run_cmds.append(command)
                self.run_lines.append(line[0:line.find(match.group(1))] + quote_cmd(command))
        return real_input_file

    def run(self):
        # scan test case for RUN: lines
        infile = self.parse_RUN_lines(self.testcase)

        if self.reduce_tool is None:
            default_tool = RunLLVMReduce if infile.suffix in (".ll", ".bc") else RunCreduce
            self.reduce_tool = self.get_llvm_ir_reduce_tool(default_tool)
        if self.args.output_file:
            reduce_input = Path(self.args.output_file).absolute()
        else:
            reduce_input = infile.with_name(infile.stem + "-reduce" + infile.suffix).absolute()
        shutil.copy(str(infile), str(reduce_input))
        with tempfile.TemporaryDirectory() as tmpdir:
            # run("ulimit -S -c 0".split())
            starttime = datetime.datetime.now()
            self.reduce_tool.reduce(input_file=reduce_input, extra_args=self.reduce_args, tempdir=tmpdir,
                                    run_cmds=self.run_cmds, run_lines=self.run_lines)
            print("Reducing the testcase took", datetime.datetime.now() - starttime)

    def get_llvm_ir_reduce_tool(self, default_tool=RunLLVMReduce):
        if self.args.reduce_tool is None:
            return default_tool(self.options)
        # if self.args.reduce_tool == "llvm-reduce-and-bugpoint":
        #     return RunLLVMReduceAndBugpoint(self.options)
        if self.args.reduce_tool == "bugpoint":
            return RunBugpoint(self.options)
        if self.args.reduce_tool == "llvm-reduce":
            return RunLLVMReduce(self.options)
        elif self.args.reduce_tool == "noop":  # for debugging purposes
            return SkipReducing(self.options)
        else:
            assert self.args.reduce_tool == "creduce"
            return RunCreduce(self.options)


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
    parser.add_argument("--llvm-reduce-cmd", help="Path to `bugpoint` tool. Default is $BINDIR/llvm-reduce")
    parser.add_argument("--creduce-cmd",
                        help="Path to `creduce`/`cvise` tool. Default is "
                             "`cvise` (or `creduce` if not found)")
    parser.add_argument("--output-file", help="The name of the output file")
    parser.add_argument("--verbose", action="store_true", help="Print more debug output")
    parser.add_argument("--timeout", type=int,
                        help="Treat the test case as not interesting if it runs longer than n seconds")
    parser.add_argument("--extremely-verbose", action="store_true", help="Print tons of debug output")
    parser.add_argument("--llvm-error", action="store_true", help="Reduce a LLVM ERROR: message instead of a crash")
    parser.add_argument("--infinite-loop", action="store_true",
                        help="Try debugging an infinite loop (-> timed out testcases are interesting)."
                             "If timeout is not set this will set it to 30 seconds")
    parser.add_argument("--crash-message",
                        help="If set the crash must contain this message to be accepted for reduction."
                             " This is useful if creduce ends up generating another crash bug that"
                             " is not the one being debugged.")
    parser.add_argument("--reduce-tool", help="The tool to use for test case reduction. "
                                              "Defaults to `llvm-reduce-and-bugpoint` if input file is a .ll or .bc "
                                              "file and `creduce` otherwise.",
                        choices=["llvm-reduce-and-bugpoint", "bugpoint", "creduce", "llvm-reduce", "noop"])
    parser.add_argument("--no-initial-reduce", help="Pass the original input file to creduce without removing #if 0 "
                                                    "regions. Generally this will speed up but in very rare corner "
                                                    "cases it might cause the test case to no longer crash.",
                        action="store_true")
    parser.add_argument("testcase", help="The file to reduce (must be a testcase with a RUN: line that crashes "
                                         "or a .sh file from a clang crash")
    # bash completion for arguments:
    try:
        # noinspection PyUnresolvedReferences
        import argcomplete
        argcomplete.autocomplete(parser)
    except ImportError:
        pass
    # Disable coredumps to avoid filling up disk space:
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    Reducer(parser).run()


if __name__ == "__main__":
    main()
