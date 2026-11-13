#pragma once
// =============================================================================
//  Части 1–2 лабораторной: двумерная сетка Grid<T>, которая САМА управляет
//  своей памятью (RAII + «правило пяти») и поддерживает g[y][x].
//
//  Это доработанный класс из условия. Что изменено и почему:
//
//  * Исходный класс получал указатель на чужую память (Grid(T* data, ...)),
//    а освобождать её должен был внешний код. Теперь память выделяется в
//    конструкторе и освобождается в деструкторе — это и есть RAII
//    (Resource Acquisition Is Initialization): время жизни ресурса совпадает
//    со временем жизни объекта, утечка или двойное освобождение невозможны.
//
//  * Поля data, y_size, x_size были объявлены const. Константное поле нельзя
//    изменить после конструирования, а значит невозможны присваивание и
//    перемещение (у перемещённого объекта надо обнулить указатель). Поэтому
//    const убран.
//
//  * Вместо удалённых (= delete) копирования и перемещения реализованы все
//    пять специальных функций («правило пяти»): если класс владеет ресурсом
//    и ему нужен свой деструктор, то ему нужны и свои копирование и
//    перемещение — иначе сгенерированные компилятором версии скопируют
//    указатель, и память будет освобождена дважды.
//
//  * Элементы хранятся в одном непрерывном блоке по строкам (row-major):
//    элемент (y, x) лежит по смещению y * x_size + x.
//
//  Определения методов — в grid2d_impl.hpp.
// =============================================================================

#include <cstddef>

namespace grid::v1 {

template <typename T>
class Grid final {
public:
    using value_type = T;
    using size_type = unsigned;

    // (1) Неявное преобразование T -> Grid<T>: сетка 1×1 с копией t.
    //     explicit не ставим специально: по условию преобразование неявное,
    //     поэтому работает, например, Grid<float> g = 2.5f;
    Grid(T const& t);  // NOLINT(google-explicit-constructor)

    // (2) Сетка y_size × x_size из элементов, созданных конструктором по умолчанию.
    //     Используется value-инициализация: для классов это вызов T(),
    //     а для float/int — ноль (иначе значения были бы неопределёнными).
    //     Этот конструктор требует от T наличия конструктора по умолчанию —
    //     но так как это шаблон, он инстанцируется, только если его вызовут.
    Grid(size_type y_size, size_type x_size);

    // (3) Сетка y_size × x_size из копий t. Работает для любых T с
    //     конструктором копирования, в том числе без конструктора по умолчанию.
    Grid(size_type y_size, size_type x_size, T const& t);

    // ---- правило пяти
    Grid(Grid const& other);                 // глубокая копия: O(y·x)
    Grid(Grid&& other) noexcept;             // забираем указатель: O(1)
    Grid& operator=(Grid const& other);      // copy-and-swap, строгая гарантия
    Grid& operator=(Grid&& other) noexcept;  // O(1)
    ~Grid();

    void swap(Grid& other) noexcept;

    // ---- доступ к элементам (как в исходном классе)
    T operator()(size_type y_idx, size_type x_idx) const;  // копия
    T& operator()(size_type y_idx, size_type x_idx);       // ссылка

    // ---- оператор индексирования: g[y][x]
    // g[y] возвращает указатель на начало строки y, а второе [x] — это уже
    // встроенная индексация указателя: (data + y*x_size)[x]. Никаких
    // промежуточных объектов не создаётся — это самый быстрый вариант.
    T const* operator[](size_type y_idx) const;
    T* operator[](size_type y_idx);

    // Заполнить всю сетку копиями t (размер не меняется).
    Grid& operator=(T const& t);

    size_type get_y_size() const noexcept { return y_size_; }
    size_type get_x_size() const noexcept { return x_size_; }
    std::size_t size() const noexcept { return static_cast<std::size_t>(y_size_) * x_size_; }

    friend bool operator==(Grid const& a, Grid const& b) {
        if (a.y_size_ != b.y_size_ || a.x_size_ != b.x_size_) return false;
        for (std::size_t i = 0; i < a.size(); ++i)
            if (!(a.data_[i] == b.data_[i])) return false;
        return true;
    }

private:
    // Проверяет, что y_size * x_size помещается в size_t (иначе length_error).
    static std::size_t checked_area(size_type y_size, size_type x_size);

    T* data_ = nullptr;
    size_type y_size_ = 0;
    size_type x_size_ = 0;
};

template <typename T>
void swap(Grid<T>& a, Grid<T>& b) noexcept {
    a.swap(b);
}

}  // namespace grid::v1

#include "grid/grid2d_impl.hpp"
