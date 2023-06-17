#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

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

    struct BaseNode {
        BaseNode* prev = this;
        BaseNode* next = this;
        BaseNode() = default;
        ~BaseNode() = default;
    };

    struct Node : BaseNode {
        T value;
        Node() = default;
        Node(const T& val)
            : value(val) {}
    };

    template <typename ConstOrNot>
    class ListIterator {
      public:
        BaseNode* curnode = nullptr;

        using value_type = ConstOrNot;
        using difference_type = std::ptrdiff_t;
        using pointer = ConstOrNot*;
        using const_pointer = const ConstOrNot*;
        using reference = ConstOrNot&;
        using iterator_category = std::bidirectional_iterator_tag;

        BaseNode* get_curnode() const {
            return curnode;
        }

        operator ListIterator<const value_type>() const {
            return ListIterator<const value_type>(curnode);
        }
        ListIterator() = default;
        ~ListIterator() = default;
        ListIterator(BaseNode* node)
            : curnode(node) {}

        ListIterator(const ListIterator<ConstOrNot>& other) {
            curnode = other.get_curnode();
        }

        ListIterator& operator=(const ListIterator& other) {
            if (&other == this) {
                return *this;
            }
            curnode = other.curnode;
            return *this;
        }

        bool operator==(const ListIterator& other) const {
            return curnode == other.get_curnode();
        }

        bool operator!=(const ListIterator& other) const {
            return curnode != other.curnode;
        }

        ListIterator& operator++() {
            curnode = curnode->next;
            return *this;
        }

        ListIterator& operator--() {
            curnode = curnode->prev;
            return *this;
        }
        ListIterator operator++(int) {
            ListIterator old(*this);
            curnode = curnode->next;
            return old;
        }

        ListIterator operator--(int) {
            ListIterator old(*this);
            curnode = curnode->prev;
            return old;
        }

        reference operator*() {
            return static_cast<Node*>(curnode)->value;
        }

        pointer operator->() const {
            return &(static_cast<Node*>(curnode)->value);
        }
    };
    // end of iterator
    using iterator = ListIterator<T>;
    using const_iterator = ListIterator<const T>;
    using reverse_iterator = std::reverse_iterator<ListIterator<T>>;
    using const_reverse_iterator = std::reverse_iterator<ListIterator<const T>>;

    iterator begin() {
        return iterator(fakeNode->next);
    }
    iterator end() {
        return iterator(fakeNode);
    }
    const_iterator cbegin() const {
        return const_iterator(fakeNode->next);
    }
    const_iterator cend() const {
        return const_iterator(fakeNode);
    }
    const_iterator begin() const {
        return cbegin();
    }
    const_iterator end() const {
        return cend();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(fakeNode);
    }
    reverse_iterator rend() {
        return reverse_iterator(fakeNode->next);
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(fakeNode);
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator(fakeNode->next);
    }
    const_reverse_iterator rbegin() const {
        return crbegin();
    }
    const_reverse_iterator rend() const {
        return crend();
    }

    using NodeAlloc =
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    NodeAlloc this_allocator;

    BaseNode this_end = BaseNode();
    BaseNode* fakeNode = &this_end;
    size_t cursize = 0;

    size_t size() const {
        return cursize;
    }

    Allocator get_allocator() const {
        return this_allocator;
    }

    template <typename... Args>
    void emplace(List<T, Allocator>::const_iterator it, const Args&... args) {
        Node* newnode = nullptr;
        try {
            newnode =
                std::allocator_traits<NodeAlloc>::allocate(this_allocator, 1);
            std::allocator_traits<NodeAlloc>::construct(this_allocator, newnode,
                                                        args...);
            BaseNode* oldonit = it.get_curnode();
            newnode->prev = it.get_curnode()->prev;
            newnode->next = it.get_curnode();
            // it.curnode->prev->next = newnode;
            // it.curnode->prev = newnode;
            oldonit->prev->next = newnode;
            oldonit->prev = newnode;
            ++cursize;
        } catch (...) {
            // std::allocator_traits<NodeAlloc>::destroy(this_allocator,
            // newnode);
            std::allocator_traits<NodeAlloc>::deallocate(this_allocator,
                                                         newnode, 1);
            throw;
        }
    }

    void erase(const_iterator it) {
        --cursize;
        it.curnode->prev->next = it.curnode->next;
        it.curnode->next->prev = it.curnode->prev;
        std::allocator_traits<NodeAlloc>::destroy(
            this_allocator, static_cast<Node*>(it.curnode));
        std::allocator_traits<NodeAlloc>::deallocate(
            this_allocator, static_cast<Node*>(it.curnode), 1);
    }

    void insert(const_iterator it, const T& val) {
        try {
            emplace(it, val);
        } catch (...) {
            throw;
        }
    }

    void push_back(const T& value) {
        try {
            emplace(end(), value);
        } catch (...) {
            throw;
        }
    }

    void push_front(const T& value) {
        try {
            emplace(begin(), value);
        } catch (...) {
            throw;
        }
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

    List(size_t n, const T& val) {
        size_t i;
        for (i = 0; i < n; ++i) {
            try {
                emplace(end(), val);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    pop_back();
                }
                throw;
            }
        }
    }

    List(size_t n, allocator_type a)
        : this_allocator(a) {
        size_t i;
        for (i = 0; i < n; ++i) {
            try {
                emplace(end());
            } catch (...) {
                while (cursize != 0) {
                    erase(--end());
                }
                throw;
            }
        }
    }

    List(size_t n)
        : List<T, Allocator>(n, this_allocator) {}

    List(const allocator_type& a)
        : this_allocator(a) {}

    List(size_t n, const T& val, allocator_type a)
        : this_allocator(a) {
        size_t i;
        for (i = 0; i < n; ++i) {
            try {
                emplace(end(), val);
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    pop_back();
                }
                throw;
            }
        }
    }

    List(const List& l) {
        clear();
        this_allocator = std::allocator_traits<
            NodeAlloc>::select_on_container_copy_construction(l.this_allocator);
        size_t n = l.size();
        size_t i;
        const_iterator it(l.begin());
        for (i = 0; i < n; ++i) {
            try {
                this->emplace(end(),
                              static_cast<Node*>(it.get_curnode())->value);
                ++it;
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    pop_back();
                }
                throw;
            }
        }
    }

    ~List() {
        while (this->size() > 0) {
            pop_back();
        }
    }

    // here could arise an issue
    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        if (std::allocator_traits<
                NodeAlloc>::propagate_on_container_copy_assignment::value) {
            this_allocator = other.this_allocator;
        }
        try {
            for (const auto& el : other) {
                push_back(el);
            }
        } catch (...) {
            while (true) {
                if (fakeNode->prev == fakeNode) {
                    break;
                }
                pop_back();
            }
        }
        return *this;
    }
};
