#!/usr/bin/env python3
"""A script to make FileCheck statements compatible with release builds of LLVM

IR names will be discarded and the numbering of variables is different so this
script replaces all CHECK: %... = ... with a FileCheck capture and all uses of
that name with the corresponding capture:

For example:
// CHECK: %memptr.virtualfn = load i32 (%class.A*)*, i32 (%class.A*)** %3, align 8
will become:
// CHECK: [[MEMPTR_VIRTUALFN:%.+]] = load i32 (%class.A*)*, i32 (%class.A*)** [[VAR3]], align 8

Usage: make_test_release_compatible.py -o new_test.c /path/to/test/old_test.c

To process multiple check prefixes use the -check-prefixes option (-- before the filename might be needed)
make_test_release_compatible.py -o new_test.c -check-prefixes CHECK OPTIMIZED -- /path/to/test/old_test.c
"""

import argparse
import re
import sys

assignmentRegex = re.compile(r"%([\w.\-]+) =")
referenceRegex = re.compile(r"%([\w.\-]+)")
print_debug_messages = False


def debug(*args, **kwargs):
    global print_debug_messages
    if print_debug_messages:
        print(*args, file=sys.stderr, **kwargs)


def replacement_var_name(varname: str):
    if all(c.isdigit() for c in varname):
        return "VAR" + varname
    return varname.replace(".", "_").upper()


def process_line(original_line: str, check_strings, output_file):
    if not any(s in original_line for s in check_strings):
        print(original_line, end="", file=output_file)
        return
    # debug("Found check string in ", line, end="")
    line = original_line
    assignment = re.search(assignmentRegex, line)
    if assignment:
        varname = assignment.group(1)
        debug("Found assignment", assignment, "to", varname, "in", original_line.strip())
        capture = "[[" + replacement_var_name(varname) + ":%.+]]"
        line = line[0:assignment.start()] + capture + " =" + line[assignment.end():]
        debug("  new assignment:", line.strip())
    # can't use finditer() here because that would break with multiple matches in one line
    search_index = 0
    i = 1
    while search_index < len(line):
        reference = referenceRegex.search(line, search_index)
        if not reference:
            break
        search_index = reference.end()
        varname = reference.group(1)
        if varname.startswith("."):
            continue  # this was created by previous replacement
        if any(varname.startswith(prefix) for prefix in ("\"", "class", "struct", "union")):
            debug("Ignoring reference", i,  reference)
            continue
        new_varname = replacement_var_name(varname)
        # debug("found ref:", reference, "->", new_varname, "in", line.strip())
        replacement = "[[" + new_varname + "]]"
        line = line[0:reference.start()] + replacement + line[reference.end():]
        debug("  new reference", i, line.strip())
        i += 1
        # we made a replacement so we need to take into account that the
        # replacement may be longer than the previous search index
        search_index = reference.start() + len(replacement)

    # debug(original_line)
    print(line, end="", file=output_file)


def main():
    global print_debug_messages
    parser = argparse.ArgumentParser()
    parser.add_argument("--debug", "-debug", action="store_true",
                        help="Print debug output")
    parser.add_argument("--output", "-o", default="-",
                        help="The output file for the updated test (default is stdout)")
    parser.add_argument("-check-prefixes", default=["CHECK"], nargs=argparse.ZERO_OR_MORE,
                        help="The check prefix used in FileCheck")
    parser.add_argument("file", default="CHECK", help="The test that should be made compatible")
    args = parser.parse_args()
    if args.debug:
        print_debug_messages = True

    check_strings = []
    for prefix in args.check_prefixes:
        check_strings.extend(prefix + s for s in (":", "-NEXT:", "-SAME:", "-NOT:"))

    with (open(args.output, 'w') if args.output != "-" else sys.stdout) as output_file:
        with open(args.file, "r") as f:
            for l in f.readlines():
                process_line(l, check_strings, output_file)

if __name__ == "__main__":
    main()
