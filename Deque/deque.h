#include <cmath>
#include <exception>
#include <iostream>
#include <vector>

using std::cin;
using std::cout;
using std::vector;

template <typename T>
class Deque {
  private:
    void Rem(int i) const {
        for (int j = beg_idx; j < i; j++) {
            int idxfir = j / onesize, idxsec = j % onesize;
            elems[idxfir][idxsec].~T();
        }
    }

    void RemConst() const {
        for (size_t i = 0; i < elems.size(); i++) {
            delete[] reinterpret_cast<char*>(elems[i]);
        }
    }

    void clear() const {
        Rem(fin_idx);
        RemConst();
    }

    T& GetPointByIdx(int id) {
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        return elems[idxfir][idxsec];
    }

    T* GetPointByIdxPointer(size_t id) {
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        return &elems[idxfir][idxsec];
    }

    T* NewArr() {
        T* x = reinterpret_cast<T*>(new char[onesize * sizeof(T)]);
        return x;
    }

    static const size_t onesize = 64;

  public:
    template <bool IsConst>
    struct common_iterator {
        int pos = 0;
        typename std::conditional_t<IsConst,
                                    typename vector<T*>::const_iterator,
                                    typename vector<T*>::iterator>
            ptr;
        typename std::conditional_t<IsConst,
                                    typename vector<T*>::const_iterator,
                                    typename vector<T*>::iterator>
            end;
        typename std::conditional_t<IsConst,
                                    typename vector<T*>::const_iterator,
                                    typename vector<T*>::iterator>
            begin;
        typename std::conditional_t<IsConst, const T*, T*> element_it;
        int id = 0;

        using value_type = std::conditional_t<IsConst, const T, T>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using iterator_category = std::random_access_iterator_tag;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using difference_type = int;

        common_iterator(
            const int id, const int count,
            typename std::conditional_t<IsConst,
                                        typename vector<T*>::const_iterator,
                                        typename vector<T*>::iterator>
                this_pointer,
            typename std::conditional_t<IsConst,
                                        typename vector<T*>::const_iterator,
                                        typename vector<T*>::iterator>
                end,
            typename std::conditional_t<IsConst,
                                        typename vector<T*>::const_iterator,
                                        typename vector<T*>::iterator>
                begin)
            : pos(count), ptr(this_pointer), end(end), begin(begin), id(id) {
            element_it = id + (*ptr);
        }

        operator common_iterator<true>() {
            return common_iterator<true>(id, pos, ptr);
        }

        common_iterator& operator+=(const int addition) {
            int curadd = addition;
            if (static_cast<int>(onesize) - id <= curadd) {
                curadd -= onesize - id;
                ptr++;
                id = 0;
            }
            ptr += (curadd / onesize);
            curadd -= (curadd / onesize) * onesize;
            id += curadd;
            pos += addition;
            if (ptr < end) {
                element_it = (*ptr + id);
            }
            return *this;
        }
        common_iterator& operator-=(const int& addition) {
            int curadd = addition;
            if (id + 1 <= curadd) {
                curadd -= (id + 1);
                ptr--;
                id = onesize - 1;
            }
            ptr -= (curadd / onesize);
            curadd -= (curadd / onesize) * onesize;
            id -= curadd;
            pos -= addition;
            if (ptr < end && ptr >= begin) {
                element_it = (*ptr + id);
            }
            return *this;
        }
        common_iterator& operator++() {
            *this += 1;
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        common_iterator& operator--() {
            *this -= 1;
            return *this;
        }

        common_iterator operator--(int) {
            common_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        common_iterator operator+(const int& curadd) const {
            common_iterator tmp = *this;
            tmp += curadd;
            return tmp;
        };

        common_iterator operator-(const int& curadd) const {
            common_iterator tmp = *this;
            tmp -= curadd;
            return tmp;
        };

        int operator-(const common_iterator& other) const {
            return pos - other.pos;
        };

        bool operator==(const common_iterator& other) const {
            return pos == other.pos;
        };

        bool operator!=(const common_iterator& other) const {
            return pos != other.pos;
        };

        bool operator<(const common_iterator& other) const {
            return pos < other.pos;
        };

        bool operator>(const common_iterator& other) const {
            return pos > other.pos;
        };

        bool operator>=(const common_iterator& other) const {
            return pos >= other.pos;
        };

        bool operator<=(const common_iterator& other) const {
            return pos <= other.pos;
        };

        std::conditional_t<IsConst, const T&, T&> operator*() const {
            return *element_it;
        };
        typename std::conditional_t<IsConst, const T*, T*> operator->() const {
            return element_it;
        };
    };

    int beg_idx = 0;
    int fin_idx = 0;
    int sz = 0;
    int cap = 0;
    vector<T*> elems;

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        if (fin_idx == cap) {
            elems.push_back(NewArr());
            cap += onesize;
        }
        return iterator(beg_idx % onesize, beg_idx,
                        (elems.begin() + beg_idx / onesize), elems.end(),
                        elems.begin());
    }
    iterator end() {
        auto it = elems.begin() + fin_idx / onesize;
        if (fin_idx == cap) {
            elems.push_back(NewArr());
            cap += onesize;
        }
        return iterator(fin_idx % onesize, fin_idx, it, elems.end(),
                        elems.begin());
    }

    const_iterator begin() const {
        return const_iterator(beg_idx % onesize, beg_idx,
                              (elems.cbegin() + beg_idx / onesize), elems.end(),
                              elems.begin());
    }
    const_iterator end() const {
        auto it = elems.begin() + fin_idx / onesize;
        return const_iterator(fin_idx % onesize, fin_idx, it, elems.end(),
                              elems.begin());
    }

    const_iterator cbegin() const {
        return const_iterator(beg_idx % onesize, beg_idx,
                              (elems.cbegin() + beg_idx / onesize), elems.end(),
                              elems.begin());
    }
    const_iterator cend() const {
        auto it = elems.begin() + fin_idx / onesize;
        return const_iterator(fin_idx % onesize, fin_idx, it, elems.end(),
                              elems.begin());
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    Deque() {
        beg_idx = 0;
        fin_idx = 0;
        elems.push_back(NewArr());
        cap = onesize;
    }

    Deque(const Deque<T>& other)
        : beg_idx(other.beg_idx),
          fin_idx(other.fin_idx),
          sz(other.sz),
          cap(other.cap) {
        elems.resize(other.elems.size());
        for (size_t i = 0; i < elems.size(); i++) {
            elems[i] = NewArr();
        }
        int i = 0;
        try {
            for (i = beg_idx; i < fin_idx; i++) {
                int idxfir = i / onesize;
                int idxsec = i % onesize;
                new (elems[idxfir] + idxsec) T(other.elems[idxfir][idxsec]);
            }
        } catch (...) {
            Rem(i);
            RemConst();
            throw std::out_of_range("Constructor failed");
        }
    }

    Deque(const int k)
        : beg_idx(0), fin_idx(k), sz(k), cap((k / onesize + 1) * onesize) {
        elems.resize(k / onesize + 1);
        size_t i = 0;
        try {
            for (i = 0; i < elems.size(); i++) {
                elems[i] = NewArr();
                T x = T();
                for (size_t j = 0; j < onesize; j++) {
                    elems[i][j] = x;
                }
            }
        } catch (...) {
            Rem(i);
            RemConst();
            throw;
        }
    }

    Deque(const int k, const T& element)
        : beg_idx(0), fin_idx(k), sz(k), cap((k / onesize + 1) * onesize) {
        elems.resize(k / onesize + 1);
        for (size_t i = 0; i < elems.size(); i++) {
            elems[i] = NewArr();
        }
        int i = 0;
        try {
            for (i = 0; i < k; i++) {
                int idxfir = i / onesize, idxsec = i % onesize;
                new (elems[idxfir] + idxsec) T(element);
            }
        } catch (...) {
            Rem(i);
            RemConst();
            throw;
        }
    }

    Deque<T>& operator=(const Deque<T>& this_pointer) {
        if (&this_pointer == this) {
            return *this;
        }

        try {
            Deque<T> tmp(this_pointer);
            std::swap(cap, tmp.cap);
            std::swap(sz, tmp.sz);
            std::swap(elems, tmp.elems);
            std::swap(beg_idx, tmp.beg_idx);
            std::swap(fin_idx, tmp.fin_idx);
        } catch (...) {
            clear();
            throw;
        }
        return *this;
    }

    size_t size() const {
        return static_cast<size_t>(sz);
    }

    T& operator[](int id) {
        id += beg_idx;
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        return elems[idxfir][idxsec];
    }
    const T& operator[](int id) const {
        id += beg_idx;
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        return elems[idxfir][idxsec];
    }
    T& at(size_t id) {
        id += beg_idx;
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        if (id >= static_cast<size_t>(fin_idx)) {
            throw std::out_of_range("index out of range");
        }
        return elems[idxfir][idxsec];
    }

    const T& at(size_t id) const {
        id += beg_idx;
        int idxfir = id / onesize;
        int idxsec = id % onesize;
        if (id >= static_cast<size_t>(fin_idx)) {
            throw std::out_of_range("index out of range");
        }
        return elems[idxfir][idxsec];
    }

    void pop_back() {
        if (beg_idx != fin_idx) {
            fin_idx--;
            GetPointByIdx(fin_idx).~T();
            sz--;
        }
    }
    void pop_front() {
        if (beg_idx != fin_idx) {
            GetPointByIdx(beg_idx).~T();
            beg_idx++;
            sz--;
        }
    }
    void insert(iterator a, const T& x) {
        int where = a - begin();
        push_back(x);
        iterator it = begin() + where;
        iterator it1 = end() - 1;
        while (it1 != it) {
            std::swap(*it1, *(it1 - 1));
            --it1;
        }
    }

    void erase(iterator it) {
        while (it != end() - 1) {
            std::swap(*it, *(it + 1));
            ++it;
        }
        (*it).~T();
        fin_idx--;
        sz--;
    }

    void push_back(const T& x) {
        int where = 0;
        if (cap == fin_idx) {
            vector<T*> help((elems.size() + 1) * 3);
            size_t id_s = elems.size() + 1;
            for (size_t i = id_s; i < id_s + elems.size(); i++) {
                delete[] reinterpret_cast<char*>(help[i]);
                help[i] = elems[i - id_s];
            }
            for (size_t i = 0; i < help.size(); i++) {
                if (help[i] == nullptr) {
                    help[i] = NewArr();
                }
            }
            where = (id_s + elems.size()) * onesize;
            beg_idx += id_s * onesize;
            elems = help;
            cap = elems.size() * onesize;
            sz++;
            fin_idx = beg_idx + sz;
            new (elems[where / onesize] + where % onesize) T(x);
        } else {
            new (elems[fin_idx / onesize] + fin_idx % onesize) T(x);
            fin_idx++;
            sz++;
        }
    }

    void push_front(const T& el) {
        if (beg_idx > 0) {
            T* to_add = GetPointByIdxPointer(beg_idx - 1);
            new (to_add) T(el);
            --beg_idx;
            sz++;
            return;
        }
        increase_size();
        --beg_idx;
        sz++;
        new (elems[beg_idx / onesize] + beg_idx % onesize) T(el);
    }

    void increase_size() {
        std::vector<T*> new_array(cap * 3 / onesize);
        for (size_t i = 0; i < elems.size(); ++i) {
            new_array[i + cap / onesize] = elems.at(i);
        }
        for (size_t i = 0; i < new_array.size(); ++i) {
            if (new_array[i] == nullptr) {
                new_array[i] = NewArr();
            }
        }
        elems = new_array;
        fin_idx = fin_idx + cap;
        beg_idx = beg_idx + cap;
        cap *= 3;
    }

    ~Deque() {
        clear();
    }
};
