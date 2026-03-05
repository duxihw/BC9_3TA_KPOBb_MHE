#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>

// Полный перебор O(N^2)
bool two_sum_bruteforce(const std::vector<int>& arr, int target) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (arr[i] + arr[j] == target) return true;
        }
    }
    return false;
}

// Два указателя для отсортированного массива O(N)
bool two_sum_sorted(const std::vector<int>& arr, int target) {
    int left = 0, right = arr.size() - 1;
    while (left < right) {
        int sum = arr[left] + arr[right];
        if (sum == target) return true;
        if (sum < target) ++left;
        else --right;
    }
    return false;
}

int main() {
    std::vector<int> sizes = {100, 500, 1000, 5000};
    // std::random_device rd;
    // std::mt19937 gen(rd());
    unsigned seed = 8080;
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, 100'000);

    std::ofstream file_brute("data/sum_bruteforce.txt");
    std::ofstream file_linear("data/sum_linear.txt");

    for (int N : sizes) {
        // Создаём массив случайных чисел
        std::vector<int> arr(N);
        for (int i = 0; i < N; ++i) arr[i] = dist(gen);

        // Для линейного алгоритма нужна отсортированная копия
        std::vector<int> sorted_arr = arr;
        std::sort(sorted_arr.begin(), sorted_arr.end());

        // Измерение полного перебора
        auto start = std::chrono::steady_clock::now();
        for (int r = 0; r < 1'000; ++r) {
            two_sum_bruteforce(arr, dist(gen));
        }
        auto end = std::chrono::steady_clock::now();
        auto time_brute = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение полного перебора (худший случай, key=-1)
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 1'000; ++r) {
            two_sum_bruteforce(arr, -1);
        }
        end = std::chrono::steady_clock::now();
        auto time_brute_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


        // Измерение линейного алгоритма
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 100'000; ++r) {
            two_sum_sorted(sorted_arr, dist(gen));
        }
        end = std::chrono::steady_clock::now();
        auto time_linear = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение линейного алгоритма (худший случай, key=-1)
        start = std::chrono::steady_clock::now();
        for (int r = 0; r < 100'000; ++r) {
            two_sum_sorted(sorted_arr, -1);
        }
        end = std::chrono::steady_clock::now();
        auto time_linear_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        file_brute << N << " " << time_brute << " " << time_brute_bad << std::endl;
        file_linear << N << " " << time_linear << " " << time_linear_bad << std::endl;

        std::cout << "N = " << N << " done" << std::endl;
    }

    file_brute.close();
    file_linear.close();
    return 0;
}
