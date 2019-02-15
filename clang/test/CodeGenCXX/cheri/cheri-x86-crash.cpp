// This should not crash (I think it still crashes upstream clang, because my IDE won't parse this file)
// TODO: verify and create bug report
// RUN: not %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-obj -mrelax-all -disable-free -main-file-name driver.cpp -mrelocation-model static -mthread-model posix -mdisable-fp-elim -fmath-errno -masm-verbose -mconstructor-aliases -munwind-tables -fuse-init-array -target-cpu x86-64 -dwarf-column-info -debug-info-kind=limited -dwarf-version=4 -debugger-tuning=gdb -coverage-notes-file /local/scratch/alr48/cheri/build/llvm-test-suite-native-build/Bitcode/Benchmarks/Halide/bilateral_grid/CMakeFiles/halide_bilateral_grid.dir/driver.cpp.gcno -sys-header-deps -D NDEBUG -Werror=date-time -w -std=c++11 -fdeprecated-macro -ferror-limit 19 -fmessage-length 0 -fobjc-runtime=gcc -fcxx-exceptions -fexceptions -fdiagnostics-show-option -cheri-linker -x c++ %s -o -
#define _GLIBCXX_BEGIN_NAMESPACE_LDBL_OR_CXX11 > use_facet;
template <money_get _GLIBCXX_BEGIN_NAMESPACE_LDBL_OR_CXX11 template <
    typename _CharT> template <> fn1() {
  _CharT &a = use_facet<_CharT>;
a
