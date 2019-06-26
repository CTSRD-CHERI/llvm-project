cmake_minimum_required(VERSION 3.8)
project(cheri_compressed_cap C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

add_compile_options(-pedantic -Wall -Wextra -Weverything -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-c++98-c++11-compat-binary-literal -Wno-c11-extensions -Wno-padded)
add_library(cheri_compressed_cap14 cheri_compressed_cap.c cheri_compressed_cap.h)
add_library(cheri_compressed_cap23 cheri_compressed_cap.c cheri_compressed_cap.h)
target_compile_definitions(cheri_compressed_cap23 PRIVATE -DCC128_OLD_FORMAT)



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
add_test(NAME simple-test COMMAND simple_test)

add_executable(setbounds_test test/setbounds_test.cpp)
add_test(NAME setbounds-test COMMAND setbounds_test)

add_executable(setbounds_test_old_format test/setbounds_test.cpp)
target_compile_definitions(setbounds_test_old_format PRIVATE -DCC128_OLD_FORMAT)
add_test(NAME setbounds-test-old-format COMMAND setbounds_test_old_format)

add_executable(simple_test_old_format test/simple_test.cpp)
target_compile_definitions(simple_test_old_format PRIVATE -DCC128_OLD_FORMAT)
add_test(NAME simple-test-old-format COMMAND simple_test_old_format)

add_executable(random_inputs_test test/random_inputs_test.cpp)
add_test(NAME random-input-test COMMAND random_inputs_test)
