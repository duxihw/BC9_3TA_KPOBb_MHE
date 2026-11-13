#pragma once
// -----------------------------------------------------------------------------
// Tracked — «подопытный» тип элемента для тестов и бенчмарка.
//
//  * НЕ имеет конструктора по умолчанию (проверяем требование условия:
//    сетка должна работать с такими типами через конструктор (3));
//  * считает все конструирования, копирования, перемещения и разрушения —
//    так видно, сколько раз сетка копирует элементы и нет ли утечек
//    (после разрушения всех сеток alive() должно быть 0);
//  * умеет бросать исключение на k-м копировании — для проверки
//    безопасности исключений (не должно быть утечек и «полуобъектов»).
// -----------------------------------------------------------------------------

#include <cstdint>
#include <stdexcept>

namespace support {

struct LifetimeStats {
    std::int64_t value_ctor = 0;   // Tracked(int)
    std::int64_t copy_ctor = 0;
    std::int64_t move_ctor = 0;
    std::int64_t copy_assign = 0;
    std::int64_t move_assign = 0;
    std::int64_t dtor = 0;

    std::int64_t alive() const noexcept {
        return value_ctor + copy_ctor + move_ctor - dtor;
    }
    std::int64_t copies() const noexcept { return copy_ctor + copy_assign; }
};

class CopyLimitExceeded : public std::runtime_error {
public:
    CopyLimitExceeded() : std::runtime_error("Tracked: copy limit exceeded") {}
};

class Tracked {
public:
    explicit Tracked(int value);
    Tracked(const Tracked& other);
    Tracked(Tracked&& other) noexcept;
    Tracked& operator=(const Tracked& other);
    Tracked& operator=(Tracked&& other) noexcept;
    ~Tracked();

    int value() const noexcept { return value_; }
    friend bool operator==(const Tracked& a, const Tracked& b) { return a.value_ == b.value_; }

    // Глобальная статистика (общая для всех объектов Tracked).
    static const LifetimeStats& stats() noexcept;
    static void reset_stats() noexcept;

    // Через сколько успешных копирований бросить исключение (-1 — никогда).
    static void throw_after_copies(std::int64_t n) noexcept;

private:
    static void on_copy();

    int value_;
};

}  // namespace support
