#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>

// Линейный поиск
bool linear_search(const std::vector<int>& arr, int key) {
    for (int x : arr) {
        if (x == key) return true;
    }
    return false;
}

// Бинарный поиск (массив должен быть отсортирован)
bool binary_search(const std::vector<int>& arr, int key) {
    int left = 0, right = arr.size() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == key) return true;
        if (arr[mid] < key) left = mid + 1;
        else right = mid - 1;
    }
    return false;
}

int main() {
    std::vector<int> sizes = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    // std::random_device rd;
    // std::mt19937 gen(rd());
    unsigned seed = 8080;
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, 1'000'000);

    std::ofstream file_linear("data/linear_search.txt");
    std::ofstream file_binary("data/binary_search.txt");

    for (int N : sizes) {
        // Создаём массив случайных чисел
        std::vector<int> arr(N);
        for (int i = 0; i < N; ++i) arr[i] = dist(gen);

        // Для бинарного поиска нужна отсортированная копия
        std::vector<int> sorted_arr = arr;
        std::sort(sorted_arr.begin(), sorted_arr.end());


        // Измерение линейного поиска
        auto start = std::chrono::steady_clock::now();
        for (int r = 0; r < 10'000; ++r) {
            // Случайный ключ (средний случай)
            linear_search(arr, dist(gen));
        }
        auto end = std::chrono::steady_clock::now();
        auto time_linear = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение линейного поиска (худший случай, key=1'000'000)
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 10'000; ++r) {
            linear_search(arr, 1'000'000);
        }
        end = std::chrono::steady_clock::now();
        auto time_linear_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


        // Измерение бинарного поиска
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 1'000'000; ++r) {
            // Случайный ключ (средний случай)
            binary_search(sorted_arr, dist(gen));
        }
        end = std::chrono::steady_clock::now();
        auto time_binary = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение бинарного поиска (худший случай, key=1'000'000)
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 1'000'000; ++r) {
            binary_search(sorted_arr, 1'000'000);
        }
        end = std::chrono::steady_clock::now();
        auto time_binary_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        file_linear << N << " " << time_linear << " " << time_linear_bad << std::endl;
        file_binary << N << " " << time_binary << " " << time_binary_bad << std::endl;

        std::cout << "N = " << N << " done" << std::endl;
    }

    file_linear.close();
    file_binary.close();
    return 0;
}
