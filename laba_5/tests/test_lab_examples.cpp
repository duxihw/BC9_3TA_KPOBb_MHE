// =============================================================================
//  Примеры кода из текста лабораторной — дословно (только assert заменены на
//  CHECK, чтобы ошибка не обрывала остальные тесты). Каждый пример
//  запускается и для grid::v1::Grid (части 1–2), и для grid::Grid (часть 3).
// =============================================================================

#include "grid/grid.hpp"
#include "grid/grid2d.hpp"
#include "test_framework.hpp"

namespace {

// Пример из раздела «Оператор индексирования».
template <template <typename> class G>
void index_operator_example() {
    G<float> g(3, 2, 0.0f);
    CHECK(3 == g.get_y_size());
    CHECK(2 == g.get_x_size());

    using gsize_t = typename G<int>::size_type;

    for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
        for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
            CHECK(0.0f == g[y_idx][x_idx]);

    for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
        for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
            g[y_idx][x_idx] = 1.0f;

    for (gsize_t y_idx = 0; y_idx != g.get_y_size(); ++y_idx)
        for (gsize_t x_idx = 0; x_idx != g.get_x_size(); ++x_idx)
            CHECK(1.0f == g(y_idx, x_idx));
}

// grid::Grid<T, N> с N = 2 по умолчанию как шаблон с одним параметром.
template <typename T>
using NdGrid2 = grid::Grid<T>;

}  // namespace

TEST(lab_example_index_operator_v1) { index_operator_example<grid::v1::Grid>(); }

TEST(lab_example_index_operator_nd) { index_operator_example<NdGrid2>(); }

// Пример из раздела «Многомерные сетки».
TEST(lab_example_multidimensional) {
    using grid::Grid;

    Grid<float, 3> const g3(2, 3, 4, 1.0f);
    CHECK(1.0f == g3(1, 1, 1));

    Grid<float, 2> g2(2, 5, 2.0f);
    CHECK(2.0f == g2(1, 1));

    g2 = g3[1];
    CHECK(1.0f == g2(1, 1));

    // После присваивания g2 принял размеры подсетки g3[1]: 3 × 4.
    CHECK_EQ(g2.get_y_size(), 3u);
    CHECK_EQ(g2.get_x_size(), 4u);
}
