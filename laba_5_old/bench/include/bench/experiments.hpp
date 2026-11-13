#pragma once
// Эксперименты. Каждый пишет свой CSV в config.out_dir.

#include <filesystem>

namespace bench {

struct Config {
    std::filesystem::path out_dir;
    bool quick = false;  // уменьшенные размеры для быстрой проверки
};

// copy_move.csv — стоимость копирования и перемещения сетки в зависимости
// от числа элементов: время, число выделений памяти, число копий элементов.
void run_copy_move(const Config& config);

// access.csv — время доступа к элементу разными способами:
// g(y, x) и g[y][x] у плоской и рекурсивной сеток, std::vector как эталон;
// обход по строкам и по столбцам; 2D и 3D.
void run_access(const Config& config);

// construct.csv — создание/разрушение сеток одного объёма, но разной формы:
// сколько выделений памяти и времени это стоит.
void run_construct(const Config& config);

}  // namespace bench
