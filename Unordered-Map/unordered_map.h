#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

template <typename T, typename Allocator = std::allocator<T>>
class List {
  public:
    using allocator_type = Allocator;
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using difference_type = ptrdiff_t;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;

    struct BaseNode {
        BaseNode* next = this;
        BaseNode* prev = this;
        BaseNode() = default;
        ~BaseNode() = default;
    };

    struct Node : BaseNode {
        T value;

        Node() = default;
        Node(const T& val)
            : value(val) {}
        Node(T&& val)
            : value(std::move(val)) {}
    };

    template <bool ConstOrNot>
    // former ListIterator
    class Iter {
      public:
        BaseNode* curnode = nullptr;

        using pointer = std::conditional_t<ConstOrNot, const T*, T*>;
        using reference = std::conditional_t<ConstOrNot, const T&, T&>;

        BaseNode* get_curnode() const {
            return curnode;
        }

        Iter() = default;
        ~Iter() = default;

        Iter(const Iter<false>& other) {
            curnode = other.curnode;
        }

        Iter(BaseNode* node)
            : curnode(node){};

        Iter& operator=(const Iter<false>& other) {
            if (&other == this) {
                return *this;
            }
            curnode = other.curnode;
            return *this;
        }
        friend bool operator==(const Iter& one, const Iter& other) {
            return one.curnode == other.curnode;
        }

        friend bool operator!=(const Iter& one, const Iter& other) {
            return one.curnode != other.curnode;
        }

        Iter& operator++() {
            curnode = curnode->next;
            return *this;
        }

        Iter& operator--() {
            curnode = curnode->prev;
            return *this;
        }

        reference operator*() const {
            return static_cast<Node*>(curnode)->value;
        }

        pointer operator->() const {
            return &(static_cast<Node*>(curnode)->value);
        }
    };
    template <bool ConstOrNot>
    friend bool operator==(const Iter<ConstOrNot>& one,
                           const Iter<ConstOrNot>& other) {
        return one.curnode == other.curnode;
    }

    template <bool ConstOrNot>
    friend bool operator!=(const Iter<ConstOrNot>& one,
                           const Iter<ConstOrNot>& other) {
        return one.curnode != other.curnode;
    }
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;

    iterator begin() const {
        if (this->size() == 0) {
            // why error?
            return iterator(fakeNode);
        }
        return iterator(fakeNode->next);
    }

    iterator end() const {
        return iterator(fakeNode);
    }

    const_iterator cbegin() const {
        if (this->size() == 0) {
            return const_iterator(fakeNode);
        }
        return const_iterator(fakeNode->next);
    }

    const_iterator cend() const {
        return const_iterator(fakeNode);
    }

    using NodeAlloc =
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    Allocator this_allocator;
    NodeAlloc nodeAlloc;
    size_t cursize = 0;

    BaseNode this_end = BaseNode();
    BaseNode* fakeNode = &this_end;

    size_type size() const {
        return cursize;
    }

    Allocator get_allocator() const {
        return this_allocator;
    }

    template <typename... Args>
    iterator emplace(List<T, Allocator>::const_iterator it, Args&&... args) {
        Node* newnode = nullptr;
        try {
            newnode = std::allocator_traits<NodeAlloc>::allocate(nodeAlloc, 1);
            std::allocator_traits<NodeAlloc>::construct(
                nodeAlloc, newnode, std::forward<Args>(args)...);
            BaseNode* oldonit = it.get_curnode();
            newnode->prev = it.get_curnode()->prev;
            newnode->next = it.get_curnode();
            oldonit->prev->next = newnode;
            oldonit->prev = newnode;
            ++cursize;
        } catch (...) {
            std::allocator_traits<NodeAlloc>::deallocate(nodeAlloc, newnode, 1);
            throw;
        }
        return iterator(newnode);
    }

    void erase(const_iterator it) {
        --cursize;
        it.curnode->prev->next = it.curnode->next;
        it.curnode->next->prev = it.curnode->prev;
        BaseNode* bn = it.get_curnode();
        std::allocator_traits<NodeAlloc>::destroy(nodeAlloc,
                                                  static_cast<Node*>(bn));
        std::allocator_traits<NodeAlloc>::deallocate(nodeAlloc,
                                                     static_cast<Node*>(bn), 1);
    }

    void insert(const_iterator it, const T& val) {
        emplace(it, val);
    }

    void insert(const_iterator it, T&& val) {
        emplace(it, std::move(val));
    }

    void pop_back() {
        erase(--end());
    }

    void pop_front() {
        erase(begin());
    }

    void clear() {
        while (cursize > 0) {
            pop_front();
        }
    }

    List() = default;

    ~List() {
        while (this->size() > 0) {
            pop_back();
        }
    }

    List(List<T, Allocator>&& other) {
        cursize = other.cursize;
        other.cursize = 0;
        nodeAlloc = std::move(other.nodeAlloc);
        this_end = std::move(other.this_end);
        fakeNode->next->prev = fakeNode;
        fakeNode->prev->next = fakeNode;
        other.fakeNode = nullptr;
    }

    List& operator=(List<T, Allocator>&& other) {
        cursize = other.cursize;
        other.cursize = 0;
        nodeAlloc = std::move(other.nodeAlloc);
        this_end = std::move(other.this_end);
        fakeNode->next->prev = fakeNode;
        fakeNode->prev->next = fakeNode;
        other.fakeNode = nullptr;
        return *this;
    }

    void swap(List& other) {
        if (this != &other) {
            auto next = fakeNode->next;
            auto prev = fakeNode->prev;
            auto other_next = other.fakeNode->next;
            auto other_prev = other.fakeNode->prev;
            std::swap(cursize, other.cursize);
            std::swap(next->prev, other_next->prev);
            std::swap(prev->next, other_prev->next);
            std::swap(fakeNode, other.fakeNode);
            std::swap(this_allocator, other.this_allocator);
            std::swap(nodeAlloc, other.nodeAlloc);
        }
    }
};

template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
  public:
    using NodeType = std::pair<const Key, Value>;

    double cur_lf = 1.0;
    Equal this_equal;
    Hash this_hash;
    Alloc this_allocator;
    size_t curlistsize = 0;

    struct ListNode {
        NodeType* value;
        size_t curhash;

        ListNode() = default;
        ListNode(NodeType* nt, size_t ch) {
            value = nt;
            curhash = ch;
        }
    };
    using ListNodeAlloc =
        typename std::allocator_traits<Alloc>::template rebind_alloc<ListNode>;
    List<ListNode, ListNodeAlloc> innerList;
    using ListIterator = typename List<ListNode, ListNodeAlloc>::iterator;
    std::vector<typename List<ListNode, ListNodeAlloc>::iterator> Buckets =
        std::vector<typename List<ListNode, ListNodeAlloc>::iterator>(
            1, innerList.end());

    template <bool ConstOrNot>
    class Iter {
      public:
        using ListIterator = typename List<ListNode, ListNodeAlloc>::iterator;
        ListIterator iterFromList;

        using value_type =
            std::conditional_t<ConstOrNot, const NodeType, NodeType>;
        using pointer =
            std::conditional_t<ConstOrNot, const NodeType*, NodeType*>;

        using reference =
            std::conditional_t<ConstOrNot, const NodeType&, NodeType&>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        Iter(const Iter& other) {
            iterFromList = other.iterFromList;
        }

        Iter() = default;
        ~Iter() = default;

        Iter(ListIterator otherlistit) {
            iterFromList = otherlistit;
        }

        Iter<ConstOrNot>& operator=(const Iter other) {
            iterFromList = other.iterFromList;
            return *this;
        }

        Iter<ConstOrNot> operator++(int) {
            Iter old(*this);
            ++iterFromList;
            return old;
        }

        Iter<ConstOrNot>& operator++() {
            ++iterFromList;
            return *this;
        }

        reference operator*() const {
            return *(*iterFromList).value;
        }

        pointer operator->() {
            return (*iterFromList).value;
        }

        template <bool Other>
        friend bool operator==(const Iter<Other>& one,
                               const Iter<ConstOrNot>& other) {
            return (one.iterFromList.curnode == other.iterFromList.curnode);
        }

        template <bool Other>
        friend bool operator!=(const Iter<Other>& one,
                               const Iter<ConstOrNot>& other) {
            return !(one == other);
        }
    };

    double load_factor() const {
        return static_cast<double>(size()) / Buckets.size();
    }
    using iterator = Iter<false>;
    using const_iterator = Iter<true>;
    using iteratorbool = std::pair<iterator, bool>;

    void swap(UnorderedMap& other) {
        Buckets.swap(other.Buckets);
        innerList.swap(other.innerList);
        std::swap(cur_lf, other.cur_lf);
    }

    void reserve(size_t n) {
        double filled = cur_lf * Buckets.size();
        if (n >= filled - 1) {
            size_t newsize = n / cur_lf;
            rehash(2 * newsize);
        }
    }

    UnorderedMap() = default;

    UnorderedMap(const UnorderedMap& other) {
        for (auto& x : other.innerList) {
            emplace(*(x.value));
        }
    }

    ~UnorderedMap() {
        while (innerList.cursize > 0) {
            // std::cout <<  it.curnode << '\n';
            erase(begin());
        }
    }

    UnorderedMap& operator=(const UnorderedMap& other) {
        UnorderedMap copy(other);
        Buckets.swap(copy.Buckets);
        innerList.swap(copy.innerList);
        return *this;
    }
    UnorderedMap& operator=(UnorderedMap&& other) {
        std::ignore = other;
        while (innerList.cursize > 0) {
            erase(begin());
        }
        Buckets.clear();
        return *this;
    }

    UnorderedMap(UnorderedMap&& other) {
        this_allocator = std::move(other.this_allocator);
        Buckets = std::move(other.Buckets);
        // std::cout <<  it.curnode << '\n';
        innerList = std::move(other.innerList);
    }

    size_t size() const {
        return innerList.size();
    }

    template <typename Inp>
    void insert(Inp it1, Inp it2) {
        auto it = it1;
        while (it != it2) {
            this->emplace(*it);
            ++it;
        }
    }
    iteratorbool insert(const NodeType& newnode) {
        return emplace(newnode);
    }
    iteratorbool insert(NodeType&& newnode) {
        return emplace(std::move(const_cast<Key&>(newnode.first)),  // NOLINT
                       std::move(newnode.second));
    }

    double max_load_factor() const {
        return cur_lf;
    }

    void max_load_factor(double new_lf) {
        cur_lf = new_lf;
    }

    iterator begin() {
        return iterator(innerList.begin());
    }
    const_iterator begin() const {
        return cbegin();
    }

    const_iterator cbegin() const {
        return const_iterator(innerList.begin());
    }

    const_iterator cend() const {
        return const_iterator(innerList.end());
    }
    iterator end() {
        return iterator(innerList.end());
    }

    const_iterator end() const {
        return cend();
    }

    size_t max_size() const {
        return Buckets.size() * cur_lf;
    }
    Value& at(const Key& key) {
        // std::cout <<  key << '\n';
        iterator theIt = find(key);
        if (theIt != end()) {
            return (*theIt).second;
        } else {
            throw std::out_of_range("no such key found in unordered map");
        }
    }

    iterator find(const Key& key) {
        size_t hash = this_hash(key);
        auto resit = end();
        auto sz = Buckets.size();
        ListIterator it = Buckets[hash % Buckets.size()];
        hash %= sz;
        // std::cout <<  it.curnode << '\n';
        // std::cout <<  (it.curnode == nullptr)  << '\n';
        while (it != innerList.end() && (*it).curhash % sz == hash) {
            if (this_equal((*(*it).value).first, key)) {
                resit = it;
            }
            ++it;
        }
        return resit;
    }

    Value& operator[](const Key& key) {
        iterator theIt = find(key);
        if (theIt != end()) {
            return (*theIt).second;
        } else {
            auto p = std::make_pair(key, Value());
            return (*this).emplace(p).first->second;
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        double filled = cur_lf * Buckets.size();
        if (filled <= innerList.size()) {
            size_t newsize = Buckets.size() * 2;
            rehash(newsize);
        }
        if (Buckets.empty()) {
            return {Buckets[0], true};
        }
        NodeType* newnode =
            std::allocator_traits<Alloc>::allocate(this_allocator, 1);
        std::allocator_traits<Alloc>::construct(this_allocator, newnode,
                                                std::forward<Args>(args)...);
        size_t hash = this_hash((*newnode).first);
        auto sz = Buckets.size();
        size_t index = hash % sz;
        iterator theIt = end();
        auto it = Buckets[hash % sz];
        while (it != innerList.end() && (*it).curhash % sz == index) {
            if (this_equal((*(*it).value).first, (*newnode).first)) {
                theIt = it;
            }
            ++it;
        }
        if (theIt == end()) {
            auto newln = ListNode(newnode, hash);
            Buckets[index] = innerList.emplace(Buckets[index], newln);
            ++curlistsize;
            return {Buckets[index], true};
        } else {
            std::allocator_traits<Alloc>::destroy(this_allocator, newnode);
            std::allocator_traits<Alloc>::deallocate(this_allocator, newnode,
                                                     1);
            return {theIt, false};
        }
    }

    template <typename Inp>
    void erase(Inp itFirst, Inp itLast) {
        Inp t;
        while (itFirst != itLast) {
            t = itFirst;
            ++t;
            erase(itFirst);
            itFirst = t;
        }
    }

    void rehash(size_t n) {
        ListIterator it;
        std::vector<ListIterator> newBuckets;
        newBuckets.resize(n);
        for (size_t i = 0; i < n; ++i) {
            newBuckets[i] = innerList.end();
        }
        auto bn = innerList.fakeNode;
        it = innerList.begin();
        typename List<ListNode, ListNodeAlloc>::BaseNode* curnode = nullptr;
        while (it != innerList.end()) {
            size_t curidx = (*it).curhash % n;
            curnode = it.curnode;
            ++it;
            if (newBuckets[curidx] != innerList.end()) {
                auto cn = newBuckets[curidx].curnode;
                auto cnp = cn->prev;
                curnode->prev = cnp;
                cnp->next = curnode;
                curnode->next = cn;
                newBuckets[curidx] = ListIterator(curnode);
                cnp = curnode;
            } else {
                curnode->prev = bn;
                bn->next = curnode;
                bn = curnode;
                curnode->next = innerList.fakeNode;
                auto k = innerList.fakeNode->prev;
                newBuckets[curidx] = ListIterator(curnode);
                bn->next->prev = k;
                bn->next = innerList.fakeNode;
                innerList.fakeNode->prev = curnode;
            }
        }
        Buckets.clear();
        Buckets = newBuckets;
    }

    void erase(iterator it) {
        ListIterator itt = it.iterFromList;
        if (this->size() == 0) {
            return;
        }
        auto curhash = (*itt).curhash;
        auto sz = Buckets.size();
        auto the_hash = curhash % sz;
        if (Buckets[the_hash] == itt) {
            ListIterator listIt(itt.curnode);
            ++listIt;
            if (listIt != innerList.end() &&
                (*listIt).curhash % sz == the_hash) {
                Buckets[the_hash] = listIt;
            } else {
                Buckets[the_hash] = innerList.end();
            }
        }
        auto ptr = (*itt).value;
        --curlistsize;
        std::allocator_traits<Alloc>::destroy(this_allocator, ptr);
        std::allocator_traits<Alloc>::deallocate(this_allocator, ptr, 1);
        innerList.erase(itt);
    }
};
