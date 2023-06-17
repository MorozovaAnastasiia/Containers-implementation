#pragma once
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

template <size_t N>
class alignas(std::max_align_t) StackStorage {
  private:
    char arr[N];
    size_t shift = 0;

  public:
    StackStorage() {}
    ~StackStorage() {}
    StackStorage(StackStorage&) = delete;
    StackStorage& operator=(StackStorage s) = delete;
    // c
    // loc
    char* locate(size_t a, size_t count_bytes) {
        shift = count_bytes + ((shift + a - 1) / a) * a;
        return arr + shift - count_bytes;
    }
    // move
    // not const ?
    StackStorage(StackStorage&&) = delete;
    StackStorage& operator=(StackStorage&&) = delete;
};

template <typename T, size_t N>
class StackAllocator {
  public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using void_pointer = void*;
    using const_void_pointer = const void_pointer;
    using difference_type = ptrdiff_t;
    using size_type = size_t;

    using propagate_on_container_copy_assignment = std::true_type;

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackAllocator() {}
    ~StackAllocator() {}
    StackAllocator(StackStorage<N>& storage) {
        _storage = &storage;
    }

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& otheralloc) {
        _storage = otheralloc.get_storage();
    }

    template <typename U>
    StackAllocator& operator=(StackAllocator<U, N> other) {
        if (other.get_storage() != _storage) {
            std::swap(other._storage, _storage);
        }
        return *this;
    }
    StackStorage<N>* get_storage() const {
        return _storage;
    }

    template <typename U, size_t OtherSize>
    bool operator==(const StackAllocator<U, OtherSize>& other) noexcept {
        return _storage == other.get_storage();
    }

    // ?
    template <typename U, size_t OtherSize>
    bool operator!=(const StackAllocator<U, OtherSize>& other) noexcept {
        return !(this == other);
    }

    pointer allocate(size_type count, const void* pp = nullptr) {
        std::ignore = pp;
        T* ptr;
        try {
            ptr = reinterpret_cast<T*>(
                _storage->locate(alignof(T), count * sizeof(T)));
        } catch (...) {
            throw;
        }
        return ptr;
    }

    template <typename... Args>
    void construct(pointer ptr, const Args&... args) {
        new (ptr) T(args...);
    }
    void destroy(pointer ptr) {
        ptr->~T();
    }
    void deallocate(pointer pk, size_type ss) {
        std::ignore = pk;
        std::ignore = ss;
    }
    // no move implementation
    StackStorage<N>* _storage;
};
