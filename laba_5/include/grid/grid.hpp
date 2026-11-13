#pragma once
// =============================================================================
//  Часть 3 лабораторной: сетка произвольной размерности Grid<T, N>.
//
//  Устройство — РЕКУРСИВНОЕ:
//
//      Grid<T, N>  владеет массивом из dims[0] объектов Grid<T, N-1>,
//      Grid<T, 1>  владеет массивом из dims[0] объектов T.
//
//      Grid<float,3> g(2, 3, 4)          data_ ──► [ Grid<float,2> | Grid<float,2> ]
//                                                         │
//                                                         ▼
//                                         data_ ──► [ Grid<float,1> ×3 ]
//                                                         │
//                                                         ▼
//                                         data_ ──► [ float ×4 ]
//
//  Почему так, а не один плоский массив?
//    По условию g[i] должен возвращать ССЫЛКУ на сетку меньшей размерности
//    (для неконстантной сетки). Ссылку можно вернуть только на существующий
//    объект — а в рекурсивной схеме подсетки Grid<T, N-1> действительно
//    существуют внутри Grid<T, N>. Тогда:
//      * g[i] = другая_сетка   меняет содержимое g (это ссылка);
//      * g[i][j][k]            — цепочка ссылок до элемента T&;
//      * для константной сетки g[i] возвращает КОПИЮ подсетки.
//    Цена — больше выделений памяти (см. исследование в docs/REPORT.md).
//
//  Каждый уровень — полноценный RAII-класс с правилом пяти, поэтому
//  копирование, перемещение и освобождение рекурсивно работают «сами»:
//  деструктор Grid<T,N> разрушает свои подсетки, а те — свои элементы.
//
//  Grid<T> (N = 2 по умолчанию) полностью удовлетворяет и частям 1–2:
//  конструкторы (1)–(3), get_y_size/get_x_size, g[y][x], g(y, x).
//
//  Определения — в grid_impl.hpp.
// =============================================================================

#include <array>
#include <cstddef>
#include <type_traits>

#include "grid/detail/traits.hpp"

namespace grid {

namespace detail {
struct construct_tag {};  // метка для служебного конструктора
}  // namespace detail

template <typename T, std::size_t N = 2>
class Grid final {
    static_assert(N >= 1, "Grid dimension must be at least 1");
    static_assert(std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>,
                  "Grid elements must be copy-constructible and copy-assignable");

public:
    using value_type = T;
    using size_type = unsigned;  // в ограничениях ниже написано unsigned — это он же
    using dims_type = std::array<size_type, N>;
    static constexpr std::size_t dimension = N;

    // Что хранится на верхнем уровне и что возвращает operator[]:
    // подсетка Grid<T, N-1> или, для N == 1, сам элемент T.
    using element_type = std::conditional_t<N == 1, T, Grid<T, N - 1>>;

    // Пустая сетка 0×…×0 (в таком же состоянии остаётся перемещённый объект).
    Grid() noexcept = default;

    // (1) Неявное преобразование T -> Grid<T, N>: сетка 1×…×1 с копией t.
    // Для N == 1 отключено: иначе запись Grid<int,1> g(5) стала бы
    // неоднозначной (5 элементов? или один элемент 5?).
    Grid(T const& t)  // NOLINT(google-explicit-constructor)
        requires(N >= 2);

    // (2) Grid(d0, d1, ..., d{N-1}) — элементы инициализируются по умолчанию
    // (value-initialization: T() для классов, 0 для арифметических типов).
    template <class... Sizes>
        requires(sizeof...(Sizes) == N && detail::all_convertible_to<unsigned, Sizes...> &&
                 std::is_default_constructible_v<T>)
    explicit Grid(Sizes... sizes);

    // (3) Grid(d0, d1, ..., d{N-1}, t) — все элементы — копии t.
    // Не требует от T конструктора по умолчанию.
    template <class... Args>
        requires detail::sizes_and_value<unsigned, T, N, Args...>
    Grid(Args const&... args);

    // ---- правило пяти
    Grid(Grid const& other);
    Grid(Grid&& other) noexcept;
    Grid& operator=(Grid const& other);
    Grid& operator=(Grid&& other) noexcept;
    ~Grid();

    void swap(Grid& other) noexcept;

    // ---- индексирование
    // Неконстантная сетка: ССЫЛКА на подсетку (или на элемент при N == 1).
    element_type& operator[](size_type idx) noexcept;
    // Константная сетка: КОПИЯ подсетки (или элемента при N == 1).
    element_type operator[](size_type idx) const;

    // ---- доступ по N индексам: g(i, j, k)
    template <class... Idx>
        requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
    T& operator()(Idx... idx) noexcept;

    template <class... Idx>
        requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
    T operator()(Idx... idx) const;

    // То же с проверкой границ: бросает std::out_of_range.
    template <class... Idx>
        requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
    T& at(Idx... idx);

    template <class... Idx>
        requires(sizeof...(Idx) == N && detail::all_convertible_to<unsigned, Idx...>)
    T const& at(Idx... idx) const;

    // Заполнить всю сетку копиями t (размеры не меняются).
    Grid& operator=(T const& t);

    // ---- размеры
    dims_type const& extents() const noexcept { return dims_; }
    size_type extent(std::size_t dim) const noexcept { return dims_[dim]; }
    std::size_t total_size() const noexcept;  // произведение всех размеров

    size_type get_y_size() const noexcept
        requires(N == 2)
    {
        return dims_[0];
    }
    size_type get_x_size() const noexcept
        requires(N == 2)
    {
        return dims_[1];
    }

    // ---- обход верхнего уровня: for (auto& row : g)
    element_type* begin() noexcept { return data_; }
    element_type* end() noexcept { return data_ + dims_[0]; }
    element_type const* begin() const noexcept { return data_; }
    element_type const* end() const noexcept { return data_ + dims_[0]; }

    friend bool operator==(Grid const& a, Grid const& b) {
        if (a.dims_ != b.dims_) return false;
        for (size_type i = 0; i < a.dims_[0]; ++i)
            if (!(a.data_[i] == b.data_[i])) return false;
        return true;
    }

private:
    // Подсетки и сетки других размерностей могут вызывать служебные методы.
    template <typename, std::size_t>
    friend class Grid;

    // Служебный конструктор: размеры dims, значение *value (nullptr ⇒ T()).
    Grid(detail::construct_tag, dims_type const& dims, T const* value);

    T& ref(size_type const* idx) noexcept;              // без проверок
    T const& ref(size_type const* idx) const noexcept;
    T const& checked_ref(size_type const* idx) const;   // с проверками

    dims_type dims_{};               // размеры по всем N осям
    element_type* data_ = nullptr;   // dims_[0] подсеток / элементов
};

template <typename T, std::size_t N>
void swap(Grid<T, N>& a, Grid<T, N>& b) noexcept {
    a.swap(b);
}

}  // namespace grid

#include "grid/grid_impl.hpp"
