#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>

struct BaseCB {
    BaseCB() = default;
    virtual void mainDel() = 0;
    int shc = 0;
    int weakc = 0;
    virtual void BDel() = 0;
    virtual void* getPtr() = 0;
    virtual ~BaseCB() = default;
};

template <typename U>
class SharedPtr {
    template <typename Y, typename... Args>
    friend SharedPtr<Y> makeShared(Args&&... args);
    template <typename Y, typename Alloc, typename... Args>
    friend SharedPtr<Y> allocateShared(Alloc alloc, Args&&... args);
    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;

    template <typename Y, typename Deleter, typename Alloc>
    struct UsualCB : BaseCB {
      private:
        using BlockAlloc = typename std::allocator_traits<
            Alloc>::template rebind_alloc<UsualCB<Y, Deleter, Alloc>>;
        using BlockAllocTraits = typename std::allocator_traits<BlockAlloc>;
        Y* ptr;
        Alloc alloc;
        Deleter deleter;

      public:
        UsualCB(Y* ptr, Deleter del, Alloc alloc)
            : ptr(ptr), alloc(alloc), deleter(del) {}

        void mainDel() override {
            deleter(ptr);
            ptr = nullptr;
        }

        void* getPtr() override {
            return ptr;
        }

        void BDel() override {
            auto ths = this;
            BlockAlloc newal = std::move(alloc);
            BlockAllocTraits::deallocate(newal, ths, 1);
        }
        ~UsualCB() override {}
    };

    template <typename Alloc>
    struct MakeSharedCB : BaseCB {
      private:
        Alloc alloc;
        U obj;
        using CusAlloc =
            typename std::allocator_traits<Alloc>::template rebind_alloc<U>;
        using CusTraits = typename std::allocator_traits<CusAlloc>;

      public:
        using BlockAlloc = typename std::allocator_traits<
            Alloc>::template rebind_alloc<MakeSharedCB<Alloc>>;
        using BlockAllocTraits = typename std::allocator_traits<BlockAlloc>;

        template <typename... Args>
        MakeSharedCB(Alloc alloc, Args&&... args)
            : alloc(alloc), obj(std::forward<Args>(args)...) {}

        void BDel() override {
            auto ths = this;
            BlockAlloc newal = std::move(alloc);
            BlockAllocTraits::deallocate(newal, ths, 1);
        }

        void* getPtr() override {
            return &obj;
        }

        void mainDel() override {
            CusAlloc AnotherAlloc(alloc);
            CusTraits::destroy(AnotherAlloc, &obj);
        }
        ~MakeSharedCB() override {}
    };

    BaseCB* cblock = nullptr;

    SharedPtr(BaseCB* cblock) : cblock(cblock) {
        if (cblock != nullptr) {
            this->cblock->shc++;
        }
    }

  public:
    SharedPtr() = default;

    template <typename Y, typename Deleter = std::default_delete<Y>,
              typename Alloc = std::allocator<Y>>
    SharedPtr(Y* ptr, Deleter dlt = Deleter(), Alloc alloc = Alloc()) {
        using BlockAlloc = typename std::allocator_traits<
            Alloc>::template rebind_alloc<UsualCB<Y, Deleter, Alloc>>;
        using BlockAllocTraits = typename std::allocator_traits<BlockAlloc>;
        BlockAlloc nwAlloc = alloc;
        auto newptr = BlockAllocTraits::allocate(nwAlloc, 1);
        ::new (newptr) UsualCB<Y, Deleter, Alloc>(ptr, dlt, alloc);
        this->cblock = newptr;
        this->cblock->shc++;
    }

    SharedPtr(const SharedPtr& other) : cblock(other.cblock) {
        if (cblock != nullptr) {
            cblock->shc++;
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : cblock(other.cblock) {
        if (cblock != nullptr) {
            cblock->shc++;
        }
    }

    SharedPtr(SharedPtr&& other) : cblock(other.cblock) {
        other.cblock = nullptr;
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) : cblock(other.cblock) {
        other.cblock = nullptr;
    }

    void swap(SharedPtr<U>& other) {
        std::swap(cblock, other.cblock);
    }

    ~SharedPtr() {
        if (cblock != nullptr) {
            cblock->shc--;
            if (cblock->shc == 0) {
                cblock->mainDel();
                if (cblock->weakc == 0) {
                    cblock->BDel();
                    cblock = nullptr;
                }
            }
        }
    }

    SharedPtr<U>& operator=(const SharedPtr<U>& other) {
        if (this == &other) {
            return *this;
        }
        SharedPtr<U> copy(other);
        swap(copy);
        return *this;
    }

    template <typename Y>
    SharedPtr<U>& operator=(const SharedPtr<Y>& other) {
        SharedPtr<U> copy(other);
        swap(copy);
        return *this;
    }

    SharedPtr<U>& operator=(SharedPtr<U>&& other) {
        SharedPtr<U> copy(std::move(other));
        swap(copy);
        return *this;
    }

    template <typename Y>
    SharedPtr<U>& operator=(SharedPtr<Y>&& other) {
        SharedPtr<U> copy(std::move(other));
        swap(copy);
        return *this;
    }

    U* get() const {
        if (cblock == nullptr) {
            return nullptr;
        }
        return static_cast<U*>(cblock->getPtr());
    }

    U& operator*() const {
        return *(get());
    }
    U* operator->() const {
        return get();
    }

    size_t use_count() const {
        if (cblock == nullptr) {
            return 0;
        }
        return cblock->shc;
    }
    template <typename Y>
    void reset(Y* ptr) {
        SharedPtr<U> copy(ptr);
        swap(copy);
    }

    void reset() {
        SharedPtr<U> to;
        swap(to);
    }
};

template <typename Y, typename Alloc, typename... Args>
SharedPtr<Y> allocateShared(Alloc alloc, Args&&... args) {
    using ControlBlock = typename SharedPtr<Y>::template MakeSharedCB<Alloc>;
    using BlockAlloc =
        typename SharedPtr<Y>::template MakeSharedCB<Alloc>::BlockAlloc;
    BlockAlloc ballocator = alloc;

    ControlBlock* blockPtr =
        ControlBlock::BlockAllocTraits::allocate(ballocator, 1);
    ControlBlock::BlockAllocTraits::construct(ballocator, blockPtr, alloc,
                                              std::forward<Args>(args)...);

    return SharedPtr<Y>(dynamic_cast<BaseCB*>(blockPtr));
}

template <typename Y, typename... Args>
SharedPtr<Y> makeShared(Args&&... args) {
    return allocateShared<Y, std::allocator<Y>, Args...>(
        std::allocator<Y>(), std::forward<Args>(args)...);
}
template <typename U>
class WeakPtr {
  private:
    BaseCB* ptr = nullptr;
    template <typename Y>
    friend class WeakPtr;

  public:
    void swap(WeakPtr& other) {
        std::swap(ptr, other.ptr);
    }

    WeakPtr() = default;

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& ptr) : ptr(ptr.cblock) {
        this->ptr->weakc++;
    }

    WeakPtr(const WeakPtr& ptr) : ptr(ptr.ptr) {
        this->ptr->weakc++;
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& ptr) : ptr(ptr.ptr) {
        this->ptr->weakc++;
    }

    WeakPtr(WeakPtr&& ptr) : ptr(ptr.ptr) {
        ptr.ptr = nullptr;
    }

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& ptr) : ptr(ptr.ptr) {
        ptr.ptr = nullptr;
    }

    ~WeakPtr() {
        if (ptr != nullptr) {
            ptr->weakc--;
            if (ptr->weakc == 0 && ptr->shc == 0) {
                ptr->BDel();
            }
        }
    }

    WeakPtr& operator=(const WeakPtr& ptr) {
        WeakPtr<U>(ptr).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& ptr) {
        WeakPtr<U>(ptr).swap(*this);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& ptr) {
        WeakPtr(std::move(ptr)).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& ptr) {
        WeakPtr<U>(std::move(ptr)).swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& ptr) {
        WeakPtr<U>(ptr).swap(*this);
        return *this;
    }

    bool expired() const {
        if (ptr == nullptr) {
            return true;
        }
        return ptr->shc == 0;
    }

    SharedPtr<U> lock() const {
        if (ptr == nullptr) {
            return SharedPtr<U>();
        }
        return SharedPtr<U>(ptr);
    }

    size_t use_count() const {
        if (ptr == nullptr) {
            return 0;
        }
        return ptr->shc;
    }
};
