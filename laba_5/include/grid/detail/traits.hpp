#pragma once
// Вспомогательные проверки для ограничений (requires) конструкторов Grid<T, N>.

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

namespace grid::detail {

// Все типы Args... неявно приводятся к Size (это индексы / размеры).
template <class Size, class... Args>
concept all_convertible_to = (std::convertible_to<Args, Size> && ...);

// Первые N типов из Args... приводятся к Size, а последний (N-й) — к T.
// Нужен для конструктора Grid(size_0, ..., size_{N-1}, value).
template <class Size, class T, class Tuple, std::size_t... I>
constexpr bool sizes_then_value(std::index_sequence<I...>) {
    constexpr std::size_t last = sizeof...(I);
    return (std::convertible_to<std::tuple_element_t<I, Tuple>, Size> && ...) &&
           std::convertible_to<std::tuple_element_t<last, Tuple>, T const&>;
}

template <class Size, class T, std::size_t N, class... Args>
concept sizes_and_value =
    sizeof...(Args) == N + 1 &&
    sizes_then_value<Size, T, std::tuple<Args...>>(std::make_index_sequence<N>{});

}  // namespace grid::detail
