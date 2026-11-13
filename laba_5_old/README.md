# Лабораторная работа «Управление ресурсами». Шаблон Grid

* **Части 1–2** (RAII, правило пяти, конструкторы, `g[y][x]`) —
  [`include/grid/grid2d.hpp`](include/grid/grid2d.hpp) + [`grid2d_impl.hpp`](include/grid/grid2d_impl.hpp),
  класс `grid::v1::Grid<T>`.
* **Часть 3** (сетка произвольной размерности) —
  [`include/grid/grid.hpp`](include/grid/grid.hpp) + [`grid_impl.hpp`](include/grid/grid_impl.hpp),
  класс `grid::Grid<T, N = 2>` (при N = 2 он же удовлетворяет частям 1–2).
* **Исследование** (копирование vs перемещение, способы доступа, выделения памяти):
  бенчмарк C++ → CSV в [`../data/laba_5/`](../data/laba_5) → анализ на Python ([`python/`](python)) → графики в [`../plots/laba_5/`](../plots/laba_5).
* **Отчёт с пояснениями и выводами** — [`docs/REPORT.md`](docs/REPORT.md).

## Структура проекта

```
laba_5/
├── CMakeLists.txt              корневой CMake
├── CMakePresets.json           пресеты Ninja: release / debug / asan
├── include/grid/
│   ├── grid2d.hpp              части 1–2: ОБЪЯВЛЕНИЕ grid::v1::Grid<T>
│   ├── grid2d_impl.hpp         части 1–2: определения методов
│   ├── grid.hpp                часть 3: ОБЪЯВЛЕНИЕ grid::Grid<T, N>
│   ├── grid_impl.hpp           часть 3: определения методов
│   └── detail/
│       ├── memory.hpp          сырая память + охранник конструирования
│       └── traits.hpp          концепты для ограничений конструкторов
├── support/                    библиотека grid_support
│   ├── include/support/tracked.hpp   тип-счётчик Tracked (без конструктора по умолчанию)
│   └── src/tracked.cpp
├── apps/demo/main.cpp          демонстрация всех частей
├── tests/
│   ├── test_framework.hpp/.cpp мини-фреймворк тестов (без внешних зависимостей)
│   ├── test_lab_examples.cpp   примеры кода из текста лабораторной
│   ├── test_grid2d.cpp         тесты частей 1–2
│   └── test_grid_nd.cpp        тесты части 3
├── bench/
│   ├── include/bench/          timer.hpp, csv_writer.hpp, alloc_counter.hpp, experiments.hpp
│   └── src/                    timer.cpp, csv_writer.cpp, alloc_counter.cpp, experiments.cpp, main.cpp
├── python/
│   ├── analyze.py              точка входа анализа
│   ├── requirements.txt
│   └── gridlab_analysis/       data.py, plots.py, report.py
├── scripts/run_all.py          сборка → тесты → бенчмарк → анализ
└── docs/REPORT.md              отчёт
```

> **Почему шаблоны разбиты на `*.hpp` + `*_impl.hpp`, а не `.hpp` + `.cpp`?**
> Код шаблона генерируется при инстанцировании с конкретным `T`, поэтому
> определения должны быть видны в каждой единице трансляции. Интерфейс и
> реализация разнесены по разным файлам, реализация подключается в конце
> заголовка. Поэтому `grid` в CMake — INTERFACE-библиотека. Весь нешаблонный
> код (тип-счётчик, таймер, CSV, счётчик выделений памяти, тесты) — обычные
> пары `.hpp` + `.cpp`.

CSV и сводные таблицы хранятся в `../data/laba_5/`, графики — в
`../plots/laba_5/`. Все команды ниже выполняются из папки `laba_5`.
Пути по умолчанию в CMake и Python привязаны к расположению проекта.

## Сборка (CMake + Ninja)

Нужны CMake ≥ 3.20, Ninja и компилятор C++20 (GCC ≥ 11, Clang ≥ 14, MSVC 19.30+).
На Windows — например, MSYS2 UCRT64 с `mingw-w64-ucrt-x86_64-{gcc,cmake,ninja}`.

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

Без пресетов:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

С санитайзерами (Linux/macOS): `cmake --preset asan && cmake --build --preset asan && ctest --preset asan`.

| Программа (`build/release/bin/`) | Что делает |
|---|---|
| `grid_demo`  | показывает все части лабораторной |
| `grid_tests` | 22 теста, включая примеры из условия |
| `grid_bench` | замеры → CSV (`--out DIR`, `--quick`, `--only copy_move\|access\|construct`), ≈ 20 с |

## Исследование

```bash
pip install -r python/requirements.txt
./build/release/bin/grid_bench      # CSV -> ../data/laba_5/
python python/analyze.py            # графики -> ../plots/laba_5/, таблицы -> ../data/laba_5/
```

Всё сразу: `python scripts/run_all.py`. Цели Ninja: `ninja -C build/release run_bench analyze`.

| Файл | Содержимое |
|---|---|
| `../data/laba_5/copy_move.csv` | `impl, n, elements, operation, ns, allocations, bytes, element_copies` |
| `../data/laba_5/access.csv` | `dims, n, elements, method, order, ns_per_element` |
| `../data/laba_5/construct.csv` | `impl, dims, d0, d1, d2, elements, allocations, bytes, bytes_per_element, construct_ns, destroy_ns` |
| `../data/laba_5/fit_copy_move.csv` | показатель α в t ~ (элементов)^α |
| `../data/laba_5/summary_access.csv` | доступ: отношение к `std::vector` |
| `../data/laba_5/check_construct.csv` | число выделений памяти и формула для него |

## Короткий пример

```cpp
#include "grid/grid.hpp"

grid::Grid<float> g(3, 2, 0.0f);   // 3 × 2, заполнена нулями
g[1][0] = 5.0f;                    // g[y][x]
float v = g(1, 0);                 // g(y, x)

grid::Grid<float, 3> const g3(2, 3, 4, 1.0f);
grid::Grid<float, 2> g2 = g3[1];   // константная сетка: g3[1] — копия подсетки
auto moved = std::move(g2);        // O(1), без копирования элементов
```
