#!/usr/bin/env python3
import argparse
import subprocess
import sys
import typing
from pathlib import Path


LLVM_SRC_PATH = Path(__file__).parent.parent.parent.parent

class ArchSpecificValues(object):
    def __init__(self, architecture: str, *, cap_range, cap_width,
                 purecap_lit_command_prefix, hybrid_lit_command_prefix):
        self.hybrid_lit_command_prefix = hybrid_lit_command_prefix
        self.purecap_lit_command_prefix = purecap_lit_command_prefix
        self.cap_width = cap_width
        self.cap_range = cap_range
        self.name = architecture
        self.tests_path = Path(__file__).parent / architecture
        assert self.tests_path.exists(), self.tests_path


MIPSConfig = ArchSpecificValues("MIPS", cap_range=64, cap_width=128,
                                purecap_lit_command_prefix=b"%cheri128_purecap_",
                                hybrid_lit_command_prefix=b"%cheri128_")
RISCV32Config = ArchSpecificValues("RISCV32", cap_range=32, cap_width=64,
                                   purecap_lit_command_prefix=b"%riscv32_cheri_purecap_",
                                   hybrid_lit_command_prefix=b"%riscv32_cheri_")
RISCV64Config = ArchSpecificValues("RISCV64", cap_range=64, cap_width=128,
                                   purecap_lit_command_prefix=b"%riscv64_cheri_purecap_",
                                   hybrid_lit_command_prefix=b"%riscv64_cheri_")


class CmdArgs(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--llvm-bindir", type=Path, required=True)
        parser.add_argument("--verbose", action="store_true")
        self.args = parser.parse_args()
        self.llvm_bindir = self.args.llvm_bindir  # type: Path
        if not self.llvm_bindir.exists():
            sys.exit("FATAL: --llvm-bindir does not exist")
        self.llc_cmd = (self.llvm_bindir / "llc")
        if not self.llc_cmd.exists():
            sys.exit("FATAL: Could not find llc in --llvm-bindir: " + str(self.llc_cmd))
        self.opt_cmd = (self.llvm_bindir / "opt")
        if not self.opt_cmd.exists():
            sys.exit("FATAL: Could not find opt in --llvm-bindir: " + str(self.opt_cmd))
        self.verbose = self.args.verbose  # type: bool


def update_one_test(test_name: str, input_file: typing.BinaryIO,
                    arch_def: ArchSpecificValues, args: CmdArgs):
    print("Updating", test_name, "for", arch_def.name)
    output_path = Path(arch_def.tests_path, test_name)
    with output_path.open("wb") as output_file:
        for line in input_file.readlines():
            converted_line = line.replace(b"%generic_cheri_purecap_",
                                          arch_def.purecap_lit_command_prefix)
            converted_line = converted_line.replace(b"%generic_cheri_hybrid_",
                                                    arch_def.hybrid_lit_command_prefix)
            converted_line = converted_line.replace(b"iCAPRANGE", b'i' + str(
                arch_def.cap_range).encode("utf-8"))
            converted_line = converted_line.replace(b"iCAPWIDTH", b'i' + str(
                arch_def.cap_width).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_BITS@", str(
                arch_def.cap_width / 8).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_BYTES@", str(
                arch_def.cap_width).encode("utf-8"))
            if args.verbose and converted_line != line:
                print("Adjusted line:")
                print("  Before:", line)
                print("  After: ", converted_line)
            output_file.write(converted_line)
    print("Wrote pre-processed test input to", output_file.name)

    # Generate the check lines using update_*_test_checks.py
    update_scripts_dir = LLVM_SRC_PATH / "utils"
    llc_checks_cmd = [sys.executable, str(update_scripts_dir / "update_llc_test_checks.py"),
                      "--llc-binary", str(args.llc_cmd), str(output_path)]
    opt_checks_cmd = [sys.executable, str(update_scripts_dir / "update_test_checks.py"),
                      "--function-signature", "--scrub-attributes",
                      "--opt-binary", str(args.opt_cmd), str(output_path)]
    for update_cmd in (llc_checks_cmd, opt_checks_cmd):
        # if args.verbose:
        print("Running", " ".join(update_cmd))
        subprocess.check_call(update_cmd)
    #/Users/alex/cheri/llvm-project/llvm/utils/update_llc_test_checks.py --verbose --llc-binary /Users/alex/cheri/llvm-project/cmake-build-debug/bin/llc /Users/alex/cheri/llvm-project/llvm/test/CodeGen/CHERI-Generic/MIPS/cheri-csub.ll

    # TODO: run update_test_checks.py


def main():
    args = CmdArgs()
    architectures = [MIPSConfig, RISCV32Config, RISCV64Config]
    # TODO: add command line flag to select subsets
    # TODO: add option to delete all files that don't exist in Inputs/ to handle renaming
    for test in (Path(__file__).parent / "Inputs").glob("*.ll"):
        for arch_def in architectures:
            with test.open("rb") as input_file:
                update_one_test(test.name, input_file, arch_def, args)


if __name__ == "__main__":
    main()
