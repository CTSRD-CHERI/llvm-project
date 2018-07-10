// XXXAR: This was created by creduce from another crash bug. Looks like an upstream bug
// TODO: report upstream
// XFAIL: *
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-obj -mrelax-all -disable-free -main-file-name driver.cpp -mrelocation-model static -mthread-model posix -mdisable-fp-elim -fmath-errno -masm-verbose -mconstructor-aliases -munwind-tables -fuse-init-array -target-cpu x86-64 -dwarf-column-info -debug-info-kind=limited -dwarf-version=4 -debugger-tuning=gdb -coverage-notes-file /local/scratch/alr48/cheri/build/llvm-test-suite-native-build/Bitcode/Benchmarks/Halide/bilateral_grid/CMakeFiles/halide_bilateral_grid.dir/driver.cpp.gcno -sys-header-deps -D NDEBUG -Werror=date-time -w -std=c++11 -fdeprecated-macro -ferror-limit 19 -fmessage-length 0 -fobjc-runtime=gcc -fcxx-exceptions -fexceptions -fdiagnostics-show-option -cheri-linker -x c++ %s -o - | FileCheck %s
template <_Alloc> template <a...> void b template <a _Alloc> b<_Tp_Alloc>
