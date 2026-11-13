// =============================================================================
//  Тесты частей 1–2: grid::v1::Grid<T> — RAII, правило пяти, конструкторы,
//  оператор индексирования.
// =============================================================================

#include <string>
#include <type_traits>
#include <utility>

#include "grid/grid2d.hpp"
#include "support/tracked.hpp"
#include "test_framework.hpp"

using grid::v1::Grid;
using support::Tracked;

// Правило пяти: все пять операций доступны, перемещение не бросает.
static_assert(std::is_copy_constructible_v<Grid<int>>);
static_assert(std::is_copy_assignable_v<Grid<int>>);
static_assert(std::is_nothrow_move_constructible_v<Grid<int>>);
static_assert(std::is_nothrow_move_assignable_v<Grid<int>>);
// (1) — неявное преобразование T -> Grid<T>.
static_assert(std::is_convertible_v<float, Grid<float>>);
// Сетка работает с типом без конструктора по умолчанию (через конструктор 3).
static_assert(!std::is_default_constructible_v<Tracked>);
static_assert(std::is_constructible_v<Grid<Tracked>, unsigned, unsigned, Tracked>);
// g[y] — указатель на строку: константная сетка не даёт менять элементы.
static_assert(std::is_same_v<decltype(std::declval<Grid<int>&>()[0]), int*>);
static_assert(std::is_same_v<decltype(std::declval<const Grid<int>&>()[0]), const int*>);

namespace {
float first(Grid<float> const& g) { return g(0, 0); }
}  // namespace

TEST(v1_ctor1_implicit_conversion_makes_1x1) {
    Grid<float> g = 2.5f;  // неявное преобразование
    CHECK_EQ(g.get_y_size(), 1u);
    CHECK_EQ(g.get_x_size(), 1u);
    CHECK_EQ(g(0, 0), 2.5f);
    CHECK_EQ(first(7.0f), 7.0f);  // float -> временная Grid<float> при вызове функции
}

TEST(v1_ctor2_value_initialized) {
    Grid<int> g(4, 3);
    CHECK_EQ(g.get_y_size(), 4u);
    CHECK_EQ(g.get_x_size(), 3u);
    for (unsigned y = 0; y < 4; ++y)
        for (unsigned x = 0; x < 3; ++x) CHECK_EQ(g(y, x), 0);
    Grid<std::string> s(2, 2);
    CHECK(s(1, 1).empty());
}

TEST(v1_ctor3_filled_and_no_default_ctor_needed) {
    Tracked::reset_stats();
    {
        Grid<Tracked> g(3, 4, Tracked(7));
        CHECK_EQ(g(2, 3).value(), 7);
        CHECK_EQ(Tracked::stats().alive(), 12);
    }
    CHECK_EQ(Tracked::stats().alive(), 0);
}

TEST(v1_copy_is_deep_and_independent) {
    Grid<int> a(2, 3, 1);
    Grid<int> b(a);
    b(0, 0) = 42;
    CHECK_EQ(a(0, 0), 1);
    CHECK_EQ(b(0, 0), 42);

    Grid<int> c(5, 5, 9);
    c = a;
    CHECK(c == a);
    CHECK_EQ(c.get_y_size(), 2u);
    const Grid<int>& self = c;
    c = self;  // самоприсваивание
    CHECK(c == a);
}

TEST(v1_move_steals_without_copies) {
    Tracked::reset_stats();
    {
        Grid<Tracked> a(100, 100, Tracked(1));
        Grid<Tracked> c(1, 1, Tracked(0));
        const auto copies_before = Tracked::stats().copies();
        Grid<Tracked> b(std::move(a));
        c = std::move(b);
        CHECK_EQ(Tracked::stats().copies(), copies_before);  // ни одной копии
        CHECK_EQ(c.get_y_size(), 100u);
        CHECK_EQ(a.size(), 0u);  // NOLINT: moved-from пуст
        CHECK_EQ(b.size(), 0u);  // NOLINT
        a = c;                   // moved-from можно переиспользовать
        CHECK(a == c);
    }
    CHECK_EQ(Tracked::stats().alive(), 0);
}

TEST(v1_index_operator_reads_and_writes) {
    Grid<int> g(3, 4, 0);
    g[1][2] = 5;
    CHECK_EQ(g(1, 2), 5);
    g(2, 3) = 6;
    CHECK_EQ(g[2][3], 6);
    const Grid<int>& cg = g;
    CHECK_EQ(cg[1][2], 5);
}

TEST(v1_assign_value_fills_without_resizing) {
    Grid<int> g(2, 3, 0);
    g = 7;
    CHECK_EQ(g.get_y_size(), 2u);
    for (unsigned y = 0; y < 2; ++y)
        for (unsigned x = 0; x < 3; ++x) CHECK_EQ(g[y][x], 7);
}

TEST(v1_exception_during_copy_leaks_nothing) {
    Tracked::reset_stats();
    {
        Grid<Tracked> a(10, 10, Tracked(3));
        Grid<Tracked> b(2, 2, Tracked(8));
        Tracked::throw_after_copies(50);  // упадёт на 51-м копировании
        CHECK_THROWS(Grid<Tracked>(a));
        Tracked::throw_after_copies(50);
        CHECK_THROWS(b = a);
        Tracked::throw_after_copies(-1);
        CHECK_EQ(b(0, 0).value(), 8);  // строгая гарантия: b не изменился
        CHECK_EQ(b.get_y_size(), 2u);
        CHECK_EQ(Tracked::stats().alive(), 104);
    }
    CHECK_EQ(Tracked::stats().alive(), 0);
}

TEST(v1_empty_grids) {
    Grid<int> g(0, 5);
    CHECK_EQ(g.size(), 0u);
    Grid<int> h(g);
    CHECK_EQ(h.get_x_size(), 5u);
}
