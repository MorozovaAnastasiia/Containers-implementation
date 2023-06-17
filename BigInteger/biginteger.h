#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
class BigInteger {
  private:
    bool plus = true;
    std::vector<long long> digits;
    static const int base = 1000000000;

  public:
    friend bool operator==(const BigInteger& ai, const BigInteger& bi);
    friend bool operator!=(const BigInteger& ai, const BigInteger& bi);
    friend BigInteger operator*(const BigInteger& a, const BigInteger& b);
    friend BigInteger operator/(const BigInteger& a, const BigInteger& b);
    friend BigInteger operator-(const BigInteger& a, const BigInteger& b);
    friend BigInteger operator+(const BigInteger& a, const BigInteger& b);
    friend BigInteger operator%(const BigInteger& a, const BigInteger& b);
    friend BigInteger operator"" _bi(const char* str, size_t /*unused*/);
    void WithZero(int n) {
        for (int i = n - digits.size(); i != 0; --i) {
            digits.push_back(0);
        }
    }
    void SetPlus(bool sign) {
        plus = sign;
    }
    // конструктор по умолчанию
    BigInteger() = default;
    // конструктор от int
    BigInteger(const int x) {
        digits.clear();
        int cop = x;
        if (x == 0) {
            plus = true;
            digits = std::vector<long long>(1, 0);
        } else {
            if (x < 0) {
                plus = false;
            }
            if (cop < 0) {
                cop = -cop;
            }
            while (cop > 0) {
                long long remnant = cop % base;
                digits.push_back(remnant);
                cop /= base;
            }
        }
    }
    // конструктор от string
    BigInteger(std::string s) {
        // digits.clear();
        if (s.length() == 0) {
            digits = std::vector<long long>(1, 0);
            plus = true;
        } else {
            plus = true;
            if (s[0] == '-') {
                plus = false;
                s = s.substr(1);
            }
            int i = s.length();
            while (i > 0) {
                if (i < 9) {
                    auto ks = s.substr(0, i);
                    int numm = std::stoi(ks);
                    digits.push_back(numm);
                } else {
                    auto ks = s.substr(i - 9, 9);
                    int numm = std::stoi(ks);
                    digits.push_back(numm);
                }
                i -= 9;
            }
            this->RemoveExtra();
        }
        plus = !plus;
        if (!(GetSize() == 0 && GetIndex(0) == 0)) {
            plus = !plus;
        }
    }
    void RemoveExtra() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
    }
    int GetSize() {
        return digits.size();
    }
    bool GetPlus() const {
        return plus;
    }
    long long GetIndex(int idx) const {
        return digits[idx];
    }
    // bool operator!=(const BigInteger& bi) { return !(*this == bi); }
    friend std::ostream& operator<<(std::ostream& os, const BigInteger& bi);
    friend std::istream& operator>>(std::istream& is, BigInteger& bi);
    // toString
    // унарный минус
    BigInteger operator-() const {
        BigInteger cop = *this;
        if (cop.GetSize() == 1 && cop.GetIndex(0) == 0) {
            return cop;
        }
        if (!cop.plus) {
            cop.plus = true;
        }
        return cop;
    }
    explicit operator bool() {
        return (!((*this).GetSize() == 1 && (*this).GetIndex(0) == 0));
    }
    BigInteger& operator*=(BigInteger bi) {
        BigInteger res = 0;
        int sizethis = (*this).GetSize();
        int sizebi = bi.GetSize();
        int ssize = 5 * std::max(sizethis, 2 * sizebi);
        if (bi.plus != plus) {
            res.plus = false;
        }
        res.WithZero(ssize);
        for (int i = 0; i < (*this).GetSize(); ++i) {
            for (int j = 0; j < bi.GetSize(); ++j) {
                // переполнение при умножении
                res.digits[i + j + 1] += ((*this).GetIndex(i) * bi.GetIndex(j) +
                                          res.GetIndex(i + j)) /
                                         base;
                res.digits[i + j] = ((*this).GetIndex(i) * bi.GetIndex(j) +
                                     res.GetIndex(i + j)) %
                                    base;
            }
        }
        res.RemoveExtra();
        if (res.GetSize() == 1 && res.GetIndex(0) == 0) {
            plus = true;
        }
        *this = res;
        return *this;
    }
    std::string toString() const {
        std::string res;
        if (!plus) {
            res = '-';
            long long num = digits[digits.size() - 1];
            res += std::to_string(num);
        } else {
            long long num = digits[digits.size() - 1];
            res += std::to_string(num);
        }
        for (int i = digits.size() - 2; i >= 0; i--) {
            if (std::to_string(GetIndex(i)).length() > 8) {
                std::string sub;
                sub = std::to_string(GetIndex(i));
                res += sub;
            } else {
                std::string ks = std::to_string(GetIndex(i));
                long long p = 9 - ks.length();
                std::string sub;
                for (long long i = 0; i < p; ++i) {
                    sub += '0';
                }
                std::string fs = std::to_string(GetIndex(i));
                std::string ss = fs;
                res += sub;
                res += ss;
            }
        }
        return res;
    }
    bool operator<(const BigInteger& v) const {
        return v > *this;
    }
    bool operator<=(const BigInteger& v) const {
        return !(v < *this);
    }
    bool operator>=(const BigInteger& v) const {
        return !(*this < v);
    }
    bool operator>(const BigInteger& other) const {
        BigInteger a = *this;
        BigInteger b = other;
        if (a.plus && !b.plus) {
            return true;
        }
        if (!a.plus && b.plus) {
            return false;
        }
        if (a.plus && b.plus) {
            if (a.GetSize() > b.GetSize()) {
                return true;
            }
            if (b.GetSize() > a.GetSize()) {
                return false;
            }
            for (int i = a.GetSize() - 1; i >= 0; --i) {
                if (a.GetIndex(i) > b.GetIndex(i)) {
                    return true;
                }
                if (a.GetIndex(i) < b.GetIndex(i)) {
                    return false;
                }
            }
            return false;
        }
        // !a.plus && !b.plus
        if (a.GetSize() > b.GetSize()) {
            return false;
        }
        if (b.GetSize() > a.GetSize()) {
            return true;
        }
        for (int i = a.GetSize() - 1; i >= 0; --i) {
            if (a.GetIndex(i) > b.GetIndex(i)) {
                return false;
            }
            if (a.GetIndex(i) < b.GetIndex(i)) {
                return true;
            }
        }
        return false;
    }
    BigInteger& operator/=(const BigInteger bi) {
        BigInteger cop = bi;
        cop.SetPlus(true);
        BigInteger res;
        int necsize = GetSize();
        res.digits.resize(necsize);
        BigInteger symb;
        for (long long i = GetSize() - 1; i >= 0; i--) {
            BigInteger forbase(1000000000);
            symb *= forbase;
            symb.digits[0] = digits[i];
            symb.RemoveExtra();
            long long ba = 1000000000;
            long long x = base, left = 0, right = ba;
            while (left <= right) {
                int middle = (left + right) / 2;
                // std::cout << left << " " << right
                //<< " " << cop * BigInteger(middle) << " "
                //<< symb << '\n';
                BigInteger mid = (BigInteger(middle));
                if (cop * mid > symb) {
                    right = middle - 1;
                } else {
                    left = middle + 1;
                    x = middle;
                }
            }
            res.digits[i] = x;
            BigInteger rem = cop * BigInteger(x);
            symb -= rem;
        }
        res.plus = (plus == bi.plus);
        res.RemoveExtra();
        if (res.GetSize() == 1 && res.GetIndex(0) == 0) {
            res.plus = true;
        }
        *this = res;
        return *this;
    }
    // todo
    BigInteger& operator+=(const BigInteger& other) {
        int add = 0;
        if (plus == other.plus) {
            int ssize = other.digits.size();
            for (int i = 0; i < ssize || add > 0; ++i) {
                int dsize = digits.size();
                if (i == dsize) {
                    digits.push_back(0);
                }
                ssize = other.digits.size();
                digits[i] += add + (i < ssize ? other.digits[i] : 0);
                if (digits[i] >= base) {
                    add = 1;
                } else {
                    add = 0;
                }
                if (digits[i] >= base) {
                    digits[i] -= base;
                }
            }
        } else if (!(other.digits.size() == 1 && other.GetIndex(0) == 0)) {
            *this -= -other;
        }
        return *this;
    }
    BigInteger& operator%=(const BigInteger& bi) {
        BigInteger wholepart = bi * (*this / bi);
        *this -= wholepart;
        return *this;
    }
    BigInteger operator%(const BigInteger& bi) {
        BigInteger divis = (*this / bi);
        return *this - bi * divis;
    }
    BigInteger& operator-=(const BigInteger& bi) {
        if (*this == bi) {
            plus = true;
            while (digits.size() > 1) {
                digits.pop_back();
            }
            digits[0] = 0;
            return *this;
        }
        if (bi.plus != plus) {
            if (bi.plus and !plus) {
                BigInteger ai = bi;
                ai -= *this;
                ai.plus = false;
                *this = ai;
                return *this;
            } else {
                BigInteger ai = bi;
                ai.plus = true;
                *this += ai;
                return *this;
            }
        }
        int cursize = this->digits.size();
        BigInteger ai = bi;
        int othersize = ai.digits.size();
        int ssize = std::max(cursize, othersize);
        ai.WithZero(ssize);
        this->WithZero(ssize);
        if (!plus) {
            plus = true;
            ai.plus = !ai.plus;
            *this -= ai;
        } else {
            if (*this > ai) {
                for (int i = 0; i < othersize; i++) {
                    if (GetIndex(i) >= ai.GetIndex(i)) {
                        this->digits[i] -= ai.GetIndex(i);
                    } else {
                        this->digits[i] += base - ai.GetIndex(i);
                        --this->digits[i + 1];
                    }
                }
                if (othersize < GetSize() && digits[othersize] == -1) {
                    for (int i = othersize; i < (*this).GetSize(); ++i) {
                        if (digits[i] > 0) {
                            digits[i]--;
                            break;
                        } else {
                            digits[i] = base - 1;
                        }
                    }
                }
                this->RemoveExtra();
                return *this;
            }
            ai -= *this;
            *this = ai;
        }
        plus = !plus;
        return *this;
    }
    BigInteger& operator++() {
        return (*this += 1);
    }
    BigInteger operator++(int) {
        BigInteger cop = *this;
        *this += 1;
        return cop;
    }
    BigInteger& operator--() {
        return (*this += 1);
    }
    BigInteger operator--(int) {
        BigInteger cop = *this;
        *this -= 1;
        return cop;
    }
    BigInteger abs() const {
        return plus ? *this : -*this;
    }
};

// Rational
class Rational {
  private:
    BigInteger chis;
    BigInteger znam;

  public:
    Rational() = default;
    Rational(const int& x, const int& y) {
        chis = x;
        znam = y;
    }
    Rational(const int x) {
        chis = BigInteger(x);
        znam = BigInteger(1);
    }
    Rational(const BigInteger& x, const BigInteger& y) {
        chis = x;
        znam = y;
    }
    Rational(const BigInteger& x) {
        chis = x;
        znam = 1;
    }
    Rational operator-() const {
        Rational a = *this;
        a.chis *= -1;
        a.Smaller();
        return a;
    }
    static BigInteger GetCommon(BigInteger a, BigInteger b) {
        a.SetPlus(true);
        b.SetPlus(true);
        while (b > 0) {
            // std::cout << a << " " << b << '\n';
            a = a % b;
            BigInteger tmp;
            tmp = a;
            a = b;
            b = tmp;
        }
        return a;
    }
    void Smaller() {
        BigInteger chisnew, znamnew;
        chisnew = chis;
        znamnew = znam;
        BigInteger com = GetCommon(znamnew, chisnew);
        znam /= com;
        chis /= com;
        if (this->chis == BigInteger(0) || (chis.GetPlus() && znam.GetPlus()) ||
            (!chis.GetPlus() && !znam.GetPlus())) {
            znam.SetPlus(true);
            chis.SetPlus(true);
        } else {
            znam.SetPlus(true);
            chis.SetPlus(false);
        }
    }
    std::string toString() {
        (*this).Smaller();
        if (znam.GetSize() == 1 && znam.GetIndex(0) == 1) {
            return chis.toString();
        } else {
            auto fpart = chis.toString();
            auto spart = znam.toString();
            return fpart + '/' + spart;
        }
    }
    // todo
    std::string asDecimal(int precision = 0) const {
        BigInteger add, whole;
        BigInteger copchis, copznam;
        copchis = chis;
        copznam = znam;
        whole = (copchis / copznam);
        std::string sign;
        add = BigInteger(1);
        if ((copchis == 0 || (whole == BigInteger(0))) &&
            (copchis.GetPlus() != copznam.GetPlus())) {
            sign = "-";
        }
        sign += whole.toString();
        if (precision == 0) {
            return sign;
        }
        // powing
        for (int i = 0; i < precision; i++) {
            add *= 10;
        }
        // тогда есть дробная часть (возможно заполним нулями)
        sign += '.';
        whole.SetPlus(true);
        copchis.SetPlus(true);
        copznam.SetPlus(true);
        auto renewed = (copchis * add) / copznam;
        auto oth = whole * add;
        std::string drob = (renewed - oth).toString();
        long long drobsize = drob.length();
        if (drob == "0") {
            for (int i = 0; i < precision; ++i) {
                sign += '0';
                drobsize++;
            }
        } else {
            // не хватает точности - дозаполним нулями
            if (precision > drobsize) {
                for (int i = 0; i < precision - drobsize; ++i) {
                    sign += '0';
                }
                sign += drob;
            } else {
                sign += drob;
            }
        }

        return sign;
    }
    explicit operator bool() const {
        Rational cop = *this;
        return cop != 0;
    }
    bool operator<(const Rational ra) const {
        Rational cop = *this;
        Rational copra = ra;
        cop.Smaller();
        copra.Smaller();
        return cop.chis * copra.znam < cop.znam * copra.chis;
    }
    bool operator>(const Rational ra) const {
        return ra < (*this);
    }
    bool operator>=(const Rational ra) const {
        return !((*this) < ra);
    }
    bool operator<=(const Rational ra) const {
        return !((*this) > ra);
    }
    bool operator==(const Rational ra) {
        Rational cop = *this;
        Rational copra = ra;
        cop.Smaller();
        copra.Smaller();
        bool rev = cop.chis * copra.znam == cop.znam * copra.chis;
        return rev;
    }
    bool GetSign() const {
        return (chis.GetPlus() == znam.GetPlus());
    }
    bool operator!=(const Rational ra) {
        return !(*this == ra);
    }
    Rational& operator+=(const Rational ra) {
        if ((*this).GetSign() == ra.GetSign()) {
            BigInteger initznam = znam;
            znam = znam * ra.znam;
            chis = chis * ra.znam + initznam * ra.chis;
            Smaller();
            return *this;
        } else {
            Rational newnum = ra;
            if (!newnum.GetSign() && GetSign()) {
                newnum.chis.SetPlus(true);
                newnum.znam.SetPlus(true);
                return *this -= newnum;
            } else {
                newnum += *this;
                return *this = newnum;
            }
        }
    }
    Rational& operator-=(const Rational ra) {
        Rational newnum = ra;
        if (!newnum.GetSign() && GetSign()) {
            newnum.chis.SetPlus(true);
            newnum.znam.SetPlus(true);
            return *this += newnum;
        }
        if (newnum.GetSign() && !GetSign()) {
            newnum -= *this;
            newnum.chis.SetPlus(false);
            newnum.znam.SetPlus(true);
            return *this = newnum;
        }
        chis = chis * ra.znam - znam * ra.chis;
        znam = znam * ra.znam;
        Smaller();
        return *this;
    }
    Rational& operator*=(const Rational ra) {
        chis = chis * ra.chis;
        znam = znam * ra.znam;
        Smaller();
        return *this;
    }
    Rational& operator/=(const Rational ra) {
        chis *= ra.znam;
        znam *= ra.chis;
        Smaller();
        return *this;
    }
    Rational& operator++() {
        return *this += 1;
    }
    Rational operator++(int) {
        Rational cop;
        cop = *this;
        *this += 1;
        return cop;
    }
    Rational& operator--() {
        return *this -= 1;
    }
    Rational operator--(int) {
        Rational cop;
        cop = *this;
        *this -= 1;
        return cop;
    }
    explicit operator double() {
        return std::stod(asDecimal(20));
    }
};
Rational operator*(const Rational& ra, const Rational& rb);
Rational operator+(const Rational& ra, const Rational& rb);
Rational operator-(const Rational& ra, const Rational& rb);
Rational operator/(const Rational& ra, const Rational& rb);
BigInteger operator"" _bi(const char* str, size_t /*unused*/);
