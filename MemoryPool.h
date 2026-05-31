#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>
#include <cstdint>

template <typename T, size_t BlockSize = 4096>
class MemoryPool
{
  public:
    using value_type = T;
    
    using propagate_on_container_copy_assignment = std::false_type;
    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;

    using propagate_on_container_move_assignment = std::true_type;
    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept
    {
      if (this != &memoryPool)
      {
        std::swap(currentBlock_, memoryPool.currentBlock_);
        currentSlot_ = std::exchange(memoryPool.currentSlot_, nullptr);
        lastSlot_ = std::exchange(memoryPool.lastSlot_, nullptr);
        freeSlots_ = std::exchange(memoryPool.freeSlots_,nullptr);
      }
      return *this; 
    }
    
    using propagate_on_container_swap = std::true_type;

    template <typename U> struct rebind {
      using other = MemoryPool<U, BlockSize>;
    };

    MemoryPool() noexcept : currentBlock_(nullptr), currentSlot_(nullptr), lastSlot_(nullptr), freeSlots_(nullptr) 
    {}
    
    MemoryPool(const MemoryPool& memoryPool) noexcept : MemoryPool() 
    {}
    
    MemoryPool(MemoryPool&& memoryPool) noexcept
    {
        currentBlock_ = std::exchange(memoryPool.currentBlock_, nullptr);
        currentSlot_ = std::exchange(memoryPool.currentSlot_, nullptr);
        lastSlot_ = std::exchange(memoryPool.lastSlot_, nullptr);
        freeSlots_ = std::exchange(memoryPool.freeSlots_,nullptr);
    }
    
    // 异型拷贝构造：由于 T 和 U 大小不同，切片池无法通用，只能新建池子
    template <class U> 
    MemoryPool(const MemoryPool<U, BlockSize>& memoryPool) noexcept : MemoryPool() 
    {}

    ~MemoryPool() noexcept
    { 
      while (currentBlock_){
        Dummy* death = currentBlock_; // 此处类型已修正为 Dummy* 以匹配 currentBlock_
        currentBlock_ = currentBlock_->next;
        ::operator delete(reinterpret_cast<void*>(death) );
      }
    }

    /* 3. 核心分配接口：只做物理内存买卖 */
    inline T* allocate(size_t n = 1, const T* hint = 0)
    {
      if (n > 1)
          return static_cast<T*>(::operator new(n * sizeof(T)));

      T* result = nullptr;

      if (freeSlots_){
        result = &freeSlots_->element; 
        freeSlots_ = freeSlots_->next; 
      }else{
        if (currentSlot_ >= lastSlot_) // 建议后续优化：改为 currentSlot_ + 1 > lastSlot_
          allocateBlock();
        result = &currentSlot_->element; 
        currentSlot_++;
      }
      return result;
    }

    inline void deallocate(T* p, size_t n = 1)
    {
      if (p == nullptr) 
        return;

      if (n > 1) {
          ::operator delete(p);
          return;
      }

      reinterpret_cast<Slot_*>(p)->next = freeSlots_; 
      freeSlots_ = reinterpret_cast<Slot_*>(p); 
    }

    // 剔除了原版的 construct, destroy, address, max_size, newElement, deleteElement
    // 全权交由 std::allocator_traits 兜底！

  private:
    // 空间复用黑魔法 (Union)
    union Slot_ {
      T element;
      Slot_* next;
    };

    using Dummy = struct Dummy{
      struct Dummy* next;
    };

    Dummy* currentBlock_;
    Slot_* currentSlot_;
    Slot_* lastSlot_;
    Slot_* freeSlots_;

    void allocateBlock()
    {
      Dummy* dummy = reinterpret_cast<Dummy*>(::operator new(BlockSize) );
      dummy->next = currentBlock_;
      currentBlock_ = dummy;

      char* newBlock = reinterpret_cast<char*>(dummy);
      char* body = newBlock + sizeof(Dummy);

      currentSlot_ = reinterpret_cast<Slot_*>(body + (alignof(Slot_)-reinterpret_cast<uintptr_t>(body) )%alignof(Slot_) );
      lastSlot_ = reinterpret_cast<Slot_*>(newBlock + BlockSize);
    }

    static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize too small.");

    // 声明友元，用于 operator== 中的状态比对
    template <typename U, size_t B> friend class MemoryPool;
};

/* 4. 生死攸关的等价性判断 (必须在全局作用域) */
// 同类型 T 的分配器，只有底层物理指针完全一致，才算等价
template <typename T, size_t B>
inline bool operator==(const MemoryPool<T, B>& a, const MemoryPool<T, B>& b) noexcept {
    return &a == &b; // 极其严格的实例比对：不是同一个对象，就绝不能互相释放！
}

// 异型分配器 (T != U) 永远不等价，因为切片大小不同
template <typename T, size_t B1, typename U, size_t B2>
inline bool operator==(const MemoryPool<T, B1>& a, const MemoryPool<U, B2>& b) noexcept {
    return false; 
}

template <typename T, size_t B1, typename U, size_t B2>
inline bool operator!=(const MemoryPool<T, B1>& a, const MemoryPool<U, B2>& b) noexcept {
    return !(a == b);
}

#endif