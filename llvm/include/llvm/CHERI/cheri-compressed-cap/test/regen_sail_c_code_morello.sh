#!/usr/bin/env bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
set -xe

if ! command -v sail > /dev/null 2>/dev/null; then
  echo "Could not find sail in PATH."
  exit 1
fi

SAIL_DIR=$(sail -dir)
if [ ! -e "${SAIL_DIR}/lib/sail.c" ]; then
  echo "Invalid sail installation, missing library files"
  exit 1
fi

SAIL_MORELLO_DIR=${SAIL_MORELLO_DIR:-${HOME}/cheri/sail-morello}
if [ ! -d "$SAIL_MORELLO_DIR/src" ]; then
  echo "Must set SAIL_MORELLO_DIR"
  exit 1
fi

sail128_srcs="prelude.sail builtins.sail decode_start.sail v8_base.sail decode_end.sail stubs.sail no_devices.sail impdefs.sail"
keep_required_functions=(
  -c_preserve CapBoundsAddress
  -c_preserve CapBoundsUsesValue
  -c_preserve CapGetBounds
  -c_preserve CapGetBottom
  -c_preserve CapGetExponent
  -c_preserve CapGetObjectType
  -c_preserve CapGetPermissions
  -c_preserve CapGetTag
  -c_preserve CapGetTop
  -c_preserve CapGetValue
  -c_preserve CapIsInternalExponent
  -c_preserve CapIsRepresentable
  -c_preserve CapIsRepresentableFast
  -c_preserve CapSetBounds
  -c_preserve doCSetBounds
)
output_dir=${SCRIPT_DIR}/contrib

cd "$SAIL_MORELLO_DIR/src" || exit 1
sail -c -O -undefined_gen -non_lexical_flow -no_warn -c_no_main -c_prefix sailgen_ -c_specialize "${keep_required_functions[@]}" -verbose=2 -o "$output_dir/sail_compression_128m" $sail128_srcs "${SCRIPT_DIR}/compression_test_morello.sail" -static
cd "$output_dir"
