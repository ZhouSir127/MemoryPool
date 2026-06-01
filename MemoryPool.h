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
    MemoryPool(const MemoryPool& memoryPool) noexcept : MemoryPool(){}

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
      using other = MemoryPool<U,BlockSize>;
    };

    MemoryPool() noexcept : currentBlock_(nullptr), currentSlot_(nullptr), lastSlot_(nullptr), freeSlots_(nullptr) 
    {}
    
    template <class U> 
    MemoryPool(const MemoryPool<U>& memoryPool) noexcept : MemoryPool() 
    {}

    MemoryPool(MemoryPool&& memoryPool) noexcept
    {
        currentBlock_ = std::exchange(memoryPool.currentBlock_, nullptr);
        currentSlot_ = std::exchange(memoryPool.currentSlot_, nullptr);
        lastSlot_ = std::exchange(memoryPool.lastSlot_, nullptr);
        freeSlots_ = std::exchange(memoryPool.freeSlots_,nullptr);
    }
    
    ~MemoryPool() noexcept
    { 
      while (currentBlock_){
        Dummy* death = currentBlock_;
        currentBlock_ = currentBlock_->next;
        ::operator delete(reinterpret_cast<void*>(death) );
      }
    }

    T* allocate(size_t n = 1)
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

    void deallocate(T* p, size_t n = 1)
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

    bool operator==(const MemoryPool& b) const noexcept 
    {
        return this == &b;
    }

    bool operator!=(const MemoryPool& b) const noexcept 
    {
        return !this->operator==(b); 
    }

    template <typename U, size_t B2>
    bool operator==(const MemoryPool<U, B2>& b) const noexcept 
    {
        return false;
    }
    template <typename U, size_t B2>
    bool operator!=(const MemoryPool<U, B2>& b) const noexcept 
    {
        return true; 
    }

  private:
    union Slot_ {
      T element;
      Slot_* next;
    };

    using Dummy = struct Dummy{
      struct Dummy* next = nullptr;
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

      char*newBlock = reinterpret_cast<char*>(dummy),*body = newBlock + sizeof(Dummy);

      currentSlot_ = reinterpret_cast<Slot_*>(body + (alignof(Slot_)-reinterpret_cast<uintptr_t>(body) )%alignof(Slot_) );
      lastSlot_ = reinterpret_cast<Slot_*>(newBlock + BlockSize);
    }

    static_assert(BlockSize >= sizeof(Dummy) + alignof(Slot_) - 1 + sizeof(Slot_), "BlockSize too small.");
};


#endif