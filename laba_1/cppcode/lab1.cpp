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

// Линейный поиск
bool linear_search(const IntArray& arr, int key, unsigned long long& o) {
    for (int x : arr) {
        o += 2;
        if (x == key) { ++o; return true; }
    }
    ++o;
    return false;
}

// Бинарный поиск (массив должен быть отсортирован)
bool binary_search(const IntArray& arr, int key, unsigned long long& o) {
    int left = 0, right = arr.size() - 1; ++o;
    while (left <= right) {
        ++o;
        int mid = left + (right - left) / 2; ++o;
        ++o;
        if (arr[mid] == key) { ++o; std::cout << arr[mid]; return true; }
        ++o;
        if (arr[mid] < key) { ++o; left = mid + 1; }
        else { ++o; right = mid - 1; }
    }
    ++o;
    return false;
}

int main() {
    int sizes[] = {100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};
    std::random_device rd;
    std::mt19937 gen(rd());
    // unsigned seed = 1234;
    // std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(0, 1e6);
    std::uniform_int_distribution<int> bool_dist(0, 1);

    std::ofstream file_linear("data/linear_search.txt");
    std::ofstream file_binary("data/binary_search.txt");

    for (int N : sizes) {
        // Создаём массив случайных чисел
        IntArray arr(N);
        for (int i = 0; i < N; ++i) arr[i] = dist(gen);

        // Для бинарного поиска нужна отсортированная копия
        IntArray sorted_arr = arr;
        std::sort(sorted_arr.begin(), sorted_arr.end());


        // Измерение линейного поиска
        // auto start = std::chrono::steady_clock::now();
        unsigned long long o = 0;
        for (int r = 0; r < 10'000; ++r) {
            // Случайный ключ (средний случай)
            linear_search(arr, dist(gen), o);
        }
        auto time_linear = o;
        // auto end = std::chrono::steady_clock::now();
        // auto time_linear = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение линейного поиска (худший случай, key=1e6)
        // start = std::chrono::steady_clock::now();
        o = 0;
        for (int r = 0; r < 10'000; ++r) {
            linear_search(arr, -dist(gen), o);
        }
        auto time_linear_bad = o;
        // end = std::chrono::steady_clock::now();
        // auto time_linear_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


        // Измерение бинарного поиска
        // start = std::chrono::steady_clock::now();
        o = 0;
        for (int r = 0; r < 1e6; ++r) {
            // Случайный ключ (средний случай)
            binary_search(sorted_arr, dist(gen), o);
        }
        auto time_binary = o;
        // end = std::chrono::steady_clock::now();
        // auto time_binary = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Измерение бинарного поиска (худший случай)
        // start = std::chrono::steady_clock::now();
        o = 0;
        for (int r = 0; r < 1e6; ++r) {
            binary_search(sorted_arr, sorted_arr[0 + bool_dist(gen) * (N - 1)], o);
        }
        auto time_binary_bad = o;
        // end = std::chrono::steady_clock::now();
        // auto time_binary_bad = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        file_linear << N << " " << time_linear << " " << time_linear_bad << std::endl;
        file_binary << N << " " << time_binary << " " << time_binary_bad << std::endl;

        std::cout << "N = " << N << " done" << std::endl;
    }

    file_linear.close();
    file_binary.close();
    return 0;
}
