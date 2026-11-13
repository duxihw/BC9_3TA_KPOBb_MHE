#include "bench/alloc_counter.hpp"

#include <cstdlib>
#include <new>

namespace bench {

namespace {
AllocStats g_stats;  // бенчмарк однопоточный
}

AllocStats alloc_stats() noexcept { return g_stats; }
void reset_alloc_stats() noexcept { g_stats = AllocStats{}; }

}  // namespace bench

// ---- замена глобальных функций выделения памяти

void* operator new(std::size_t size) {
    ++bench::g_stats.allocations;
    bench::g_stats.bytes += size;
    if (size == 0) size = 1;
    if (void* p = std::malloc(size)) return p;
    throw std::bad_alloc();
}

void* operator new[](std::size_t size) { return ::operator new(size); }

void operator delete(void* p) noexcept {
    if (p == nullptr) return;
    ++bench::g_stats.deallocations;
    std::free(p);
}

void operator delete[](void* p) noexcept { ::operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { ::operator delete(p); }
