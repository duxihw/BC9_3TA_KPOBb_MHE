#pragma once
// -----------------------------------------------------------------------------
// Работа с «сырой» памятью.
//
// Почему не new T[n]?
//   new T[n] сразу вызывает конструктор по умолчанию у каждого элемента, а
//   по условию сетка должна работать с типами БЕЗ конструктора по умолчанию.
//   Поэтому память выделяется отдельно (allocate), а объекты создаются в ней
//   отдельно (placement new / std::uninitialized_*). Разрушение — тоже в два
//   шага: std::destroy_n, затем deallocate.
// -----------------------------------------------------------------------------

#include <cstddef>
#include <memory>

namespace grid::detail {

// Выделяет память под n объектов U (без их создания). n == 0 -> nullptr.
template <class U>
U* allocate(std::size_t n) {
    if (n == 0) return nullptr;
    return std::allocator<U>{}.allocate(n);
}

// Освобождает память, полученную от allocate(n). Объекты уже должны быть разрушены.
template <class U>
void deallocate(U* p, std::size_t n) noexcept {
    if (p != nullptr) std::allocator<U>{}.deallocate(p, n);
}

// RAII-охранник на время конструирования: если в процессе создания элементов
// вылетит исключение, охранник разрушит уже созданные элементы и вернёт память.
// После успешного завершения вызывается release() — и он ничего не делает.
template <class U>
class ConstructionGuard {
public:
    ConstructionGuard(U* p, std::size_t capacity) noexcept : p_(p), capacity_(capacity) {}
    ConstructionGuard(const ConstructionGuard&) = delete;
    ConstructionGuard& operator=(const ConstructionGuard&) = delete;

    ~ConstructionGuard() {
        if (p_ == nullptr) return;
        std::destroy_n(p_, constructed_);
        deallocate(p_, capacity_);
    }

    void constructed_one() noexcept { ++constructed_; }
    void set_constructed(std::size_t n) noexcept { constructed_ = n; }
    void release() noexcept { p_ = nullptr; }

private:
    U* p_;
    std::size_t capacity_;
    std::size_t constructed_ = 0;
};

}  // namespace grid::detail
