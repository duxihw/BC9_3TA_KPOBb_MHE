#include "bench/timer.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace bench {

double Stopwatch::elapsed_ns() const {
    return std::chrono::duration<double, std::nano>(clock::now() - start_).count();
}

double median(std::vector<double> values) {
    if (values.empty()) throw std::invalid_argument("median of empty sample");
    const auto mid = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2);
    std::nth_element(values.begin(), mid, values.end());
    if (values.size() % 2 == 1) return *mid;
    const double upper = *mid;
    const double lower = *std::max_element(values.begin(), mid);
    return (lower + upper) / 2.0;
}

double mean(const std::vector<double>& values) {
    if (values.empty()) throw std::invalid_argument("mean of empty sample");
    return std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
}

namespace {
volatile std::size_t g_sink = 0;  // запись в volatile нельзя выбросить
}

void do_not_optimize(std::size_t value) { g_sink = g_sink + value; }

}  // namespace bench
