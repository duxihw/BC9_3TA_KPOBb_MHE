#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <fstream>

struct IntArray {
private:
    int* data_;
    size_t length_;

public:
    IntArray() : data_(nullptr), length_(0) {}
    
    IntArray(size_t size) : length_(size) {
        data_ = new int[length_]();
    }
    
    IntArray(const IntArray& other) : length_(other.length_) {
        data_ = new int[length_];
        for (size_t i = 0; i < length_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    IntArray(size_t size, int value) : length_(size) {
        data_ = new int[length_]();
        for (size_t i = 0; i < length_; i++) data_[i] = value;
    }
    
    IntArray(IntArray&& other) noexcept 
        : data_(other.data_), length_(other.length_) {
        other.data_ = nullptr;
        other.length_ = 0;
    }
    
    ~IntArray() {
        delete[] data_;
    }
    
    IntArray& operator=(const IntArray& other) {
        if (this != &other) {
            IntArray temp(other);
            swap(temp);
        }
        return *this;
    }
    
    IntArray& operator=(IntArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            length_ = other.length_;
            other.data_ = nullptr;
            other.length_ = 0;
        }
        return *this;
    }
    
    size_t size() const {
        return length_;
    }
    
    int* begin() {
        return data_;
    }
    
    const int* begin() const {
        return data_;
    }
    
    int* end() {
        return data_ + length_;
    }
    
    const int* end() const {
        return data_ + length_;
    }
    
    int& operator[](size_t index) {
        return data_[index];
    }
    
    const int& operator[](size_t index) const {
        return data_[index];
    }
    
    void swap(IntArray& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(length_, other.length_);
    }
};

// Стратегия A: переместить найденный элемент в начало
bool search_A(IntArray& arr, int key) {
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
bool search_B(IntArray& arr, int key) {
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
bool search_C(IntArray& arr, IntArray& counts, int key) {
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
    int sizes[] = {100, 500, 1000, 5000, 10000, 50000, 100000};
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
        IntArray base_arr(N);
        for (int i = 0; i < N; ++i) base_arr[i] = i;

        // --- Равномерное распределение ---
        std::uniform_int_distribution<int> uniform_key(0, N-1);
        std::uniform_int_distribution<int> uniform_outside(N, N + 1000);

        // Стратегия A
        {
            IntArray arr = base_arr;
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
            IntArray arr = base_arr;
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
            IntArray arr = base_arr;
            IntArray counts(N, 0);
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
        IntArray weights(N);
        for (int i = 0; i < N; ++i) weights[i] = (N/2 - i)*(N/2 - i);
        std::discrete_distribution<int> nonuniform_dist(weights.begin(), weights.end());

        // Стратегия A
        {
            IntArray arr = base_arr;
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
            IntArray arr = base_arr;
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
            IntArray arr = base_arr;
            IntArray counts(N, 0);
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
