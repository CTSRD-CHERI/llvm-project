cmake_minimum_required(VERSION 3.8)
project(cheri_compressed_cap C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

add_compile_options(-pedantic -Wall -Wextra -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-c++98-c++11-compat-binary-literal -Wno-c11-extensions -Wno-padded)
# TODO: -Weverything
add_library(cheri_compressed_cap14 cheri_compressed_cap.c cheri_compressed_cap.h)

add_executable(decompress_c128_cap decompress_c128_cap.c)
add_executable(decompress_c128_cap23 decompress_c128_cap.c)
target_compile_definitions(decompress_c128_cap23 PRIVATE -DCC128_OLD_FORMAT)
add_executable(decompress_c128_cap14 decompress_c128_cap.c)
include(GNUInstallDirs)
install(TARGETS decompress_c128_cap RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS decompress_c128_cap23 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS decompress_c128_cap14 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

enable_testing()
add_executable(simple_test test/simple_test.cpp)
target_compile_options(simple_test PRIVATE -fsanitize=undefined,address)
target_link_libraries(simple_test PRIVATE -fsanitize=undefined,address)
add_test(NAME simple-test COMMAND simple_test)

add_executable(setbounds_test test/setbounds_test.cpp)
target_compile_options(setbounds_test PRIVATE -fsanitize=undefined,address)
target_link_libraries(setbounds_test PRIVATE -fsanitize=undefined,address)
add_test(NAME setbounds-test COMMAND setbounds_test)


add_executable(random_inputs_test test/random_inputs_test.cpp)
target_compile_options(random_inputs_test PRIVATE -fsanitize=undefined,address)
target_link_libraries(random_inputs_test PRIVATE -fsanitize=undefined,address)
add_test(NAME random-input-test COMMAND random_inputs_test)

if (NOT APPLE)
    add_executable(fuzz_decompress_asan test/fuzz-decompress.cpp)
    target_compile_options(fuzz_decompress_asan PRIVATE -fsanitize=undefined,address,fuzzer)
    target_link_libraries(fuzz_decompress_asan PRIVATE -fsanitize=undefined,address,fuzzer)
endif()
