#include <iostream>
#include <memory>
#include <vector>

template <typename T> class SharedPtr;
template <typename T> class WeakPtr;

// for debug
bool flag = false;
template<typename T>
void D(const T& mess) { if (flag) std::cout << "DEB " << mess << std::endl; }

struct BaseControlBlock {
  int shared_count = 0;
  int weak_count = 0;
  BaseControlBlock(int sc, int wc) : shared_count(sc), weak_count(wc) {}
  
  virtual void useDeleter(void *ptr) = 0;
  virtual void Deallocate() = 0;
};

template <typename T, typename Allocator, typename Deleter = std::false_type>
struct AllocControlBlock : public BaseControlBlock {
  [[no_unique_address]] Allocator allocator; 
  [[no_unique_address]] Deleter deleter;

  using AT_base = std::allocator_traits<Allocator>;

  AllocControlBlock(int int1, int int2, const Allocator &a, const Deleter &d)
      : BaseControlBlock(int1, int2), allocator(a), deleter(d) {}

  virtual void useDeleter(void *ptr_tmp) final {
    T* ptr = static_cast<T*>(ptr_tmp);
    if constexpr (std::is_same_v<Deleter, std::false_type>) {
      AT_base::destroy(allocator, ptr);
      
    } else {
      deleter(ptr);
    }
  }

  virtual void Deallocate() {
    using Alloc_this =
        typename AT_base::template rebind_alloc<AllocControlBlock>;
    using AT = std::allocator_traits<Alloc_this>;
    Alloc_this alloc_tmp(std::move(allocator));
    AT::deallocate(alloc_tmp, this, 1);
  }
};

template <typename T, typename Allocator, typename Deleter>
struct ControlBlockRegular: public AllocControlBlock<T, Allocator, Deleter> {
  bool boolarr[sizeof(T)];
  using Parent = AllocControlBlock<T, Allocator, Deleter>;
  using Parent::allocator;
  using Parent::deleter;
  using AT_base = std::allocator_traits<Allocator>;

  virtual void Deallocate() {
    using Alloc_this =
        typename AT_base::template rebind_alloc<ControlBlockRegular>;
    using AT = std::allocator_traits<Alloc_this>;
    Alloc_this alloc_tmp(std::move(allocator));
    AT::deallocate(alloc_tmp, this, 1);
  }

  template <typename... Args>
  ControlBlockRegular(int int1, int int2, Allocator allocator_1, Deleter deleter_1, Args&&... args) : Parent(int1, int2, allocator_1, deleter_1) {
    auto p = reinterpret_cast<T*>(boolarr);
    AT_base::construct(allocator, p, std::forward<Args>(args)...);
  }

  T &getObject() const { return reinterpret_cast<T*>(boolarr); }
};

template <typename T> class SharedPtr {

  template <typename U> friend class WeakPtr;
  template <typename U> friend class SharedPtr;

  T *ptr = nullptr;
  BaseControlBlock *cb = nullptr;

  SharedPtr(const WeakPtr<T> &wptr) : ptr(wptr.ptr), cb(wptr.cb) {
    ++(cb->shared_count);
  }

public:
  SharedPtr() {}

  SharedPtr(const SharedPtr &other) : ptr(other.ptr), cb(other.cb) {
    ++(cb->shared_count);
  }

  
  template <typename U>
  SharedPtr(const SharedPtr<U> &other) : ptr(other.ptr), cb(other.cb) {
    ++(cb->shared_count);
  }

  template <typename U>
  SharedPtr(SharedPtr<U> &&other) : ptr(other.ptr), cb(other.cb) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  SharedPtr(T *ptr, BaseControlBlock *cb) : ptr(ptr), cb(cb) {}

  SharedPtr(T *ptr) : ptr(ptr) {
    auto local_alloc = std::allocator<T>();
    auto local_deleter = std::false_type();
    cb =
        new AllocControlBlock<T, std::allocator<T>, std::false_type>(1, 0, local_alloc, local_deleter);
  }

  template <typename Deleter, typename Allocator = std::allocator<T>>
  SharedPtr(T *ptr, Deleter deleter = {}, Allocator allocator = std::allocator<T>()): ptr(ptr) {
    using ACB = AllocControlBlock<T,  Allocator,  Deleter>;
    using AT = typename std::allocator_traits<Allocator>::template rebind_alloc<ACB>;
    auto actual_alloc = AT();
    auto cb_ptr = actual_alloc.allocate(1);
    new (cb_ptr) ACB(1, 0, allocator, deleter); 
    cb = cb_ptr;
  }

  SharedPtr &operator=(const SharedPtr &other) {
    ptr = other.ptr;
    cb = other.cb;
    ++(cb->shared_count);
    return *this;
  }
  SharedPtr &operator=(SharedPtr &&other) {
    SharedPtr sh = std::move(other);
    std::swap(ptr, sh.ptr);
    std::swap(cb, sh.cb);
    return *this;
  }

  void decrcount() {
    if (!cb) return;
    --(cb->shared_count);
    if (cb->shared_count == 0) {
      cb->useDeleter(ptr);
      if (cb->weak_count == 0) {
        cb->Deallocate();
      }
    }
    ptr = nullptr;
    cb = nullptr;
  }

  ~SharedPtr() { if (cb) {decrcount();} }

  int use_count() const { return cb->shared_count; }

  void reset(T *p = nullptr) {
    if (cb) decrcount();
    if (p != nullptr) {
      *this = SharedPtr(p);
    }
  }

  T &operator*() const { return *ptr; }
  T *operator->() const { return ptr; }
  T* get() const {return ptr;}

  void swap(SharedPtr &other) {
    std::swap(ptr, other.ptr);
    std::swap(cb, other.cb);
  }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args &&...args) {
   auto local_alloc = std::allocator<T>();
    auto local_deleter = std::false_type();
  auto cb1 = new ControlBlockRegular<T, std::allocator<T>, std::false_type>(
      1, 0, local_alloc, local_deleter, std::forward<Args>(args)...);
  auto cb2 = static_cast<BaseControlBlock*>(cb1);
  return SharedPtr<T>(reinterpret_cast<T*>(&cb1->boolarr), cb2);
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc &alloc, Args &&...args) {
  
  using CBR = ControlBlockRegular<T, Alloc, std::false_type>;
  using CBAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<CBR>;
  using AT = typename std::allocator_traits<CBAlloc>;
  auto tmp_alloc = CBAlloc(alloc);
  auto cb1 = AT::allocate(tmp_alloc, 1);
  new (cb1) CBR(1, 0, std::move(tmp_alloc), std::false_type(), std::forward<Args>(args)...);
  //AT::construct(alloc, &cb1->object, std::forward<Args>(args)...);
  auto cb2 = static_cast<BaseControlBlock*>(cb1);
  return SharedPtr<T>(reinterpret_cast<T*>(&cb1->boolarr), (cb2));
  
}

template <typename T> class WeakPtr {
  template <typename U> friend class SharedPtr;
  template <typename U> friend class WeakPtr;

  T *ptr;
  BaseControlBlock *cb;

public:
  WeakPtr() {}

  WeakPtr(const WeakPtr& other) : ptr(other.ptr), cb(other.cb) {
     ++(cb->weak_count);
  }

  template <typename U>
  WeakPtr(const SharedPtr<U> &other) : ptr(other.ptr), cb(other.cb) {
    ++(cb->weak_count);
  }
  template <typename U>
  WeakPtr(const WeakPtr<U> &other) : ptr(other.ptr), cb(other.cb) {
    ++(cb->weak_count);
  }
  template <typename U>
  WeakPtr(WeakPtr<U> &&other) : ptr(other.ptr), cb(other.cb) {
    other.ptr = nullptr;
    other.cb = nullptr;
  }

  WeakPtr &operator=(const WeakPtr &other) {
    ptr = other.ptr;
    cb = other.cb;
    ++(cb->weak_count);
    return *this;
  }
  WeakPtr &operator=(WeakPtr<T> &&other) {
    ptr = other.ptr;
    cb = other.cb;
    other.ptr = nullptr;
    other.cb = nullptr;
    return *this;
  }
  bool expired() const noexcept {
    if (cb->shared_count == 0) {
      return true;
    }
    return false;
  }
  int use_count() const { return cb->shared_count; }
  SharedPtr<T> lock() const noexcept { return SharedPtr<T>(*this); }
  ~WeakPtr() {
    --(cb->weak_count);
    if (cb->weak_count == 0 && cb->shared_count == 0) {
      cb->Deallocate(); 
    }
  }
};
