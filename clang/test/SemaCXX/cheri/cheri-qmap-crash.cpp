// RUN: %cheri_purecap_cc1 %s -verify
// previously this would assert because __uintcap_t was not seen as a valid builtin operator overload

typedef __uintcap_t quintptr;

struct QMapNodeBase
{
    quintptr p;
    QMapNodeBase *left;
    QMapNodeBase *right;

    enum Color { Red = 0, Black = 1 };
    enum { Mask = 3 }; // reserve the second bit as well

    Color color() const { return Color(p & 1); } // expected-warning {{using bitwise and on capability types may give surprising results}}
    void setColor(Color c) { if (c == Black) p |= Black; else p &= ~Black; } // expected-warning {{using bitwise and on capability types may give surprising results}}
    QMapNodeBase *parent() const { return reinterpret_cast<QMapNodeBase *>(p & ~Mask); } // expected-warning {{using bitwise and on capability types may give surprising results}}
    void setParent(QMapNodeBase *pp) { p = (p & Mask) | quintptr(pp); } // expected-warning {{using bitwise and on capability types may give surprising results}}

    bool comparisonOp() const { return (p & Mask) == Black; } // expected-warning {{using bitwise and on capability types may give surprising results}}
    bool comparisonOp2() const { return Red == (p & Mask); } // expected-warning {{using bitwise and on capability types may give surprising results}}

    bool capRHS() const { return (Black | p) == p; } // expected-warning{{binary expression on capability and non-capability types: 'int' and 'quintptr' (aka '__uintcap_t')}}
};
