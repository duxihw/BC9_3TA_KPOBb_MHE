#pragma once

#include <memory>
#include <utility>
#include <vector>

template <typename T, unsigned N = 2>
class Grid;

// Одномерная сетка хранит сами элементы. Остальные сетки состоят из подсеток.
template <typename T>
class Grid<T, 1> final {
public:
    using value_type = T;
    using size_type = unsigned;

    Grid() = default;

    explicit Grid(size_type count) : size(count) {
        if (size == 0) return;
        data = allocator.allocate(size);
        try {
            // Вызываем конструктор по умолчанию для каждого элемента.
            // Значения обычных int и float при этом не инициализируются.
            std::uninitialized_default_construct_n(data, size);
        } catch (...) {
            allocator.deallocate(data, size);
            throw;
        }
    }

    Grid(size_type count, const T& value) : size(count) {
        if (size == 0) return;
        // allocate только выделяет память. Поэтому T может не иметь
        // конструктора по умолчанию: сразу создаём копии value.
        data = allocator.allocate(size);
        try {
            std::uninitialized_fill_n(data, size, value);
        } catch (...) {
            allocator.deallocate(data, size);
            throw;
        }
    }

    // Копирование создаёт отдельный массив, независимый от исходного.
    Grid(const Grid& other) : size(other.size) {
        if (size == 0) return;
        data = allocator.allocate(size);
        try {
            std::uninitialized_copy_n(other.data, size, data);
        } catch (...) {
            allocator.deallocate(data, size);
            throw;
        }
    }

    // При перемещении забираем массив, а other становится пустой сеткой.
    Grid(Grid&& other) noexcept {
        swap(other);
    }

    Grid& operator=(const Grid& other) {
        if (this != &other) {
            Grid copy(other);
            swap(copy);
            // Старый массив теперь принадлежит copy и освободится сам.
        }
        return *this;
    }

    Grid& operator=(Grid&& other) noexcept {
        if (this != &other) {
            Grid moved(std::move(other));
            swap(moved);
        }
        return *this;
    }

    ~Grid() {
        if (data != nullptr) {
            std::destroy_n(data, size);
            allocator.deallocate(data, size);
        }
    }

    Grid& operator=(const T& value) {
        for (size_type i = 0; i < size; ++i) data[i] = value;
        return *this;
    }

    T& operator[](size_type i) { return data[i]; }
    T operator[](size_type i) const { return data[i]; }

    T& operator()(size_type i) { return data[i]; }
    T operator()(size_type i) const { return data[i]; }

    size_type get_size() const { return size; }

private:
    size_type size = 0;
    std::allocator<T> allocator;
    T* data = nullptr;

    void swap(Grid& other) noexcept {
        std::swap(size, other.size);
        std::swap(data, other.data);
    }
};

// Например, Grid<int, 3> хранит двумерные сетки,
// а каждая двумерная сетка хранит одномерные строки.
template <typename T, unsigned N>
class Grid final {
    static_assert(N > 1, "Grid dimension must be positive");

public:
    using value_type = T;
    using size_type = unsigned;

    Grid() = default;

    // Неявное преобразование T в сетку 1 x 1 x ... x 1.
    Grid(const T& value) {
        if constexpr (N == 2) {
            data.emplace_back(1, value);
        } else {
            data.emplace_back(value);
        }
    }

    // Первый аргумент задаёт число подсеток, остальные передаются им.
    // Так работают и Grid<int>(3, 2), и Grid<int, 3>(2, 3, 4, 7).
    template <typename... Args>
    Grid(size_type count, const Args&... args)
        : data(count) {
        static_assert(sizeof...(Args) == N - 1 || sizeof...(Args) == N,
                      "Expected N sizes and an optional fill value");
        for (auto& subgrid : data) {
            subgrid = Grid<T, N - 1>(args...);
        }
    }

    // vector управляет массивом подсеток, поэтому здесь достаточно
    // стандартных методов правила пяти. Они вызывают методы подсеток.
    Grid(const Grid&) = default;
    Grid(Grid&&) noexcept = default;
    Grid& operator=(const Grid&) = default;
    Grid& operator=(Grid&&) noexcept = default;
    ~Grid() = default;

    Grid& operator=(const T& value) {
        for (auto& subgrid : data) subgrid = value;
        return *this;
    }

    // Для изменяемой сетки возвращаем ссылку, для константной — копию.
    Grid<T, N - 1>& operator[](size_type i) { return data[i]; }
    Grid<T, N - 1> operator[](size_type i) const { return data[i]; }

    template <typename... Indices>
    T& operator()(size_type first, Indices... rest) {
        static_assert(sizeof...(Indices) == N - 1, "Expected N indices");
        return data[first](rest...);
    }

    template <typename... Indices>
    T operator()(size_type first, Indices... rest) const {
        static_assert(sizeof...(Indices) == N - 1, "Expected N indices");
        return data[first](rest...);
    }

    size_type get_size() const { return static_cast<size_type>(data.size()); }
    size_type get_y_size() const { return get_size(); }
    size_type get_x_size() const {
        return data.empty() ? 0 : data[0].get_size();
    }

private:
    std::vector<Grid<T, N - 1>> data;
};
