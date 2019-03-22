// RUNs: %cheri_purecap_cc1 -munwind-tables -fuse-init-array -mllvm -cheri-cap-table-abi=pcrel -dwarf-column-info -debug-info-kind=line-tables-only -dwarf-version=5 -debugger-tuning=gdb -nobuiltininc -cheri-uintcap=offset -O2 -std=c++11 -mstack-alignment=16 -S %s -emit-llvm -o %s.opt.ll
// RUNs: %cheri_purecap_cc1 -munwind-tables -fuse-init-array -mllvm -cheri-cap-table-abi=pcrel -dwarf-column-info -debug-info-kind=standalone -dwarf-version=2 -debugger-tuning=gdb -nobuiltininc -cheri-uintcap=offset -O2 -std=c++11 -mstack-alignment=16 -o - -S %s
// RUN: %cheri_purecap_cc1 -cheri-uintcap=offset -O2 -std=c++11 -o - -S %s
// RUN: %cheri_purecap_cc1 -cheri-uintcap=offset -O2 -std=c++11 -o - -emit-llvm %s
// REQUIRES: clang

extern "C" __uintcap_t minimal_test_case(__int128 y) {
  __uintcap_t r;
  __builtin_memcpy(&r, (void *)(__uintcap_t)y, sizeof(r));
  return r;
}
extern "C" __uintcap_t minimal_test_case2(__int128 y) {
  __uintcap_t r;
  __builtin_memcpy(&r, (__uintcap_t*)(void *)(__uintcap_t)y, sizeof(r));
  return r;
}

extern "C" __uintcap_t minimal_test_case3(__int128 y) {
  __uintcap_t r;
  // This is out of bounds for CHERI256: __builtin_memcpy(&r, &y, sizeof(r));
  __builtin_memcpy(&r, &y, sizeof(y));
  return r;
}

// extern "C" __uintcap_t minimal_test_case2(char* buf) {
//   __uintcap_t r;
//   memcpy(&r, buf, sizeof(r));
//   return r;
// }
