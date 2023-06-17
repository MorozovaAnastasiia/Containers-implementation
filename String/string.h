#include <cstring>
#include <iostream>
class String {
  private:
    char* arr = nullptr;
    size_t strsize = 0;
    size_t cap = 0;
    void swap(String& str);
    void alloc_mem_for_one(int newcap);
    void alloc_mem_for_two(const String& str);
    const char* get_pointer() const;
    bool is_substring(const String& substring, int start) const;

  public:
    String(const char* s);
    String(char s);
    String(size_t n, char c);
    String();
    String(const String& str);
    String& operator=(const String& str);
    char& operator[](size_t index);
    const char& operator[](size_t index) const;
    char getind(int i) const;
    size_t length() const;
    void push_back(char c);
    void pop_back();
    char& front();
    char& back();
    const char& front() const;
    const char& back() const;
    String& operator+=(char c);
    String& operator+=(const String& str);
    size_t find(const String& str) const;
    size_t rfind(const String& str) const;
    String substr(size_t start, int count) const;
    bool empty() const;
    void clear();
    size_t size() const;
    size_t capacity() const;
    void shrink_to_fit();
    const char* data() const;
    char* data();
    ~String();
};
std::ostream& operator<<(std::ostream& outstr, const String& str);
std::istream& operator>>(std::istream& instr, String& str);
bool operator==(const String& st1, const String& st2);
bool operator!=(const String& st1, const String& st2);
bool operator<(const String& st1, const String& st2);
bool operator>(const String& st1, const String& st2);
bool operator<=(const String& st1, const String& st2);
bool operator>=(const String& st1, const String& st2);
String operator+(const String& st, char c);
String operator+(char c, const String& st1);
String operator+(const String& st1, const String& st2);
