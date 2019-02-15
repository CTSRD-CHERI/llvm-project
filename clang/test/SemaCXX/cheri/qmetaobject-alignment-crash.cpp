// This previously crashed because of wrong value of SuitableAlign in Mips.h
// RUN: %cheri_purecap_cc1 -fuse-init-array -target-cpu mips4 -mllvm -cheri-cap-table-abi=pcrel -mllvm -mxcaptable=true -fcxx-exceptions -fexceptions -fcolor-diagnostics -o /dev/null -O0 -emit-llvm -disable-O0-optnone  %s
class a {
public:
  virtual ~a();
};
const int i = __BIGGEST_ALIGNMENT__;
const int *j = &i;
void b() {
  throw a();
}
