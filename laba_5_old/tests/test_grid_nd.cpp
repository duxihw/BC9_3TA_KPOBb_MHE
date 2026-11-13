// =============================================================================
//  Тесты части 3: grid::Grid<T, N> произвольной размерности.
// =============================================================================

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "grid/grid.hpp"
#include "support/tracked.hpp"
#include "test_framework.hpp"

using grid::Grid;
using support::Tracked;

// ---- статические проверки интерфейса
static_assert(std::is_same_v<Grid<int>, Grid<int, 2>>);  // N = 2 по умолчанию
static_assert(std::is_nothrow_move_constructible_v<Grid<int, 4>>);
static_assert(std::is_nothrow_move_assignable_v<Grid<int, 4>>);
static_assert(std::is_convertible_v<int, Grid<int, 3>>);   // (1) для N >= 2
static_assert(!std::is_convertible_v<int, Grid<int, 1>>);  // для N = 1 отключено

// operator[]: ссылка для неконстантной сетки, копия — для константной.
static_assert(std::is_same_v<decltype(std::declval<Grid<int, 3>&>()[0]), Grid<int, 2>&>);
static_assert(std::is_same_v<decltype(std::declval<const Grid<int, 3>&>()[0]), Grid<int, 2>>);
static_assert(std::is_same_v<decltype(std::declval<Grid<int, 1>&>()[0]), int&>);
static_assert(std::is_same_v<decltype(std::declval<const Grid<int, 1>&>()[0]), int>);

// operator() принимает ровно N индексов.
template <class G, class... I>
concept callable_with = requires(G g, I... i) { g(i...); };
static_assert(callable_with<Grid<int, 3>, int, int, int>);
static_assert(!callable_with<Grid<int, 3>, int, int>);
static_assert(!callable_with<Grid<int, 3>, int, int, int, int>);

TEST(nd_ctor1_implicit_conversion) {
    Grid<double, 3> g = 2.0;
    CHECK((g.extents() == Grid<double, 3>::dims_type{1, 1, 1}));
    CHECK_EQ(g(0, 0, 0), 2.0);
}

TEST(nd_ctor2_value_initialized) {
    Grid<int, 4> g(2, 3, 4, 5);
    CHECK_EQ(g.total_size(), 120u);
    CHECK_EQ(g.extent(3), 5u);
    CHECK_EQ(g(1, 2, 3, 4), 0);
    Grid<int, 1> line(7);  // N = 1: семь элементов
    CHECK_EQ(line.extent(0), 7u);
}

TEST(nd_ctor3_and_type_without_default_ctor) {
    Tracked::reset_stats();
    {
        Grid<Tracked, 3> g(2, 3, 4, Tracked(5));
        CHECK_EQ(g(1, 2, 3).value(), 5);
        CHECK_EQ(Tracked::stats().alive(), 24 + 0);
    }
    CHECK_EQ(Tracked::stats().alive(), 0);

    Grid<float, 2> f(2, 2, 1.5);  // double -> float в последнем аргументе
    CHECK_EQ(f(1, 1), 1.5f);
}

TEST(nd_index_returns_reference_for_nonconst) {
    Grid<int, 3> g(2, 3, 4, 0);
    g[1][2][3] = 7;
    CHECK_EQ(g(1, 2, 3), 7);

    // g[1] — ссылка: присваивание меняет саму сетку g.
    g[0] = Grid<int, 2>(3, 4, 9);
    CHECK_EQ(g(0, 2, 3), 9);
    g[1][0] = 5;  // заполнить строку значением
    CHECK_EQ(g(1, 0, 3), 5);
}

TEST(nd_index_returns_copy_for_const) {
    const Grid<int, 2> g(2, 2, 1);
    Grid<int, 1> row = g[1];  // копия
    row[0] = 100;
    CHECK_EQ(g(1, 0), 1);  // оригинал не изменился
    CHECK_EQ(g[1][1], 1);
}

TEST(nd_copy_and_move) {
    Tracked::reset_stats();
    {
        Grid<Tracked, 3> a(3, 4, 5, Tracked(1));
        Grid<Tracked, 3> b(a);
        CHECK(a == b);
        b(0, 0, 0) = Tracked(2);
        CHECK_EQ(a(0, 0, 0).value(), 1);

        const auto copies = Tracked::stats().copies();
        Grid<Tracked, 3> c(std::move(b));
        CHECK_EQ(Tracked::stats().copies(), copies);  // перемещение без копий
        CHECK_EQ(b.total_size(), 0u);                 // NOLINT
        CHECK_EQ(c(0, 0, 0).value(), 2);

        a = c;
        CHECK(a == c);
        a = std::move(c);
        CHECK_EQ(c.total_size(), 0u);  // NOLINT
        const Grid<Tracked, 3>& self = a;
        a = self;
        CHECK_EQ(a(0, 0, 0).value(), 2);
    }
    CHECK_EQ(Tracked::stats().alive(), 0);
}

TEST(nd_exception_safety_in_nested_construction) {
    Tracked::reset_stats();
    {
        const Tracked proto(4);
        // 2·3·4 = 24 копии нужно, упадём на 11-й — в середине второй подсетки.
        Tracked::throw_after_copies(10);
        CHECK_THROWS((Grid<Tracked, 3>(2, 3, 4, proto)));
        Tracked::throw_after_copies(-1);
        CHECK_EQ(Tracked::stats().alive(), 1);  // остался только proto

        Grid<Tracked, 3> ok(2, 3, 4, proto);
        Grid<Tracked, 3> target(1, 1, 1, proto);
        Tracked::throw_after_copies(15);
        CHECK_THROWS(target = ok);
        Tracked::throw_after_copies(-1);
        CHECK_EQ(target.total_size(), 1u);  // строгая гарантия
    }
    CHECK_EQ(Tracked::stats().alive(), 0);
}

TEST(nd_at_checks_bounds) {
    Grid<int, 3> g(2, 3, 4, 1);
    CHECK_EQ(g.at(1, 2, 3), 1);
    CHECK_THROWS(g.at(2, 0, 0));
    CHECK_THROWS(g.at(0, 3, 0));
    CHECK_THROWS(g.at(0, 0, 4));
}

TEST(nd_assign_value_and_iteration) {
    Grid<std::string, 2> g(2, 3, std::string("a"));
    g = std::string("b");
    int count = 0;
    for (auto& row : g)
        for (auto& s : row) {
            CHECK_EQ(s, std::string("b"));
            ++count;
        }
    CHECK_EQ(count, 6);
}

TEST(nd_high_dimension_and_empty) {
    Grid<char, 6> g(2, 2, 2, 2, 2, 2, 'x');
    CHECK_EQ(g.total_size(), 64u);
    CHECK_EQ(g(1, 1, 1, 1, 1, 1), 'x');

    Grid<int, 3> e(0, 5, 5);
    CHECK_EQ(e.total_size(), 0u);
    Grid<int, 3> e2(e);
    CHECK_EQ(e2.extent(2), 5u);
    Grid<int, 2> d;  // пустая по умолчанию
    CHECK_EQ(d.total_size(), 0u);
}
