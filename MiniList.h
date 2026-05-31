#include "MemoryPool.h"

#include <iostream>
#include <utility>

template <typename T, typename Alloc = MemoryPool<T>>
class MiniList {
private:
    struct Node {
        T data;
        Node* next;

        template <typename... Args>
        Node(Node* n, Args&&... args) : next(n), data(std::forward<Args>(args)...) {}
    };

    //typename { [Alloc::(template rebind<Node>)] :: other } 

    using NodeAllocator = typename Alloc::template rebind<Node>::other;//MemoryPool<Node>;

    Node* head_;            
    NodeAllocator alloc_;

public:
    explicit MiniList(const Alloc& user_alloc ) // MemoryPool<T> user_alloc
    : head_(nullptr), alloc_(user_alloc) // template<Node> MemoryPool(const MemoryPool<T>&)
    {}

    MiniList() : head_(nullptr)
    {}

    ~MiniList() {
        while (head_) {
            Node* next = head_->next;

            head_->~Node();
            alloc_.deallocate(head_, 1);

            head_ = next;
        }
        head_ = nullptr;
    }

    template <typename... Args>
    void emplace_front(Args&&... args) {
        Node* raw_memory = alloc_.allocate(1);
        ::new (static_cast<void*>(raw_memory)) Node(head_, std::forward<Args>(args)...);

        head_ = raw_memory;
    }

    void pop_front() {
        if (head_ == nullptr) 
            return;

        Node* death_node = head_;      
        head_ = head_->next;           

        death_node->~Node();     
        alloc_.deallocate(death_node, 1); 
    }
    
};