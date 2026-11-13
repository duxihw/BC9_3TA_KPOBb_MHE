#include "support/tracked.hpp"

namespace support {

namespace {
// Состояние счётчиков. Программа однопоточная, поэтому atomic не нужен.
LifetimeStats g_stats;
std::int64_t g_copies_left = -1;
}  // namespace

Tracked::Tracked(int value) : value_(value) { ++g_stats.value_ctor; }

Tracked::Tracked(const Tracked& other) : value_(other.value_) {
    on_copy();  // может бросить ДО учёта — объект тогда не считается созданным
    ++g_stats.copy_ctor;
}

Tracked::Tracked(Tracked&& other) noexcept : value_(other.value_) { ++g_stats.move_ctor; }

Tracked& Tracked::operator=(const Tracked& other) {
    on_copy();
    value_ = other.value_;
    ++g_stats.copy_assign;
    return *this;
}

Tracked& Tracked::operator=(Tracked&& other) noexcept {
    value_ = other.value_;
    ++g_stats.move_assign;
    return *this;
}

Tracked::~Tracked() { ++g_stats.dtor; }

const LifetimeStats& Tracked::stats() noexcept { return g_stats; }

void Tracked::reset_stats() noexcept {
    g_stats = LifetimeStats{};
    g_copies_left = -1;
}

void Tracked::throw_after_copies(std::int64_t n) noexcept { g_copies_left = n; }

void Tracked::on_copy() {
    if (g_copies_left == 0) throw CopyLimitExceeded();
    if (g_copies_left > 0) --g_copies_left;
}

}  // namespace support
