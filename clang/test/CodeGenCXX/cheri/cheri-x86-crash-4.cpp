// XFAIL: *
// XXXAR: TODO: report upstream
// RUN: %clang_cc1 "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" "-mrelax-all" "-disable-free" "-main-file-name" "driver.cpp" "-mrelocation-model" "static" "-mthread-model" "posix" "-mdisable-fp-elim" "-fmath-errno" "-masm-verbose" "-mconstructor-aliases" "-munwind-tables" "-fuse-init-array" "-target-cpu" "x86-64" "-dwarf-column-info" "-debug-info-kind=limited" "-dwarf-version=4" "-debugger-tuning=gdb" "-coverage-notes-file" "/local/scratch/alr48/cheri/build/llvm-test-suite-native-build/Bitcode/Benchmarks/Halide/local_laplacian/CMakeFiles/halide_local_laplacian.dir/driver.cpp.gcno" "-sys-header-deps" "-D" "NDEBUG" "-Werror=date-time" "-w" "-std=c++11" "-fdeprecated-macro" "-ferror-limit" "19" "-fmessage-length" "0" "-fobjc-runtime=gcc" "-fcxx-exceptions" "-fexceptions" "-fdiagnostics-show-option" "-cheri-linker" "-x" "c++" "%s" -o -

template < typename > struct a 
template < typename > struct b
template < typename c > struct b< a< c  > 
       >
}
     a {
  ? b<a<int>>() b<a<int>>()
