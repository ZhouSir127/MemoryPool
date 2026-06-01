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


    
// };