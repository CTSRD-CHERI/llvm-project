// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -target-abi purecap %s -std=c++14 -emit-llvm -o -
// XFAIL: *
// Bitfields with underlying type __uintcap_t used to cause assertions. Found while compiling qdatetime.cpp for purecap ABI

typedef __intcap_t qintptr;
typedef __uintcap_t quintptr;

struct ShortData {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        quintptr status : 8;
#endif

        qintptr msecs : sizeof(void *) * 8 - 8;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        quintptr status : 8;
#endif
};

class QDateTime {
  void setMsecs(long f);
  struct {
    ShortData data;
  } d;
};

void QDateTime::setMsecs(long f) {
  d.data.msecs = f;
  d.data.msecs = qintptr(f);
  d.data.status = 24;
}
