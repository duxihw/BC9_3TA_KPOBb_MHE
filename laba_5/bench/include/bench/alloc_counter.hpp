#pragma once
// -----------------------------------------------------------------------------
// Счётчик выделений динамической памяти.
//
// В alloc_counter.cpp глобальные operator new / operator delete заменены
// своими версиями, которые считают вызовы и байты, а затем вызывают malloc /
// free. Это стандартный и законный способ: замена глобальных функций
// выделения памяти явно разрешена стандартом C++ ([replacement.functions]).
//
// Используется только в бенчмарке, чтобы измерить, сколько выделений делает
// каждая реализация сетки (плоская — одно, рекурсивная — по одному на
// каждую подсетку).
// -----------------------------------------------------------------------------

#include <cstddef>
#include <cstdint>

namespace bench {

struct AllocStats {
    std::uint64_t allocations = 0;
    std::uint64_t deallocations = 0;
    std::uint64_t bytes = 0;  // суммарно запрошено байт
};

AllocStats alloc_stats() noexcept;
void reset_alloc_stats() noexcept;

}  // namespace bench
