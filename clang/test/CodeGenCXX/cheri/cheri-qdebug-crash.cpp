// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -target-abi purecap %s -std=c++14 -emit-llvm -O0 -o - | FileCheck -check-prefix PURECAP %s
// RUN: %cheri_cc1 -triple cheri-unknown-freebsd -target-abi n64 %s -std=c++14 -emit-llvm -O0 -o - | FileCheck -check-prefix HYBRID %s

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
  // HYBRID: call inreg { i64, i64 } @_Z11qSetPadCharv()
  // HYBRID: call dereferenceable(1) %class.QTextStream* @_ZlsR11QTextStream22QTextStreamManipulator(%class.QTextStream* dereferenceable(1) {{%.+}}, i64 inreg {{%.+}}, i64 inreg {{%.+}})
  // PURECAP: [[TMP:%.+]] = alloca %class.QTextStreamManipulator, align [[CAP_SIZE:(16|32)]], addrspace(200)
  // PURECAP: call void @_Z11qSetPadCharv(%class.QTextStreamManipulator addrspace(200)* sret [[TMP]])
  // PURECAP: call dereferenceable(1) %class.QTextStream addrspace(200)* @_ZlsU3capR11QTextStream22QTextStreamManipulator(%class.QTextStream addrspace(200)* dereferenceable(1) {{%.+}}, %class.QTextStreamManipulator addrspace(200)*  byval(%class.QTextStreamManipulator) align [[CAP_SIZE]] [[TMP]])
}

// TODO: we should probably return an inreg struct for CHERI too
// HYBRID: declare inreg { i64, i64 } @_Z11qSetPadCharv()
// PURECAP: declare dereferenceable(1) %class.QTextStream addrspace(200)* @_ZlsU3capR11QTextStream22QTextStreamManipulator(%class.QTextStream addrspace(200)* dereferenceable(1), %class.QTextStreamManipulator addrspace(200)* byval(%class.QTextStreamManipulator) align [[CAP_SIZE]])
// PURECAP: declare void @_Z11qSetPadCharv(%class.QTextStreamManipulator addrspace(200)* sret)
