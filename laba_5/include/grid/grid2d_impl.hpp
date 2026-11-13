#pragma once
// Определения методов grid::v1::Grid<T>. Подключается из grid2d.hpp.

#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "grid/detail/memory.hpp"
#include "grid/grid2d.hpp"

namespace grid::v1 {

template <typename T>
std::size_t Grid<T>::checked_area(size_type y_size, size_type x_size) {
    const std::size_t area = static_cast<std::size_t>(y_size) * x_size;
    if (x_size != 0 && area / x_size != y_size)
        throw std::length_error("Grid: y_size * x_size overflows size_t");
    return area;
}

// ------------------------------------------------------------ конструкторы

template <typename T>
Grid<T>::Grid(T const& t) : Grid(1, 1, t) {}

template <typename T>
Grid<T>::Grid(size_type y_size, size_type x_size) {
    const std::size_t n = checked_area(y_size, x_size);
    T* p = detail::allocate<T>(n);
    // Если конструктор T бросит исключение на k-м элементе,
    // std::uninitialized_value_construct_n сам разрушит первые k-1,
    // а мы освободим память. Так объект не «наполовину создан».
    try {
        std::uninitialized_value_construct_n(p, n);
    } catch (...) {
        detail::deallocate(p, n);
        throw;
    }
    data_ = p;
    y_size_ = y_size;
    x_size_ = x_size;
}

template <typename T>
Grid<T>::Grid(size_type y_size, size_type x_size, T const& t) {
    const std::size_t n = checked_area(y_size, x_size);
    T* p = detail::allocate<T>(n);
    try {
        std::uninitialized_fill_n(p, n, t);  // копирующий конструктор T
    } catch (...) {
        detail::deallocate(p, n);
        throw;
    }
    data_ = p;
    y_size_ = y_size;
    x_size_ = x_size;
}

// ------------------------------------------------------------ правило пяти

// Копирование: новая память + поэлементные копии. Указатель НЕ копируется.
template <typename T>
Grid<T>::Grid(Grid const& other) {
    const std::size_t n = other.size();
    T* p = detail::allocate<T>(n);
    try {
        std::uninitialized_copy_n(other.data_, n, p);
    } catch (...) {
        detail::deallocate(p, n);
        throw;
    }
    data_ = p;
    y_size_ = other.y_size_;
    x_size_ = other.x_size_;
}

// Перемещение: забираем ресурс у other и оставляем его пустым (0×0),
// чтобы его деструктор ничего не освободил. Ни одного выделения памяти и
// ни одного копирования элементов, поэтому noexcept.
template <typename T>
Grid<T>::Grid(Grid&& other) noexcept
    : data_(std::exchange(other.data_, nullptr)),
      y_size_(std::exchange(other.y_size_, 0)),
      x_size_(std::exchange(other.x_size_, 0)) {}

// Копирующее присваивание по идиоме copy-and-swap:
//   1) копируем other во временный объект (может бросить — *this не тронут);
//   2) обмениваемся с ним (noexcept);
//   3) деструктор временного освобождает НАШИ старые данные.
// Даёт строгую гарантию безопасности исключений и корректно работает
// при самоприсваивании.
template <typename T>
Grid<T>& Grid<T>::operator=(Grid const& other) {
    if (this != &other) {
        Grid tmp(other);
        swap(tmp);
    }
    return *this;
}

template <typename T>
Grid<T>& Grid<T>::operator=(Grid&& other) noexcept {
    if (this != &other) {
        Grid tmp(std::move(other));  // other становится пустым
        swap(tmp);                   // старые данные *this уйдут с tmp
    }
    return *this;
}

template <typename T>
Grid<T>::~Grid() {
    std::destroy_n(data_, size());
    detail::deallocate(data_, size());
}

template <typename T>
void Grid<T>::swap(Grid& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(y_size_, other.y_size_);
    std::swap(x_size_, other.x_size_);
}

// ------------------------------------------------------------ доступ

template <typename T>
T Grid<T>::operator()(size_type y_idx, size_type x_idx) const {
    assert(y_idx < y_size_ && x_idx < x_size_);
    return data_[static_cast<std::size_t>(y_idx) * x_size_ + x_idx];
}

template <typename T>
T& Grid<T>::operator()(size_type y_idx, size_type x_idx) {
    assert(y_idx < y_size_ && x_idx < x_size_);
    return data_[static_cast<std::size_t>(y_idx) * x_size_ + x_idx];
}

template <typename T>
T const* Grid<T>::operator[](size_type y_idx) const {
    assert(y_idx < y_size_);
    return data_ + static_cast<std::size_t>(y_idx) * x_size_;
}

template <typename T>
T* Grid<T>::operator[](size_type y_idx) {
    assert(y_idx < y_size_);
    return data_ + static_cast<std::size_t>(y_idx) * x_size_;
}

template <typename T>
Grid<T>& Grid<T>::operator=(T const& t) {
    for (auto it = data_, end = data_ + size(); it != end; ++it) *it = t;
    return *this;
}

}  // namespace grid::v1
