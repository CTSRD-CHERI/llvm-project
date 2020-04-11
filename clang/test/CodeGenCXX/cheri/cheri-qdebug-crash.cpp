// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -target-abi n64 %s -std=c++14 -emit-llvm -O0 -o - | %cheri_FileCheck -check-prefix HYBRID %s
// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -target-abi purecap %s -std=c++14 -emit-llvm -O0 -o - | %cheri_FileCheck -check-prefix PURECAP %s

// Reduced test case from a crash compiling qdebug.cpp (caused by passing pointers-to-members as arguments)

class QTextStream {};
class QDebug {
  void putUcs4();
private:
  QTextStream ts;
};
typedef void (QTextStream::*g)();
class QTextStreamManipulator {
  g i;
};
QTextStream& operator<<(QTextStream&, QTextStreamManipulator);
QTextStreamManipulator qSetPadChar();

void QDebug::putUcs4() {
  ts << qSetPadChar();
  // HYBRID: [[P2M:%.+]] = call inreg { i64, i64 } @_Z11qSetPadCharv()
  // HYBRID: call dereferenceable(1) %class.QTextStream* @_ZlsR11QTextStream22QTextStreamManipulator(%class.QTextStream* dereferenceable(1) {{%.+}}, i64 inreg {{%.+}}, i64 inreg {{%.+}})
  // PURECAP: [[P2M:%.+]] = call inreg { { i8 addrspace(200)*, i64 } } @_Z11qSetPadCharv()
  // TODO: we should be passing pointers-to-members inreg and not byval
  // PURECAP: call dereferenceable(1) %class.QTextStream addrspace(200)* @_ZlsU3capR11QTextStream22QTextStreamManipulator(%class.QTextStream addrspace(200)* dereferenceable(1) {{%.+}}, %class.QTextStreamManipulator addrspace(200)*  byval(%class.QTextStreamManipulator) align [[#CAP_SIZE]] {{%.+}})
}

// Note: pointers-to-members are also returned inreg for purecap now
// HYBRID: declare inreg { i64, i64 } @_Z11qSetPadCharv()
// PURECAP: declare inreg { { i8 addrspace(200)*, i64 } } @_Z11qSetPadCharv() addrspace(200)
