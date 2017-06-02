// RUN: %cheri_cc1 -target-abi purecap %s -o /dev/null
// previously this would assert because __uintcap_t was not seen as a valid builtin operator overload

typedef __uintcap_t quintptr;

struct QMapNodeBase
{
    quintptr p;
    QMapNodeBase *left;
    QMapNodeBase *right;

    enum Color { Red = 0, Black = 1 };
    enum { Mask = 3 }; // reserve the second bit as well

    Color color() const { return Color(p & 1); }
    void setColor(Color c) { if (c == Black) p |= Black; else p &= ~Black; } // expected-warning {{binary expression on capability and non-capability types}}
    QMapNodeBase *parent() const { return reinterpret_cast<QMapNodeBase *>(p & ~Mask); } // expected-warning {{binary expression on capability and non-capability types}}
    void setParent(QMapNodeBase *pp) { p = (p & Mask) | quintptr(pp); }
};
