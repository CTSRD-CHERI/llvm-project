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

SAIL_RISCV_DIR=${SAIL_RISCV_DIR:-${HOME}/cheri/sail-cheri-riscv/}
if [ ! -d "$SAIL_RISCV_DIR" ]; then
  echo "Must set SAIL_RISCV_DIR"
  exit 1
fi

sail_pre_srcs="sail-riscv/model/prelude.sail sail-riscv/model/prelude_mapping.sail"
sail128_srcs="$sail_pre_srcs sail-riscv/model/riscv_xlen64.sail sail-riscv/model/riscv_flen_D.sail src/cheri_prelude.sail src/cheri_types.sail src/cheri_prelude_128.sail src/cheri_cap_common.sail"
sail64_srcs="$sail_pre_srcs sail-riscv/model/riscv_xlen32.sail sail-riscv/model/riscv_flen_F.sail src/cheri_prelude.sail src/cheri_types.sail src/cheri_prelude_64.sail src/cheri_cap_common.sail"
output_dir=${SCRIPT_DIR}/contrib
keep_required_functions=(-c_preserve capToBits
	-c_preserve capToMemBits
	-c_preserve capBitsToCapability
	-c_preserve memBitsToCapability
	-c_preserve fastRepCheck
	-c_preserve setCapAddr
	-c_preserve setCapBounds
	-c_preserve getCapBounds
	-c_preserve getCapFlags
	-c_preserve setCapPerms
	-c_preserve capToString
	-c_preserve doCSetBounds
	-c_preserve getRepresentableLength
	-c_preserve getRepresentableAlignmentMask)

cd "$SAIL_RISCV_DIR"
# -c_no_rts
sail -c -c_no_main -c_prefix sailgen_ -c_specialize "${keep_required_functions[@]}" -verbose=2 -o "$output_dir/sail_compression_128" $sail128_srcs "${SCRIPT_DIR}/compression_test.sail" -static
sail -c -c_no_main -c_prefix sailgen_ -c_specialize "${keep_required_functions[@]}" -verbose=2 -o "$output_dir/sail_compression_64" $sail64_srcs "${SCRIPT_DIR}/compression_test.sail" -static
cd "$output_dir"

for i in sail.h sail.c sail_failure.c sail_failure.h elf.h rts.h; do
  cp "${SAIL_DIR}/lib/${i}" .
done
