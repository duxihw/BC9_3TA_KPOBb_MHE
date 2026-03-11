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

// Полный перебор O(N^2)
bool two_sum_bruteforce(const IntArray& arr, int target) {
    int n = arr.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (arr[i] + arr[j] == target) return true;
        }
    }
    return false;
}

// Два указателя для отсортированного массива O(N)
bool two_sum_sorted(const IntArray& arr, int target) {
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
    int sizes[] = {100, 500, 1000, 5000};
    std::random_device rd;
    std::mt19937 gen(rd());
    // unsigned seed = 8080;
    // std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, 100'000);

    std::ofstream file_brute("data/sum_bruteforce.txt");
    std::ofstream file_linear("data/sum_linear.txt");

    for (int N : sizes) {
        // Создаём массив случайных чисел
        IntArray arr(N);
        for (int i = 0; i < N; ++i) arr[i] = dist(gen);

        // Для линейного алгоритма нужна отсортированная копия
        IntArray sorted_arr = arr;
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
            two_sum_sorted(sorted_arr, -dist(gen));
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
