// #include "MemoryPool.h"

// #include <iostream>
// #include <utility>
// #include <type_traits>
// #include <cstdlib> // for std::abort

//     void swap(MiniList& other) noexcept {
//         // [核心交互 4]：查阅内存池的 POCS 规则 (你的代码里是 true_type)
//         if constexpr (NodeAllocator::propagate_on_container_swap::value) {
//             // 完美！你的池子允许 swap，我们把池子和头指针一起交换
//             std::swap(alloc_, other.alloc_);
//             std::swap(head_, other.head_);
//         } else {
//             // 如果不准交换分配器，容器只能呼叫 operator== 进行生死判定
//             if (alloc_ == other.alloc_) {
//                 std::swap(head_, other.head_); // 一家人，换指针没问题
//             } else {
//                 // 【致命未定义行为】！两个不能互相兼容的池子被强行交换数据，必然引发析构时的释放崩溃！
//                 std::cerr << "Undefined Behavior: Cannot swap non-equal allocators without POCS!\n";
//                 std::abort();
//             }
//         }
//     }

//     // -------------------------------------------------------------
//     // 6. 跨容器拼接：operator== 的绝对防线
//     // -------------------------------------------------------------
//     // 把 other 的所有节点直接抢过来，挂到自己身上 (O(1) 或 O(N) 指针操作，不申请新内存)
//     void splice_front(MiniList& other) {
//         if (this == &other || other.head_ == nullptr) return;

//         // [核心交互 5]：抢别人节点之前，必须审问：我用我的池子，能释放你切出来的节点吗？！
//         if (alloc_ != other.alloc_) {
//             // 如果 operator== 返回 false，说明池子不同！绝对不能抢指针！
//             std::cerr << "Exception: allocators are not equal, cannot splice directly!\n";
//             return; 
//             // 真正的 STL 在这种情况下可能抛异常，或者无奈地退化成深拷贝
//         }

//         // 走到这里，说明 alloc_ == other.alloc_ 成立 (一家人)。可以安全执行物理接管！
//         Node* other_head = other.head_;
//         other.head_ = nullptr; // 剥夺对方头指针

//         // 找到 other 链表的尾巴，接在我的旧 head_ 上
//         Node* tail = other_head;
//         while (tail->next != nullptr) {
//             tail = tail->next;
//         }
//         tail->next = head_;
//         head_ = other_head;
//     }

    
// };