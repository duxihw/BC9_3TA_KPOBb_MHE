#pragma once
// Определения методов grid::Grid<T, N>. Подключается из grid.hpp.

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "grid/detail/memory.hpp"
#include "grid/grid.hpp"

namespace grid {

// ============================================================ конструкторы

// Служебный конструктор: через него работают все остальные.
template <typename T, std::size_t N>
Grid<T, N>::Grid(detail::construct_tag, dims_type const& dims, T const* value) {
    const std::size_t n = dims[0];
    element_type* p = detail::allocate<element_type>(n);
    // Охранник освободит память (и разрушит созданные подсетки), если
    // где-то в глубине рекурсии конструктор T бросит исключение.
    detail::ConstructionGuard<element_type> guard(p, n);

    if constexpr (N == 1) {
        // Нижний уровень — элементы T. Эти алгоритмы при исключении сами
        // разрушают уже созданные элементы, охраннику остаётся вернуть память.
        if (value != nullptr)
            std::uninitialized_fill_n(p, n, *value);
        else if constexpr (std::is_default_constructible_v<T>)
            std::uninitialized_value_construct_n(p, n);
        // Для T без конструктора по умолчанию ветка value == nullptr
        // недостижима: конструктор (2) для таких T запрещён ограничением.
    } else {
        // Верхние уровни — подсетки Grid<T, N-1> с размерами dims[1..N-1].
        typename Grid<T, N - 1>::dims_type sub{};
        for (std::size_t d = 1; d < N; ++d) sub[d - 1] = dims[d];
        for (std::size_t i = 0; i < n; ++i) {
            // placement new: создаём объект в уже выделенной памяти.
            ::new (static_cast<void*>(p + i)) element_type(detail::construct_tag{}, sub, value);
            guard.constructed_one();
        }
    }

    guard.release();  // всё создано успешно — теперь памятью владеет *this
    dims_ = dims;
    data_ = p;
}

// (1) T -> сетка 1×…×1.
template <typename T, std::size_t N>
Grid<T, N>::Grid(T const& t)
    requires(N >= 2)
    : Grid(detail::construct_tag{}, [] {
          dims_type ones;
          ones.fill(1);
          return ones;
      }(), std::addressof(t)) {}

// (2) Размеры, элементы по умолчанию.
template <typename T, std::size_t N>
template <class... Sizes>
    requires(sizeof...(Sizes) == N && detail::all_convertible_to<unsigned, Sizes...> &&
             std::is_default_constructible_v<T>)
Grid<T, N>::Grid(Sizes... sizes)
    : Grid(detail::construct_tag{}, dims_type{static_cast<size_type>(sizes)...}, nullptr) {}

// (3) Размеры + значение. Последний аргумент может быть другого типа
// (например, 1.0 вместо 1.0f) — тогда он один раз преобразуется в T.
template <typename T, std::size_t N>
template <class... Args>
    requires detail::sizes_and_value<unsigned, T, N, Args...>
Grid<T, N>::Grid(Args const&... args) {
    auto tuple = std::forward_as_tuple(args...);
    dims_type dims{};
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((dims[I] = static_cast<size_type>(std::get<I>(tuple))), ...);
    }(std::make_index_sequence<N>{});

    auto build = [&](T const& value) {
        Grid tmp(detail::construct_tag{}, dims, std::addressof(value));
        swap(tmp);  // *this был пуст, tmp заберёт «пустоту» и ничего не освободит
    };
    using Last = std::tuple_element_t<N, std::tuple<Args...>>;
    if constexpr (std::is_same_v<Last, T>)
        build(std::get<N>(tuple));     // без лишней копии
    else
        build(T(std::get<N>(tuple)));  // явное преобразование, например double -> float
}

// ============================================================ правило пяти

template <typename T, std::size_t N>
Grid<T, N>::Grid(Grid const& other) {
    const std::size_t n = other.dims_[0];
    element_type* p = detail::allocate<element_type>(n);
    detail::ConstructionGuard<element_type> guard(p, n);
    // Для N > 1 это рекурсивно вызывает копирующие конструкторы подсеток —
    // получается полная («глубокая») копия всей структуры.
    std::uninitialized_copy_n(other.data_, n, p);
    guard.release();
    dims_ = other.dims_;
    data_ = p;
}

template <typename T, std::size_t N>
Grid<T, N>::Grid(Grid&& other) noexcept
    : dims_(std::exchange(other.dims_, dims_type{})),
      data_(std::exchange(other.data_, nullptr)) {}

template <typename T, std::size_t N>
Grid<T, N>& Grid<T, N>::operator=(Grid const& other) {
    if (this != &other) {
        Grid tmp(other);  // copy-and-swap: строгая гарантия
        swap(tmp);
    }
    return *this;
}

template <typename T, std::size_t N>
Grid<T, N>& Grid<T, N>::operator=(Grid&& other) noexcept {
    if (this != &other) {
        Grid tmp(std::move(other));
        swap(tmp);
    }
    return *this;
}

template <typename T, std::size_t N>
Grid<T, N>::~Grid() {
    // Разрушаем подсетки (каждая освободит своё), затем свой массив.
    std::destroy_n(data_, dims_[0]);
    detail::deallocate(data_, dims_[0]);
}

template <typename T, std::size_t N>
void Grid<T, N>::swap(Grid& other) noexcept {
    std::swap(dims_, other.dims_);
    std::swap(data_, other.data_);
}

// ============================================================ доступ

template <typename T, std::size_t N>
auto Grid<T, N>::operator[](size_type idx) noexcept -> element_type& {
    assert(idx < dims_[0]);
    return data_[idx];
}

template <typename T, std::size_t N>
auto Grid<T, N>::operator[](size_type idx) const -> element_type {
    assert(idx < dims_[0]);
    return data_[idx];  // копия
}

template <typename T, std::size_t N>
T& Grid<T, N>::ref(size_type const* idx) noexcept {
    assert(idx[0] < dims_[0]);
    if constexpr (N == 1)
        return data_[idx[0]];
    else
        return data_[idx[0]].ref(idx + 1);  // спуск к подсетке без копирования
}

template <typename T, std::size_t N>
T const& Grid<T, N>::ref(size_type const* idx) const noexcept {
    assert(idx[0] < dims_[0]);
    if constexpr (N == 1)
        return data_[idx[0]];
    else
        return data_[idx[0]].ref(idx + 1);
}

template <typename T, std::size_t N>
T const& Grid<T, N>::checked_ref(size_type const* idx) const {
    if (idx[0] >= dims_[0])
        throw std::out_of_range("Grid::at: index " + std::to_string(idx[0]) +
                                " out of range [0, " + std::to_string(dims_[0]) + ")");
    if constexpr (N == 1)
        return data_[idx[0]];
    else
        return data_[idx[0]].checked_ref(idx + 1);
}

// operator() константной сетки идёт по ссылкам напрямую: никаких копий
// подсеток не создаётся, копируется только итоговый элемент.
template <typename T, std::size_t N>
template <class... Idx>
    requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
T& Grid<T, N>::operator()(Idx... idx) noexcept {
    const dims_type i{static_cast<size_type>(idx)...};
    return ref(i.data());
}

template <typename T, std::size_t N>
template <class... Idx>
    requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
T Grid<T, N>::operator()(Idx... idx) const {
    const dims_type i{static_cast<size_type>(idx)...};
    return ref(i.data());
}

template <typename T, std::size_t N>
template <class... Idx>
    requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
T& Grid<T, N>::at(Idx... idx) {
    const dims_type i{static_cast<size_type>(idx)...};
    return const_cast<T&>(checked_ref(i.data()));
}

template <typename T, std::size_t N>
template <class... Idx>
    requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
T const& Grid<T, N>::at(Idx... idx) const {
    const dims_type i{static_cast<size_type>(idx)...};
    return checked_ref(i.data());
}

template <typename T, std::size_t N>
Grid<T, N>& Grid<T, N>::operator=(T const& t) {
    // Для N > 1 data_[i] = t вызывает этот же оператор у подсетки
    // (точное совпадение типа важнее преобразования T -> Grid).
    for (size_type i = 0; i < dims_[0]; ++i) data_[i] = t;
    return *this;
}

template <typename T, std::size_t N>
std::size_t Grid<T, N>::total_size() const noexcept {
    std::size_t total = 1;
    for (size_type d : dims_) total *= d;
    return total;
}

}  // namespace grid
