#pragma once
// -----------------------------------------------------------------------------
// Минимальный тестовый фреймворк (чтобы проект собирался без интернета и
// внешних зависимостей вроде GoogleTest).
//
//   TEST(name) { CHECK(cond); CHECK_EQ(a, b); CHECK_THROWS(expr); }
//
// Тесты регистрируются статически, run_all_tests() запускает их все.
// -----------------------------------------------------------------------------

#include <functional>
#include <sstream>
#include <string>

namespace testing_lite {

struct TestFailure {
    std::string message;
};

// Регистрирует тест в глобальном списке; возвращает true (для static-инициализации).
bool register_test(const char* name, std::function<void()> body);

// Запускает все тесты, печатает отчёт. Возвращает число упавших тестов.
int run_all_tests();

[[noreturn]] void fail(const char* file, int line, const std::string& what);

template <class A, class B>
void check_eq(const A& a, const B& b, const char* sa, const char* sb, const char* file, int line) {
    if (!(a == b)) {
        std::ostringstream os;
        os << sa << " == " << sb << "  (" << a << " vs " << b << ")";
        fail(file, line, os.str());
    }
}

}  // namespace testing_lite

#define TL_CONCAT_IMPL(a, b) a##b
#define TL_CONCAT(a, b) TL_CONCAT_IMPL(a, b)

#define TEST(name)                                                              \
    static void name();                                                         \
    [[maybe_unused]] static const bool TL_CONCAT(name, _registered) =           \
        ::testing_lite::register_test(#name, &name);                            \
    static void name()

#define CHECK(cond)                                                             \
    do {                                                                        \
        if (!(cond)) ::testing_lite::fail(__FILE__, __LINE__, #cond);           \
    } while (0)

#define CHECK_EQ(a, b) ::testing_lite::check_eq((a), (b), #a, #b, __FILE__, __LINE__)

#define CHECK_THROWS(expr)                                                      \
    do {                                                                        \
        bool thrown_ = false;                                                   \
        try {                                                                   \
            (void)(expr);                                                       \
        } catch (...) {                                                         \
            thrown_ = true;                                                     \
        }                                                                       \
        if (!thrown_) ::testing_lite::fail(__FILE__, __LINE__, "expected exception: " #expr); \
    } while (0)
