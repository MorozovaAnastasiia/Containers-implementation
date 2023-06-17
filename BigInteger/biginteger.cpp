#include "biginteger.h"

BigInteger operator+(const BigInteger& a, const BigInteger& b) {
    BigInteger res(a);
    res += b;
    return res;
}
BigInteger operator-(const BigInteger& a, const BigInteger& b) {
    BigInteger res(a);
    res -= b;
    return res;
}
BigInteger operator*(const BigInteger& a, const BigInteger& b) {
    BigInteger res(a);
    res *= b;
    return res;
}
BigInteger operator/(const BigInteger& a, const BigInteger& b) {
    BigInteger res(a);
    res /= b;
    return res;
}
BigInteger operator%(const BigInteger& a, const BigInteger& b) {
    BigInteger res(a);
    res %= b;
    return res;
}
bool operator==(const BigInteger& ai, const BigInteger& bi) {
    BigInteger copya = ai;
    copya.RemoveExtra();
    BigInteger copyb = bi;
    copyb.RemoveExtra();
    if (ai.GetPlus() != bi.GetPlus()) {
        return false;
    }
    int sizea = copya.GetSize();
    int sizeb = copyb.GetSize();
    if (sizea != sizeb) {
        return false;
    }
    for (int i = 0; i < sizea; ++i) {
        if (ai.GetIndex(i) != bi.GetIndex(i)) {
            return false;
        }
    }
    return true;
}
bool operator!=(const BigInteger& ai, const BigInteger& bi) {
    return !(ai == bi);
}
std::ostream& operator<<(std::ostream& os, const BigInteger& bi) {
    return os << bi.toString();
}
std::istream& operator>>(std::istream& is, BigInteger& bi) {
    std::string ss;
    is >> ss;
    bi = BigInteger(ss);
    return is;
}
BigInteger operator"" _bi(const char* str, size_t /*unused*/) {
    return BigInteger(str);
}
Rational operator/(const Rational& ra, const Rational& rb) {
    Rational cop(ra);
    cop /= rb;
    return cop;
}
Rational operator*(const Rational& ra, const Rational& rb) {
    Rational cop(ra);
    cop *= rb;
    return cop;
}
Rational operator+(const Rational& ra, const Rational& rb) {
    Rational cop(ra);
    cop += rb;
    return cop;
}
Rational operator-(const Rational& ra, const Rational& rb) {
    Rational cop(ra);
    cop -= rb;
    return cop;
}
