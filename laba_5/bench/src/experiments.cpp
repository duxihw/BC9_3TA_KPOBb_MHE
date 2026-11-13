// =============================================================================
//  Эксперименты исследовательской части.
//
//  Сравниваются две реализации, которые получились в лабораторной:
//    flat   — grid::v1::Grid<float>    (части 1–2): один непрерывный блок памяти;
//    nested — grid::Grid<float, N>     (часть 3):  рекурсивная структура,
//                                       у каждой подсетки свой блок памяти.
//  Эталон для доступа к элементам — std::vector<float> с ручным y*n + x.
//
//  Все времена — медиана по нескольким повторам (устойчива к выбросам).
// =============================================================================

#include "bench/experiments.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "bench/alloc_counter.hpp"
#include "bench/csv_writer.hpp"
#include "bench/timer.hpp"
#include "grid/grid.hpp"
#include "grid/grid2d.hpp"
#include "support/tracked.hpp"

namespace bench {

namespace {

using FlatGrid = grid::v1::Grid<float>;
using NestedGrid = grid::Grid<float, 2>;
using NestedGrid3 = grid::Grid<float, 3>;

// В эксперименте с доступом элементы целые: сумма целых чисел может
// векторизоваться (SIMD), а сумма float — нет (без -ffast-math порядок
// сложений float менять нельзя), и тогда все способы доступа упираются в
// задержку сложения и неотличимы. С int видно, какой доступ компилятор
// способен оптимизировать, а какой — нет.
using Elem = std::int32_t;
using FlatGridI = grid::v1::Grid<Elem>;
using NestedGridI = grid::Grid<Elem, 2>;
using NestedGrid3I = grid::Grid<Elem, 3>;

volatile double g_sink = 0.0;
volatile std::int64_t g_isink = 0;  // чтобы компилятор не выбросил вычисления

std::vector<unsigned> powers_of_two(unsigned from, unsigned to) {
    std::vector<unsigned> v;
    for (unsigned n = from; n <= to; n *= 2) v.push_back(n);
    return v;
}

// Медиана времени (нс) выполнения body(), повторённого reps раз.
template <class Body>
double median_ns(int reps, Body body) {
    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(reps));
    for (int i = 0; i < reps; ++i) samples.push_back(body());
    return median(std::move(samples));
}

// Число выделений памяти во время выполнения f().
template <class F>
AllocStats count_allocations(F f) {
    reset_alloc_stats();
    f();
    return alloc_stats();
}

// --------------------------------------------------------------- copy / move

struct OpResult {
    double ns;
    std::uint64_t allocations;
    std::uint64_t bytes;
    std::int64_t element_copies;
};

// G — тип сетки, make(n, value) создаёт сетку n×n.
template <class G, class Make>
std::vector<std::pair<std::string, OpResult>> measure_copy_move(unsigned n, Make make, int reps) {
    std::vector<std::pair<std::string, OpResult>> out;
    G a = make(n, 1.0f);

    // --- копирующий конструктор: время только самого копирования,
    //     разрушение копии вынесено за пределы замера.
    {
        std::optional<G> c;
        const double ns = median_ns(reps, [&] {
            c.reset();
            Stopwatch sw;
            c.emplace(a);
            escape(&*c);
            return sw.elapsed_ns();
        });
        c.reset();
        const auto al = count_allocations([&] { G tmp(a); });
        out.push_back({"copy_ctor", {ns, al.allocations, al.bytes, 0}});
    }
    // --- копирующее присваивание (в сетку того же размера).
    {
        G b = make(n, 2.0f);
        const double ns = median_ns(reps, [&] {
            Stopwatch sw;
            b = a;
            escape(&b);
            return sw.elapsed_ns();
        });
        const auto al = count_allocations([&] { b = a; });
        out.push_back({"copy_assign", {ns, al.allocations, al.bytes, 0}});
    }
    // --- перемещения слишком быстрые для одиночного замера:
    //     меряем серию из K операций и делим на K.
    constexpr int K = 10000;
    {
        const double ns = median_ns(reps, [&] {
            Stopwatch sw;
            for (int i = 0; i < K; ++i) {
                G c(std::move(a));  // перемещающий конструктор
                escape(&c);
                a.swap(c);          // вернуть ресурс в a (обмен полей)
                escape(&a);
            }
            return sw.elapsed_ns() / K;
        });
        const auto al = count_allocations([&] { G c(std::move(a)); a.swap(c); });
        out.push_back({"move_ctor", {ns, al.allocations, al.bytes, 0}});
    }
    {
        G b(std::move(a));
        a = std::move(b);  // теперь b пуст, a снова владеет данными
        const double ns = median_ns(reps, [&] {
            Stopwatch sw;
            for (int i = 0; i < K; ++i) {
                b = std::move(a);
                escape(&b);
                a = std::move(b);
                escape(&a);
            }
            return sw.elapsed_ns() / (2.0 * K);
        });
        const auto al = count_allocations([&] { b = std::move(a); });
        a = std::move(b);
        out.push_back({"move_assign", {ns, al.allocations, al.bytes, 0}});
    }
    g_sink = g_sink + a(0, 0);
    return out;
}

// Число копий элементов на операцию — на типе-счётчике Tracked.
template <template <class> class G>
std::vector<std::int64_t> count_element_copies(unsigned n) {
    using support::Tracked;
    std::vector<std::int64_t> copies;
    G<Tracked> a(n, n, Tracked(1));
    auto measure = [&](auto op) {
        Tracked::reset_stats();
        op();
        return Tracked::stats().copies();
    };
    copies.push_back(measure([&] { G<Tracked> c(a); }));
    G<Tracked> b(n, n, Tracked(2));
    copies.push_back(measure([&] { b = a; }));
    copies.push_back(measure([&] { G<Tracked> c(std::move(a)); a.swap(c); }));
    copies.push_back(measure([&] { b = std::move(a); a = std::move(b); }));
    return copies;
}

template <class T>
using Nested2 = grid::Grid<T, 2>;

// --------------------------------------------------------------- доступ

// Обход сетки n×n с суммированием; get(y, x) — способ доступа.
template <class Get>
std::int64_t traverse2(unsigned n, bool by_rows, Get get) {
    std::int64_t sum = 0;
    if (by_rows) {
        for (unsigned y = 0; y < n; ++y)
            for (unsigned x = 0; x < n; ++x) sum += get(y, x);
    } else {
        for (unsigned x = 0; x < n; ++x)
            for (unsigned y = 0; y < n; ++y) sum += get(y, x);
    }
    return sum;
}

template <class Get>
std::int64_t traverse3(unsigned n, bool by_rows, Get get) {
    std::int64_t sum = 0;
    for (unsigned a = 0; a < n; ++a)
        for (unsigned b = 0; b < n; ++b)
            for (unsigned c = 0; c < n; ++c)
                sum += by_rows ? get(a, b, c) : get(c, b, a);
    return sum;
}

}  // namespace

// =============================================================================
void run_copy_move(const Config& cfg) {
    CsvWriter csv(cfg.out_dir / "copy_move.csv",
                  {"impl", "n", "elements", "operation", "ns", "allocations", "bytes",
                   "element_copies"});
    const auto sizes = powers_of_two(8, cfg.quick ? 512 : 2048);
    for (unsigned n : sizes) {
        const int reps = n <= 512 ? 21 : 11;
        auto flat = measure_copy_move<FlatGrid>(
            n, [](unsigned s, float v) { return FlatGrid(s, s, v); }, reps);
        auto nested = measure_copy_move<NestedGrid>(
            n, [](unsigned s, float v) { return NestedGrid(s, s, v); }, reps);
        const auto flat_copies = count_element_copies<grid::v1::Grid>(n);
        const auto nested_copies = count_element_copies<Nested2>(n);

        const std::size_t elements = static_cast<std::size_t>(n) * n;
        for (std::size_t i = 0; i < flat.size(); ++i) {
            const auto& [op, f] = flat[i];
            const auto& r = nested[i].second;
            csv.row("flat", n, elements, op, f.ns, f.allocations, f.bytes, flat_copies[i]);
            csv.row("nested", n, elements, op, r.ns, r.allocations, r.bytes, nested_copies[i]);
        }
        std::cout << "  copy/move n=" << n << ": copy " << flat[0].second.ns << " / "
                  << nested[0].second.ns << " ns, move " << flat[2].second.ns << " / "
                  << nested[2].second.ns << " ns (flat / nested)\n";
    }
    std::cout << "-> " << csv.path().string() << "\n";
}

// =============================================================================
void run_access(const Config& cfg) {
    CsvWriter csv(cfg.out_dir / "access.csv",
                  {"dims", "n", "elements", "method", "order", "ns_per_element"});

    // ---- 2D
    for (unsigned n : powers_of_two(64, cfg.quick ? 1024 : 4096)) {
        const int reps = n <= 1024 ? 21 : 9;
        const std::size_t elements = static_cast<std::size_t>(n) * n;

        std::vector<Elem> vec(elements, 1);
        FlatGridI flat(n, n, 1);
        NestedGridI nested(n, n, 1);

        // Каждый метод — свой лямбда-шаблон доступа; traverse2 инстанцируется
        // отдельно для каждого, поэтому вызовы get(...) встраиваются (inline).
        auto time_method = [&](const char* name, bool rows, auto get) {
            const double ns = median_ns(reps, [&] {
                Stopwatch sw;
                g_isink = g_isink + traverse2(n, rows, get);
                return sw.elapsed_ns();
            });
            csv.row(2, n, elements, name, rows ? "rows" : "columns",
                    ns / static_cast<double>(elements));
            return ns / static_cast<double>(elements);
        };
        for (bool rows : {true, false}) {
            // Эталон: «ручная» индексация плоского массива. Указатель и ширина
            // захватываются ПО ЗНАЧЕНИЮ, как они лежат в полях сетки.
            const double base = time_method(
                "std::vector", rows,
                [p = vec.data(), w = static_cast<std::size_t>(n)](unsigned y, unsigned x) {
                    return p[y * w + x];
                });
            time_method("flat g(y,x)", rows, [&](unsigned y, unsigned x) { return flat(y, x); });
            time_method("flat g[y][x]", rows, [&](unsigned y, unsigned x) { return flat[y][x]; });
            const double nc =
                time_method("nested g(y,x)", rows, [&](unsigned y, unsigned x) { return nested(y, x); });
            time_method("nested g[y][x]", rows,
                        [&](unsigned y, unsigned x) { return nested[y][x]; });
            if (rows)
                std::cout << "  access 2D n=" << n << " rows: vector " << base
                          << " ns/el, nested g(y,x) " << nc << " ns/el\n";
        }
    }

    // ---- 3D: плоский массив против рекурсивной сетки Grid<float, 3>
    for (unsigned n : powers_of_two(8, cfg.quick ? 128 : 256)) {
        const int reps = n <= 128 ? 21 : 9;
        const std::size_t elements = static_cast<std::size_t>(n) * n * n;
        std::vector<Elem> vec(elements, 1);
        NestedGrid3I nested(n, n, n, 1);

        auto time_method = [&](const char* name, bool rows, auto get) {
            const double ns = median_ns(reps, [&] {
                Stopwatch sw;
                g_isink = g_isink + traverse3(n, rows, get);
                return sw.elapsed_ns();
            });
            csv.row(3, n, elements, name, rows ? "rows" : "columns",
                    ns / static_cast<double>(elements));
        };
        for (bool rows : {true, false}) {
            time_method("std::vector", rows,
                        [p = vec.data(), w = static_cast<std::size_t>(n)](unsigned a, unsigned b,
                                                                          unsigned c) {
                            return p[(a * w + b) * w + c];
                        });
            time_method("nested g(z,y,x)", rows,
                        [&](unsigned a, unsigned b, unsigned c) { return nested(a, b, c); });
            time_method("nested g[z][y][x]", rows,
                        [&](unsigned a, unsigned b, unsigned c) { return nested[a][b][c]; });
        }
        std::cout << "  access 3D n=" << n << " done\n";
    }
    std::cout << "-> " << csv.path().string() << "\n";
}

// =============================================================================
void run_construct(const Config& cfg) {
    CsvWriter csv(cfg.out_dir / "construct.csv",
                  {"impl", "dims", "d0", "d1", "d2", "elements", "allocations", "bytes",
                   "bytes_per_element", "construct_ns", "destroy_ns"});
    const unsigned log_e = cfg.quick ? 18 : 20;
    const unsigned total = 1u << log_e;  // одинаковый объём для всех форм
    const int reps = 11;

    // Замер создания и разрушения сетки, которую строит make().
    auto measure = [&](const char* impl, int dims, unsigned d0, unsigned d1, unsigned d2,
                       auto make) {
        using G = decltype(make());
        std::optional<G> g;
        std::vector<double> c_ns, d_ns;
        for (int r = 0; r < reps; ++r) {
            Stopwatch sw;
            g.emplace(make());
            c_ns.push_back(sw.elapsed_ns());
            sw.restart();
            g.reset();
            d_ns.push_back(sw.elapsed_ns());
        }
        const auto al = count_allocations([&] { G tmp = make(); });
        csv.row(impl, dims, d0, d1, d2, total, al.allocations, al.bytes,
                static_cast<double>(al.bytes) / total, median(c_ns), median(d_ns));
    };

    // 2D: d0 строк × (total / d0) столбцов
    for (unsigned k = 0; k <= log_e; k += 2) {
        const unsigned d0 = 1u << k, d1 = total / d0;
        measure("flat", 2, d0, d1, 0, [=] { return FlatGrid(d0, d1, 1.0f); });
        measure("nested", 2, d0, d1, 0, [=] { return NestedGrid(d0, d1, 1.0f); });
    }
    // 3D: d × d × (total / d²)
    for (unsigned k = 0; 2 * k <= log_e; k += 2) {
        const unsigned d = 1u << k, d2 = total / (d * d);
        measure("nested", 3, d, d, d2, [=] { return NestedGrid3(d, d, d2, 1.0f); });
    }
    std::cout << "-> " << csv.path().string() << "\n";
}

}  // namespace bench
