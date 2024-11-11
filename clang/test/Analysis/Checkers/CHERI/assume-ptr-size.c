// RUN: %cheri_purecap_cc1 -analyze -verify %s \
// RUN:   -analyze -analyzer-checker=core,cheri.PointerSizeAssumptions


typedef __intcap_t intptr_t;
typedef __typeof__(sizeof(int)) size_t;
extern void *memcpy(void *dest, const void *src, size_t n);

void *f(long i64) {
  void *p;
  if (sizeof(i64) == sizeof(p)) { // expected-warning{{This code may fail to consider the case of 128-bit pointers}}
    memcpy(&p, &i64, sizeof(p));
  } else {
    int i32 = i64 & 0xffffffffL;
    memcpy(&p, &i32, sizeof(p));
  }
  return p;
}

void *f2(long i64){
  void *p;
  if( sizeof(i64)==i64 ){
    memcpy(&p, &i64, sizeof(p));
  }else{
    int i32 = i64 & 0xffffffffL;
    memcpy(&p, &i32, sizeof(p));
  }
  return p;
}

void *f3(long i64){
  void *p;
  if( sizeof(p)==8 ){ // expected-warning{{This code may fail to consider the case of 128-bit pointers}}
    memcpy(&p, &i64, sizeof(p));
  }else{
    int i32 = i64 & 0xffffffffL;
    memcpy(&p, &i32, sizeof(p));
  }
  return p;
}

void *f4(long i64){
  void *p;
  if( sizeof(p)==sizeof(long) ){ // expected-warning{{This code may fail to consider the case of 128-bit pointers}}
    memcpy(&p, &i64, sizeof(p));
  }else{
    int i32 = i64 & 0xffffffffL;
    memcpy(&p, &i32, sizeof(p));
  }
  return p;
}

void *f5(long i64){
  void *p;
  if( sizeof(p)==sizeof(i64) ){ // no warn
    memcpy(&p, &i64, sizeof(p));
  }else if (sizeof(p)==sizeof(int)) {
    int i32 = i64 & 0xffffffffL;
    memcpy(&p, &i32, sizeof(p));
  } else if (sizeof(p)==sizeof(intptr_t)) {
    p = 0;
  }
  return p;
}
