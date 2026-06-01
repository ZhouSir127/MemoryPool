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
    using value_type = typename Alloc::value_type;

    void clear(){
        while (head_) {
            Node* next = head_->next;

            head_->~Node();
            alloc_.deallocate(head_, 1);

            head_ = next;
        }
        head_ = nullptr;
    }

    void deepCopy(const MiniList& other) {
        for (Node* p = other.head_,*end = head_; p ; p = p->next) {
            Node* raw = alloc_.allocate(1);
            ::new (static_cast<void*>(raw)) Node (nullptr, p->data);
            end ? end->next = raw : head_ = raw;
            end = raw;
        }
    }

    explicit MiniList(const Alloc& user_alloc) // MemoryPool<T> user_alloc
    : head_(nullptr), alloc_(user_alloc) // template<Node> MemoryPool(const MemoryPool<T>&)
    {}

    MiniList() : head_(nullptr){}

    MiniList(const MiniList& other) : head_(nullptr), alloc_(other.alloc_){    
        deepCopy(other);        
    }

    
    MiniList& operator=(const MiniList& other) {
        if (this == &other) 
            return *this;

        if constexpr (NodeAllocator::propagate_on_container_copy_assignment::value){
            if(alloc_ != other.alloc_)
                clear(); 
                    
            alloc_ = other.alloc_;
        //想实现alloc_作为别名引用other.alloc_，但C++不允许，所以只能在这里重新赋值
        //但是浅拷贝会导致两个对象的alloc_指向同一块内存，内存践踏且析构时会重复释放同一块内存，造成错误，所以有状态的分配器必须禁止拷贝赋值
        }

        Node* curr = head_;
        Node* p = other.head_;
        Node* end = head_; 
        //可以复用的前提是旧内存未被deallocate
        while (p && curr){
            curr->data = p->data; 
            end = curr;
            curr = curr->next;
            p = p->next;
        }
        
        if (p) {
            do {
                Node* raw = alloc_.allocate(1);
                ::new (static_cast<void*>(raw)) Node(nullptr, p->data);

                if(end)
                    end = end->next = raw; 
                else
                    end = head_ = raw; 
                
                p = p->next;
            }while(p);
        }else if (curr) {
            if(end != curr)
                end->next = nullptr; 
            else
                head_ = nullptr; 

            do{
                Node* next = curr->next;
                curr->~Node();
                alloc_.deallocate(curr, 1);
                curr = next;
            }while(curr);
        }
        return *this;
    }

    MiniList& operator=(MiniList&& other) noexcept {
        if (this == &other) 
            return *this;

        if constexpr (NodeAllocator::propagate_on_container_move_assignment::value) {
            clear();
            alloc_ = std::move(other.alloc_);
            head_ = std::exchange(other.head_, nullptr);
        } else {
            if (alloc_ == other.alloc_) {
                clear();
                head_ = std::exchange(other.head_, nullptr);
            }else{
                Node* curr = head_;
                Node* p = other.head_;
                Node* end = head_; 
                //可以复用的前提是旧内存未被deallocate
                while (p && curr){
                    curr->data = std::move(p->data); 
                    end = curr;
                    curr = curr->next;
                    p = p->next;
                }
                
                if (p) {
                    do {
                        Node* raw = alloc_.allocate(1);
                        ::new (static_cast<void*>(raw)) Node(nullptr, std::move(p->data));

                        if(end)
                            end = end->next = raw; 
                        else
                            end = head_ = raw; 
                        
                        p = p->next;
                    }while(p);
                }else if (curr) {
                    if(end != curr)
                        end->next = nullptr; 
                    else
                        head_ = nullptr; 

                    do{
                        Node* next = curr->next;
                        curr->~Node();
                        alloc_.deallocate(curr, 1);
                        curr = next;
                    }while(curr);
                }
            }
        }
        return *this;
    }

    ~MiniList() {
        clear();
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