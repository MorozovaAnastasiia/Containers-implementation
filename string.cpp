#include "string.h"
void String::alloc_mem_for_one(int newcap) {
    char* newarr = new char[newcap];
    int ss = strsize;
    memcpy(newarr, arr, ss);
    delete[] arr;
    arr = newarr;
    cap = newcap - 1;
}
void String::swap(String& str) {
    std::swap(arr, str.arr);
    std::swap(strsize, str.strsize);
    std::swap(cap, str.cap);
}
const char* String::get_pointer() const {
    return arr;
}
bool String::is_substring(const String& substring, int start) const {
    int finish = std::min(start + substring.strsize, strsize);
    for (int i = start; i < finish; ++i) {
        if (arr[i] != substring[i - start]) {
            return false;
        }
    }
    return true;
}
String::String(const char* s)
    : arr(new char[std::strlen(s) + 1]),
      strsize(std::strlen(s)),
      cap(strsize + 1) {
    std::copy(s, s + strsize + 1, arr);
}
String::String(char s) : arr(new char[1]), strsize(1), cap(strsize + 1) {
    arr[0] = s;
}
String::String(size_t n, char c)
    : arr(new char[n + 1]), strsize(n), cap(n + 1) {
    memset(arr, c, n);
    arr[n] = '\0';
}
String::String() : arr(new char[1]), strsize(0), cap(1) {
    arr[0] = '\0';
}
String::String(const String& str)
    : arr(new char[str.cap + 1]), strsize(str.strsize), cap(str.cap) {
    memcpy(arr, str.arr, strsize + 1);
}
String& String::operator=(const String& str) {
    String copy = str;
    swap(copy);
    return *this;
}
char& String::operator[](size_t index) {
    return arr[index];
}
const char& String::operator[](size_t index) const {
    return arr[index];
}
char String::getind(int i) const {
    return arr[i];
}
size_t String::length() const {
    return strsize;
}
void String::push_back(char c) {
    if (strsize == cap - 1) {
        alloc_mem_for_one(cap * 2);
    }
    arr[strsize] = c;
    arr[strsize + 1] = '\0';
    ++strsize;
}
void String::pop_back() {
    arr[strsize - 1] = '\0';
    --strsize;
}
char& String::front() {
    return arr[0];
}
char& String::back() {
    return arr[strsize - 1];
}
const char& String::front() const {
    return arr[0];
}
const char& String::back() const {
    return arr[strsize - 1];
}
String& String::operator+=(char c) {
    push_back(c);
    return *this;
}
String& String::operator+=(const String& str) {
    if (cap > strsize + str.strsize) {
        int size1 = strsize;
        int size2 = str.strsize;
        std::memcpy(arr + size1, str.arr, size2);
        std::memcpy(arr + size1, str.arr, size2);
        strsize = strsize + str.strsize;
        arr[strsize] = '\0';
    } else {
        alloc_mem_for_one(cap + str.cap);
        std::memcpy(arr + strsize, str.arr, str.strsize);
        strsize = strsize + str.strsize;
        arr[strsize] = '\0';
        cap = std::max(cap, str.cap) * 2;
    }
    return *this;
}
size_t String::find(const String& str) const {
    if (strsize < str.strsize) {
        return size();
    }
    int secsize = str.strsize;
    for (size_t i = 0; i <= strsize - secsize; ++i) {
        if (is_substring(str, i)) {
            return i;
        }
    }
    return size();
}
size_t String::rfind(const String& str) const {
    if (strsize < str.strsize) {
        return size();
    }
    int secsize = str.strsize;
    for (int i = strsize - secsize; i >= 0; --i) {
        if (is_substring(str, i)) {
            return i;
        }
    }
    return size();
}
String String::substr(size_t start, int count) const {
    String res(count, 'a');
    int st = start;
    std::memcpy(res.arr, arr + st, count);
    return res;
}
bool String::empty() const {
    return (strsize == 0);
}
void String::clear() {
    arr[0] = '\0';
    strsize = 0;
}
size_t String::size() const {
    return strsize;
}
size_t String::capacity() const {
    return cap;
}
void String::shrink_to_fit() {
    if (strsize == cap - 1) {
        return;
    } else {
        alloc_mem_for_one(strsize + 1);
        arr[strsize] = '\0';
    }
}
const char* String::data() const {
    return arr;
}
char* String::data() {
    return arr;
}
String::~String() {
    delete[] arr;
}
std::ostream& operator<<(std::ostream& outstr, const String& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        outstr << str[i];
    }
    return outstr;
}
std::istream& operator>>(std::istream& instr, String& str) {
    char c;
    str.clear();
    while (instr.get(c)) {
        if (c == '\n' || c == ' ') {
            break;
        }
        str.push_back(c);
    }
    return instr;
}
bool operator==(const String& st1, const String& st2) {
    if (st1.length() != st2.length()) {
        return false;
    }
    for (size_t i = 0; i < st1.length(); ++i) {
        if (st1.getind(i) != st2.getind(i)) {
            return false;
        }
    }
    return true;
}
bool operator!=(const String& st1, const String& st2) {
    return !(st1 == st2);
}
bool operator<(const String& st1, const String& st2) {
    size_t minn = std::min(st1.length(), st2.length());
    for (size_t i = 0; i < minn; ++i) {
        if (st1.getind(i) < st2.getind(i)) {
            return true;
        }
        if (st2.getind(i) < st1.getind(i)) {
            return false;
        }
    }
    return (st1.length() < st2.length());
}
bool operator>(const String& st1, const String& st2) {
    return (st2 < st1);
}
bool operator<=(const String& st1, const String& st2) {
    return !(st1 > st2);
}
bool operator>=(const String& st1, const String& st2) {
    return !(st1 < st2);
}
String operator+(const String& st, char c) {
    String res(st);
    res += c;
    return res;
}
String operator+(char c, const String& st1) {
    String res(c);
    res += st1;
    return res;
}
String operator+(const String& st1, const String& st2) {
    String res(st1);
    res += st2;
    return res;
}
