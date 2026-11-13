#include <iostream>
#include <utility>

#include "grid.hpp"

// Пример типа, у которого нет конструктора без параметров.
struct Number {
    int value;

    explicit Number(int initial) : value(initial) {}
};

int main() {
    Grid<float> single = 3.5f;
    std::cout << "Single element: " << single(0, 0) << '\n';

    Grid<float> g(3, 2, 0.0f);
    std::cout << "Grid size: " << g.get_y_size() << " x "
              << g.get_x_size() << '\n';

    // Два способа доступа обращаются к одному и тому же элементу.
    for (unsigned y = 0; y < g.get_y_size(); ++y) {
        for (unsigned x = 0; x < g.get_x_size(); ++x) {
            g[y][x] = 1.0f;
            std::cout << g(y, x) << ' ';
        }
        std::cout << '\n';
    }

    Grid<float> copy = g;
    copy[0][0] = 9.0f;
    std::cout << "Original: " << g(0, 0)
              << ", copy: " << copy(0, 0) << '\n';

    Grid<float> moved = std::move(copy);
    std::cout << "Moved element: " << moved(0, 0) << '\n';

    // Пример многомерной сетки из задания.
    const Grid<float, 3> g3(2, 3, 4, 1.0f);
    Grid<float, 2> g2(2, 5, 2.0f);
    g2 = g3[1];
    std::cout << "3D element: " << g3(1, 1, 1) << '\n';
    std::cout << "Copied slice: " << g2.get_y_size() << " x "
              << g2.get_x_size() << ", value: " << g2(1, 1) << '\n';

    // Объекты Number сразу создаются копированием Number(7).
    Grid<Number> numbers(2, 3, Number(7));
    std::cout << "Without default constructor: " << numbers(1, 2).value << '\n';

    // Присваивание значения заполняет всю сетку.
    g = 5.0f;
    std::cout << "After filling: " << g(2, 1) << '\n';

    return 0;
}
