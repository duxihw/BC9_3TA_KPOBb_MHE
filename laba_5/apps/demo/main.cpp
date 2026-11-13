// =============================================================================
//  Демонстрация: по одному примеру на каждый пункт лабораторной.
//  Запуск: build/release/bin/grid_demo
// =============================================================================

#include <iostream>
#include <string>
#include <utility>

#include "grid/grid.hpp"
#include "grid/grid2d.hpp"
#include "support/tracked.hpp"

namespace {

void section(const std::string& name) { std::cout << "\n=== " << name << " ===\n"; }

template <class G>
void print2d(const std::string& title, const G& g) {
    std::cout << title << " (" << g.get_y_size() << "x" << g.get_x_size() << "):\n";
    for (unsigned y = 0; y < g.get_y_size(); ++y) {
        std::cout << "  ";
        for (unsigned x = 0; x < g.get_x_size(); ++x) std::cout << g(y, x) << ' ';
        std::cout << '\n';
    }
}

void print_stats(const std::string& what) {
    const auto& s = support::Tracked::stats();
    std::cout << what << ": копий " << s.copies() << ", перемещений "
              << s.move_ctor + s.move_assign << ", живых объектов " << s.alive() << '\n';
}

}  // namespace

int main() {
    using support::Tracked;

    section("Часть 1. Конструкторы RAII-сетки grid::v1::Grid");
    grid::v1::Grid<float> one = 3.5f;  // (1) неявное T -> Grid<T>
    print2d("Grid<float> one = 3.5f", one);
    grid::v1::Grid<int> zeros(2, 3);   // (2) элементы по умолчанию
    print2d("Grid<int>(2, 3)", zeros);
    grid::v1::Grid<int> sevens(2, 4, 7);  // (3) копии значения
    print2d("Grid<int>(2, 4, 7)", sevens);

    section("Часть 1. Правило пяти на типе без конструктора по умолчанию");
    Tracked::reset_stats();
    {
        grid::v1::Grid<Tracked> a(100, 100, Tracked(1));
        print_stats("создали a(100x100)        ");
        grid::v1::Grid<Tracked> b = a;
        print_stats("скопировали b = a         ");
        grid::v1::Grid<Tracked> c = std::move(a);
        print_stats("переместили c = move(a)   ");
        std::cout << "  размер a после перемещения: " << a.get_y_size() << "x"  // NOLINT
                  << a.get_x_size() << '\n';
    }
    print_stats("все сетки разрушены       ");

    section("Часть 2. Оператор индексирования g[y][x]");
    grid::v1::Grid<float> g(3, 2, 0.0f);
    for (unsigned y = 0; y < g.get_y_size(); ++y)
        for (unsigned x = 0; x < g.get_x_size(); ++x) g[y][x] = static_cast<float>(10 * y + x);
    print2d("после g[y][x] = 10y + x", g);

    section("Часть 3. Многомерная сетка grid::Grid<T, N>");
    grid::Grid<float, 3> const g3(2, 3, 4, 1.0f);
    std::cout << "g3(1, 1, 1) = " << g3(1, 1, 1) << '\n';
    grid::Grid<float, 2> g2(2, 5, 2.0f);
    print2d("g2 до присваивания", g2);
    g2 = g3[1];  // g3 константна -> g3[1] — КОПИЯ Grid<float,2>, затем move-присваивание
    print2d("g2 = g3[1]", g2);

    grid::Grid<int, 3> cube(2, 2, 2, 0);
    cube[1][0][1] = 5;           // неконстантная сетка: цепочка ссылок до элемента
    cube[0] = grid::Grid<int>(2, 2, 9);  // подсетка заменяется целиком
    std::cout << "cube(1,0,1) = " << cube(1, 0, 1) << ", cube(0,1,1) = " << cube(0, 1, 1)
              << ", total_size = " << cube.total_size() << '\n';

    try {
        volatile unsigned bad = 2;  // индекс «из внешнего мира» (неизвестен компилятору)
        cube.at(bad, 0u, 0u);
    } catch (const std::out_of_range& e) {
        std::cout << "at(2,0,0): " << e.what() << '\n';
    }
    return 0;
}
