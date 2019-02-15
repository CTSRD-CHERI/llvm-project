// RUN:  %cheri_purecap_cc1 -emit-llvm -std=c++11 -o - %s | FileCheck %s
struct tuple_leaf {
  int& value;
  tuple_leaf(int& t) : value(t) {}
};

struct tuple : public tuple_leaf {
  tuple(int u) : tuple_leaf(u) {}
};

struct  pair {
  int first;
  pair(tuple t) {
    first = t.value;
  }
};

int main() {
  // CHECK-NOT: { i64, i64, i64, i64 }
  pair p(tuple(1));
  return 0;
}
