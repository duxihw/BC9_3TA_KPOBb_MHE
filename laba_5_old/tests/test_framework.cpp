#include "test_framework.hpp"

#include <exception>
#include <iostream>
#include <utility>
#include <vector>

namespace testing_lite {

namespace {

struct TestCase {
    const char* name;
    std::function<void()> body;
};

// Функция со статической переменной вместо глобального вектора —
// защищает от «static initialization order fiasco».
std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

}  // namespace

bool register_test(const char* name, std::function<void()> body) {
    registry().push_back({name, std::move(body)});
    return true;
}

void fail(const char* file, int line, const std::string& what) {
    throw TestFailure{std::string(file) + ":" + std::to_string(line) + ": " + what};
}

int run_all_tests() {
    int failed = 0;
    for (const auto& t : registry()) {
        try {
            t.body();
            std::cout << "[  OK  ] " << t.name << '\n';
        } catch (const TestFailure& f) {
            ++failed;
            std::cout << "[ FAIL ] " << t.name << "\n         " << f.message << '\n';
        } catch (const std::exception& e) {
            ++failed;
            std::cout << "[ FAIL ] " << t.name << "\n         unexpected exception: " << e.what()
                      << '\n';
        }
    }
    std::cout << "\n" << registry().size() - static_cast<std::size_t>(failed) << "/"
              << registry().size() << " tests passed\n";
    return failed;
}

}  // namespace testing_lite

int main() { return testing_lite::run_all_tests() == 0 ? 0 : 1; }
