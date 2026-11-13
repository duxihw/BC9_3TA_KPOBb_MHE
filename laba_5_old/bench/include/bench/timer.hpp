#pragma once
// Простые средства замера времени.

#include <chrono>
#include <utility>
#include <vector>

namespace bench {

class Stopwatch {
public:
    using clock = std::chrono::steady_clock;  // монотонные часы, не «прыгают»

    Stopwatch() : start_(clock::now()) {}
    void restart() { start_ = clock::now(); }
    double elapsed_ns() const;

private:
    clock::time_point start_;
};

// Время выполнения f() в наносекундах.
template <class F>
double time_ns(F&& f) {
    Stopwatch sw;
    std::forward<F>(f)();
    return sw.elapsed_ns();
}

// Медиана — устойчивая к выбросам оценка (фоновые процессы, прерывания ОС).
double median(std::vector<double> values);
double mean(const std::vector<double>& values);

// Не даёт компилятору выбросить «бесполезные» вычисления в замерах.
void do_not_optimize(std::size_t value);

// Барьер оптимизатора: компилятор обязан считать, что объект по адресу p
// мог быть прочитан/изменён «кем-то ещё». Без этого цикл
//     for (...) { b = std::move(a); a = std::move(b); }
// оптимизатор видит как «ничего не изменилось» и удаляет целиком.
template <class T>
inline void escape(T* p) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(p) : "memory");
#else
    static volatile const void* sink;
    sink = p;
#endif
}

}  // namespace bench
