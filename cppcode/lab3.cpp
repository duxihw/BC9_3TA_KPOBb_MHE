#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>

// Стратегия A: переместить найденный элемент в начало
bool search_A(std::vector<int>& arr, int key) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        if (arr[i] == key) {
            if (i != 0) std::swap(arr[i], arr[0]);
            return true;
        }
    }
    return false;
}

// Стратегия B: обменять с предыдущим
bool search_B(std::vector<int>& arr, int key) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        if (arr[i] == key) {
            if (i != 0) std::swap(arr[i], arr[i-1]);
            return true;
        }
    }
    return false;
}

// Стратегия C: счётчики, при превышении левого соседа обмен
bool search_C(std::vector<int>& arr, std::vector<int>& counts, int key) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        if (arr[i] == key) {
            counts[i]++;
            if (i > 0 && counts[i] > counts[i-1]) {
                std::swap(arr[i], arr[i-1]);
                std::swap(counts[i], counts[i-1]);
            }
            return true;
        }
    }
    return false;
}

int main() {
    std::vector<int> sizes = {100, 500, 1000, 5000, 10000, 50000, 100000};
    const int queries = 100'000;      // число запросов для каждого N
    double fail_prob = 0.05;         // вероятность не найти элемент

    // std::random_device rd;
    // std::mt19937 gen(rd());
    unsigned seed = 8080;
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    // Файлы для результатов
    std::ofstream file_A_uniform("data/strategyA_uniform.txt");
    std::ofstream file_A_nonuniform("data/strategyA_nonuniform.txt");
    std::ofstream file_B_uniform("data/strategyB_uniform.txt");
    std::ofstream file_B_nonuniform("data/strategyB_nonuniform.txt");
    std::ofstream file_C_uniform("data/strategyC_uniform.txt");
    std::ofstream file_C_nonuniform("data/strategyC_nonuniform.txt");

    for (int N : sizes) {
        std::cout << "Processing N = " << N << std::endl;

        // Исходный массив: 0, 1, 2, ..., N-1
        std::vector<int> base_arr(N);
        for (int i = 0; i < N; ++i) base_arr[i] = i;

        // --- Равномерное распределение ---
        std::uniform_int_distribution<int> uniform_key(0, N-1);
        std::uniform_int_distribution<int> uniform_outside(N, N + 1000);

        // Стратегия A
        {
            std::vector<int> arr = base_arr;
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : uniform_key(gen);
                search_A(arr, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_A_uniform << N << " " << time << std::endl;
        }

        // Стратегия B
        {
            std::vector<int> arr = base_arr;
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : uniform_key(gen);
                search_B(arr, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_B_uniform << N << " " << time << std::endl;
        }

        // Стратегия C
        {
            std::vector<int> arr = base_arr;
            std::vector<int> counts(N, 0);
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : uniform_key(gen);
                search_C(arr, counts, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_C_uniform << N << " " << time << std::endl;
        }

        // --- Неравномерное распределение ---
        // Веса: чем меньше индекс, тем выше вероятность (линейно убывающие)
        std::vector<double> weights(N);
        for (int i = 0; i < N; ++i) weights[i] = N - i;
        std::discrete_distribution<int> nonuniform_dist(weights.begin(), weights.end());

        // Стратегия A
        {
            std::vector<int> arr = base_arr;
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : nonuniform_dist(gen);
                search_A(arr, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_A_nonuniform << N << " " << time << std::endl;
        }

        // Стратегия B
        {
            std::vector<int> arr = base_arr;
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : nonuniform_dist(gen);
                search_B(arr, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_B_nonuniform << N << " " << time << std::endl;
        }

        // Стратегия C
        {
            std::vector<int> arr = base_arr;
            std::vector<int> counts(N, 0);
            auto start = std::chrono::steady_clock::now();
            for (int q = 0; q < queries; ++q) {
                int key = (uni(gen) < fail_prob) ? uniform_outside(gen) : nonuniform_dist(gen);
                search_C(arr, counts, key);
            }
            auto end = std::chrono::steady_clock::now();
            auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            file_C_nonuniform << N << " " << time << std::endl;
        }

    }

    // Закрываем файлы
    file_A_uniform.close();
    file_A_nonuniform.close();
    file_B_uniform.close();
    file_B_nonuniform.close();
    file_C_uniform.close();
    file_C_nonuniform.close();

    return 0;
}
