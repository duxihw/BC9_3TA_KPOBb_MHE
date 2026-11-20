#include <iostream>

template <typename T, unsigned N = 2>
class Grid;

// База рекурсии: одномерная сетка хранит сами элементы.
template <typename T>
class Grid<T, 1> final {
public:
    using value_type = T;
    using size_type = unsigned;

    Grid() = default;

    explicit Grid(size_type count) : size(count), data(new T[size]) {}

    Grid(size_type count, const T& value) : Grid(count) {
        *this = value;
    }

    // Копирование создаёт отдельный массив.
    Grid(const Grid& other) : size(other.size), data(new T[size]) {
        for (size_type i = 0; i < size; ++i) data[i] = other.data[i];
    }

    // Перемещение передаёт владение массивом.
    Grid(Grid&& other) noexcept : size(other.size), data(other.data) {
        other.size = 0;
        other.data = nullptr;
    }

    Grid& operator=(const Grid& other) {
        if (this != &other) {
            Grid copy(other);
            swap(copy);
        }
        return *this;
    }

    Grid& operator=(Grid&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;
            other.size = 0;
            other.data = nullptr;
        }
        return *this;
    }

    ~Grid() { delete[] data; }

    Grid& operator=(const T& value) {
        for (size_type i = 0; i < size; ++i) data[i] = value;
        return *this;
    }

    size_type get_size() const { return size; }

    T& operator[](size_type i) { return data[i]; }
    T operator[](size_type i) const { return data[i]; }
    T& operator()(size_type i) { return data[i]; }
    T operator()(size_type i) const { return data[i]; }

private:
    size_type size = 0;
    T* data = nullptr;

    void swap(Grid& other) noexcept {
        size_type old_size = size;
        size = other.size;
        other.size = old_size;
        T* old_data = data;
        data = other.data;
        other.data = old_data;
    }
};

// Каждый уровень добавляет измерение: массив сеток размерности N - 1.
template <typename T, unsigned N>
class Grid final {
    static_assert(N > 1, "Grid dimension must be positive");

public:
    using value_type = T;
    using size_type = unsigned;

    Grid() = default;

    // Сетка 1 x ... x 1 с одним значением.
    Grid(const T& value) : size(1), data(new Grid<T, N - 1>[1]) {
        if constexpr (N == 2) data[0] = Grid<T, 1>(1, value);
        else data[0] = Grid<T, N - 1>(value);
    }

    // Первый размер относится к этому уровню, остальные — к подсеткам.
    template <typename... Args>
    Grid(size_type count, const Args&... args)
        : size(count), data(new Grid<T, N - 1>[size]) {
        static_assert(sizeof...(Args) == N - 1 || sizeof...(Args) == N,
                      "Expected N sizes and an optional fill value");
        for (size_type i = 0; i < size; ++i)
            data[i] = Grid<T, N - 1>(args...);
    }

    // Копирование создаёт отдельный массив.
    Grid(const Grid& other) : size(other.size), data(new Grid<T, N - 1>[size]) {
        for (size_type i = 0; i < size; ++i) data[i] = other.data[i];
    }

    // Перемещение передаёт владение массивом.
    Grid(Grid&& other) noexcept : size(other.size), data(other.data) {
        other.size = 0;
        other.data = nullptr;
    }

    Grid& operator=(const Grid& other) {
        if (this != &other) {
            Grid copy(other);
            swap(copy);
        }
        return *this;
    }

    Grid& operator=(Grid&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;
            other.size = 0;
            other.data = nullptr;
        }
        return *this;
    }

    ~Grid() { delete[] data; }

    Grid& operator=(const T& value) {
        for (size_type i = 0; i < size; ++i) data[i] = value;
        return *this;
    }

    size_type get_size() const { return size; }

    // Изменяемую подсетку возвращаем по ссылке, константную — копируем.
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
        const Grid<T, N - 1>& subgrid = data[first];
        return subgrid(rest...);
    }

    size_type get_y_size() const { return size; }
    size_type get_x_size() const {
        return size == 0 ? 0 : data[0].get_size();
    }

private:
    size_type size = 0;
    Grid<T, N - 1>* data = nullptr;

    void swap(Grid& other) noexcept {
        size_type old_size = size;
        size = other.size;
        other.size = old_size;
        Grid<T, N - 1>* old_data = data;
        data = other.data;
        other.data = old_data;
    }
};

int main() {
    Grid<float> single = 3.5f;
    std::cout << "Single element: " << single(0, 0) << '\n';

    Grid<float> g(3, 2, 0.0f);
    std::cout << "Grid size: " << g.get_y_size() << " x "
              << g.get_x_size() << '\n';

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

    // Приведение к rvalue-ссылке выбирает конструктор перемещения.
    Grid<float> moved = static_cast<Grid<float>&&>(copy);
    std::cout << "Moved element: " << moved(0, 0) << '\n';

    Grid<float> assigned;
    assigned = g;
    assigned = static_cast<Grid<float>&&>(moved);
    std::cout << "After move assignment: " << assigned(0, 0) << '\n';

    g = 5.0f;
    std::cout << "After filling: " << g(2, 1) << '\n';
    // Трёхмерная сетка состоит из двумерных, а те — из строк.
    const Grid<float, 3> cube(2, 3, 4, 1.0f);
    Grid<float> slice(2, 5, 2.0f);
    slice = cube[1];
    std::cout << "Cube element: " << cube(1, 1, 1) << '\n';
    std::cout << "Slice size: " << slice.get_y_size() << " x "
              << slice.get_x_size() << '\n';

    Grid<int, 4> space(2, 3, 4, 5, 0);
    space[1][2][3][4] = 7;
    std::cout << "4D element: " << space(1, 2, 3, 4) << '\n';
    return 0;
}
