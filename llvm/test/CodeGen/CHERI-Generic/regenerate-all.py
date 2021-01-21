#!/usr/bin/env python3
import argparse
import subprocess
import sys
import typing
from pathlib import Path

import math

LLVM_SRC_PATH = Path(__file__).parent.parent.parent.parent


class ArchSpecificValues(object):
    def __init__(self, architecture: str, *, cap_range, cap_width,
                 common_args: list, hybrid_sf_args: list, hybrid_hf_args: list,
                 purecap_sf_args: list, purecap_hf_args: list, datalayout: bytes,
                 base_architecture: str = None):
        self.hybrid_datalayout = datalayout
        self.purecap_datalayout = datalayout + b"-A200-P200-G200"
        self.hybrid_softfloat_args = (" ".join(common_args + hybrid_sf_args)).encode("utf-8")
        self.hybrid_hardfloat_args = (" ".join(common_args + hybrid_hf_args)).encode("utf-8")
        self.purecap_softfloat_args = (" ".join(common_args + purecap_sf_args)).encode("utf-8")
        self.purecap_hardfloat_args = (" ".join(common_args + purecap_hf_args)).encode("utf-8")
        self.cap_width = cap_width
        self.cap_range = cap_range
        self.name = architecture
        self.base_name = base_architecture if base_architecture is not None else architecture
        self.tests_path = Path(__file__).parent / architecture
        assert self.tests_path.exists(), self.tests_path


MIPSConfig = ArchSpecificValues(
    "MIPS", cap_range=64, cap_width=128,
    common_args=["-mtriple=mips64", "-mcpu=cheri128", "-mattr=+cheri128",
                 "--relocation-model=pic"],
    hybrid_sf_args=["-target-abi", "n64", "-mattr=+soft-float"],
    hybrid_hf_args=["-target-abi", "n64"],
    purecap_sf_args=["-target-abi", "purecap", "-mattr=+soft-float"],
    purecap_hf_args=["-target-abi", "purecap"],
    datalayout=b"E-m:e-pf200:128:128:128:64-i8:8:32-i16:16:32-i64:64-n32:64-S128")
RISCV32Config = ArchSpecificValues(
    "RISCV32", base_architecture="RISCV", cap_range=32, cap_width=64,
    common_args=["-mtriple=riscv32", "--relocation-model=pic"],
    hybrid_sf_args=["-target-abi", "ilp32", "-mattr=+xcheri,-f"],
    hybrid_hf_args=["-target-abi", "ilp32f", "-mattr=+xcheri,+f"],
    purecap_sf_args=["-target-abi", "il32pc64", "-mattr=+xcheri,+cap-mode,-f"],
    purecap_hf_args=["-target-abi", "il32pc64f", "-mattr=+xcheri,+cap-mode,+f"],
    datalayout=b"e-m:e-pf200:64:64:64:32-p:32:32-i64:64-n32-S128")
RISCV64Config = ArchSpecificValues(
    "RISCV64", base_architecture="RISCV", cap_range=64, cap_width=128,
    common_args=["-mtriple=riscv64", "--relocation-model=pic"],
    hybrid_sf_args=["-target-abi", "lp64", "-mattr=+xcheri,-f,-d"],
    hybrid_hf_args=["-target-abi", "lp64d", "-mattr=+xcheri,+f,+d"],
    purecap_sf_args=["-target-abi", "l64pc128", "-mattr=+xcheri,+cap-mode,-f,-d"],
    purecap_hf_args=["-target-abi", "l64pc128d", "-mattr=+xcheri,+cap-mode,+f,+d"],
    datalayout=b"e-m:e-pf200:128:128:128:64-p:64:64-i64:64-i128:128-n64-S128")

ALL_ARCHITECTURES = [MIPSConfig, RISCV32Config, RISCV64Config]
ALL_ARCHITECTURE_IF_STRS = set([b"@IF-" + arch_def.name.encode() + b"@" for arch_def in ALL_ARCHITECTURES] + [
                            b"@IF-" + arch_def.base_name.encode() + b"@" for arch_def in ALL_ARCHITECTURES])
ALL_ARCHITECTURE_IFNOT_STRS = set([b"@IFNOT-" + arch_def.name.encode() + b"@" for arch_def in ALL_ARCHITECTURES] + [
                               b"@IFNOT-" + arch_def.base_name.encode() + b"@" for arch_def in ALL_ARCHITECTURES])


class CmdArgs(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--llvm-bindir", type=Path, required=True)
        parser.add_argument("--verbose", action="store_true")
        parser.add_argument("tests", default=[], nargs=argparse.ZERO_OR_MORE)
        self.args = parser.parse_args()
        print(self.args)
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
    manual_checks_only = False
    current_arch_if = b"@IF-" + arch_def.name.encode() + b"@"
    current_arch_base_if = b"@IF-" + arch_def.base_name.encode() + b"@"
    current_arch_ifnot = b"@IFNOT-" + arch_def.name.encode() + b"@"
    current_arch_base_ifnot = b"@IFNOT-" + arch_def.base_name.encode() + b"@"
    update_llc_args = []
    update_opt_args = ["--function-signature", "--scrub-attributes"]
    with output_path.open("wb") as output_file:
        output_file.write(b"; DO NOT EDIT -- This file was generated from " + str(
            Path(input_file.name).relative_to(LLVM_SRC_PATH)).encode("utf-8") + b"\n")
        for line in input_file.readlines():
            if b"!DO NOT AUTOGEN!" in line:
                manual_checks_only = True
                continue
            # Handle @IF-<arch>@ and @IFNOT-<arch>@ prefixes:
            if line.startswith(b"@IF-"):
                if line.startswith(current_arch_if):
                    print("REMOVING", current_arch_if, "from line: ", line)
                    line = line[len(current_arch_if):]
                elif line.startswith(current_arch_base_if):
                    print("REMOVING", current_arch_base_if, "from line: ", line)
                    line = line[len(current_arch_base_if):]
                elif any(line.startswith(prefix) for prefix in ALL_ARCHITECTURE_IF_STRS):
                    print("Ignoring @IF- directive for other architecture: ", line)
                    continue
                else:
                    sys.exit("Invalid @IF- directive: " + line.decode("utf-8"))
            elif line.startswith(b"@IFNOT-"):
                if line.startswith(current_arch_ifnot):
                    print("Skipping", current_arch_ifnot, "line: ", line)
                    continue
                elif line.startswith(current_arch_base_ifnot):
                    print("Skipping", current_arch_base_ifnot, "line: ", line)
                    continue
                valid_directive = False
                for prefix in ALL_ARCHITECTURE_IFNOT_STRS:
                    if line.startswith(prefix):
                        print("Removing @IFNOT- directive for other architecture: ", line)
                        line = line[len(prefix):]
                        valid_directive = True
                        break
                if not valid_directive:
                    sys.exit("Invalid @IF- directive: " + line.decode("utf-8"))
            elif line.startswith(b"; UPDATE_OPT_ARGS:"):
                argstr = line[len(b"; UPDATE_OPT_ARGS:"):].decode("utf-8")
                update_opt_args = [x.strip() for x in argstr.split()]
                print("Set update_test_checks.py args to: ", update_opt_args)
                continue
            elif line.startswith(b"; UPDATE_LLC_ARGS:"):
                argstr = line[len(b"; UPDATE_LLC_ARGS:"):].decode("utf-8")
                update_llc_args = [x.strip() for x in argstr.split()]
                print("Set update_llc_test_checks.py args to: ", update_llc_args)
                continue

            converted_line = line.replace(b"iCAPRANGE", b"i" + str(
                arch_def.cap_range).encode("utf-8"))
            converted_line = converted_line.replace(b"iCAPWIDTH", b"i" + str(
                arch_def.cap_width).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_BITS@", str(
                arch_def.cap_width).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_BYTES@", str(
                int(arch_def.cap_width // 8)).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_BYTES_P2@", str(
                int(math.log2(arch_def.cap_width / 8))).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_RANGE_BITS@", str(
                arch_def.cap_range).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_RANGE_BYTES@", str(
                int(arch_def.cap_range // 8)).encode("utf-8"))
            converted_line = converted_line.replace(b"@CAP_RANGE_BYTES_P2@", str(
                int(math.log2(arch_def.cap_range / 8))).encode("utf-8"))
            # Opt tests require a datalayout since the lit substitutions don't
            # include it in their commandline
            converted_line = converted_line.replace(b"@PURECAP_DATALAYOUT@", arch_def.purecap_datalayout)
            converted_line = converted_line.replace(b"@HYBRID_DATALAYOUT@", arch_def.hybrid_datalayout)
            converted_line = converted_line.replace(b"@HYBRID_SOFTFLOAT_ARGS@", arch_def.hybrid_softfloat_args)
            converted_line = converted_line.replace(b"@HYBRID_HARDFLOAT_ARGS@", arch_def.hybrid_hardfloat_args)
            converted_line = converted_line.replace(b"@PURECAP_SOFTFLOAT_ARGS@", arch_def.purecap_softfloat_args)
            converted_line = converted_line.replace(b"@PURECAP_HARDFLOAT_ARGS@", arch_def.purecap_hardfloat_args)
            if args.verbose and converted_line != line:
                print("Adjusted line:")
                print("  Before:", line)
                print("  After: ", converted_line)
            output_file.write(converted_line)
    print("Wrote pre-processed test input to", output_file.name)

    if manual_checks_only:
        print("Not auto-generating check lines.")
        return

    # Generate the check lines using update_*_test_checks.py
    update_scripts_dir = LLVM_SRC_PATH / "utils"
    llc_checks_cmd = [sys.executable, str(update_scripts_dir / "update_llc_test_checks.py"),
                      "--llc-binary", str(args.llc_cmd),
                      "--opt-binary", str(args.opt_cmd),
                      "--force-update"] + update_llc_args + [str(output_path)]
    opt_checks_cmd = [sys.executable, str(update_scripts_dir / "update_test_checks.py"),
                      "--opt-binary", str(args.opt_cmd),
                      "--force-update"] + update_opt_args + [str(output_path)]
    for update_cmd in (llc_checks_cmd, opt_checks_cmd):
        # if args.verbose:
        print("Running", " ".join(update_cmd))
        subprocess.check_call(update_cmd)


def main():
    options = CmdArgs()
    architectures = ALL_ARCHITECTURES
    # TODO: add command line flag to select subsets
    # TODO: add option to delete all files that don't exist in Inputs/ to handle renaming
    if options.args.tests:
        options.args.tests = [Path(x) for x in options.args.tests]
    else:
        options.args.tests = [Path(__file__).parent / "Inputs"]
    all_input_files = []
    for t in options.args.tests:
        if t.is_dir():
            all_input_files.extend(t.glob("*.ll"))
        else:
            all_input_files.append(t)
    for test in all_input_files:
        if not test.parent.name == "Inputs":
            print("Skipping non-Inputs path:", test)
            continue
        for arch_def in architectures:
            with test.open("rb") as input_file:
                update_one_test(test.name, input_file, arch_def, options)


if __name__ == "__main__":
    main()
