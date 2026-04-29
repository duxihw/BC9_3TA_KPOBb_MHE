#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <filesystem>

void forward_step(unsigned* arr, unsigned const begin_idx, unsigned const end_idx);
void backward_step(unsigned* arr, unsigned const begin_idx, unsigned const end_idx);
void shaker_sort(unsigned* arr, unsigned begin_idx, unsigned end_idx);
void comb_sort(unsigned* arr, unsigned begin_idx, unsigned end_idx, unsigned long long& swaps);
void shell_sort_classic(unsigned* arr, unsigned N, unsigned long long& swaps);
void shell_sort_hibbard(unsigned* arr, unsigned N, unsigned long long& swaps);
void shell_sort_fibonacci(unsigned* arr, unsigned N, unsigned long long& swaps);

void fill_random(unsigned* arr, unsigned N) {
    static std::mt19937 gen(42);
    std::uniform_int_distribution<unsigned> dist(1, 100000);
    for (unsigned i = 0; i < N; ++i) arr[i] = dist(gen);
}

void copy_array(unsigned* src, unsigned* dst, unsigned N) {
    for (unsigned i = 0; i < N; ++i) dst[i] = src[i];
}

void shaker_run() {
    std::filesystem::create_directories("../../data/laba_2");
    std::ofstream file("../../data/laba_2/shaker_experiment.csv");
    file << "N,ForwardTime_ms,BackwardTime_ms,ShakerTime_ms\n";

    unsigned sizes[] = {100, 500, 1000, 5000, 10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000};

    std::cout << "shaker expirement run\n";
    
    for (unsigned N : sizes) {
        unsigned* original = new unsigned[N];
        unsigned* working = new unsigned[N];
        fill_random(original, N);

        copy_array(original, working, N);
        auto start = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < N; i++)
            forward_step(working, 0, N);
        auto end = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double, std::milli>(end - start).count();

        copy_array(original, working, N);
        start = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < N; i++)
            backward_step(working, 0, N);
        end = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double, std::milli>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        shaker_sort(original, 0, N);
        end = std::chrono::high_resolution_clock::now();
        double t3 = std::chrono::duration<double, std::milli>(end - start).count();

        file << N << "," << t1 << "," << t2 << "," << t3 << "\n";

        delete[] original;
        delete[] working;
        std::cout << "N=" << N << " processed.\n";
    }
}

void comb_run() {
    std::filesystem::create_directories("../../data/laba_2");
    std::ofstream file("../../data/laba_2/comb_experiment.csv");
    file << "N,CombTime_ms,CombSteps\n";

    unsigned sizes[] = {1000, 5000, 10000, 50000, 100000};

    std::cout << "comb expirement run\n";
    
    for (unsigned N : sizes) {
        unsigned* original = new unsigned[N];
        fill_random(original, N);
        unsigned long long s = 0;

        auto start = std::chrono::high_resolution_clock::now();
        comb_sort(original, 0, N, s);
        auto end = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double, std::milli>(end - start).count();

        
        file << N << "," << t1 << "," << s << "\n";

        delete[] original;
        std::cout << "N=" << N << " processed.\n";
    }
}

void shell_run() {
    std::filesystem::create_directories("../../data/laba_2");
    std::ofstream file("../../data/laba_2/shell_experiment.csv");
    file << "N,ShellTime_ms,ShellSwaps,HibbardTime_ms,HibbardSwaps,FiboTime_ms,FiboSwaps\n";

    unsigned sizes[] = {1000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000};
    
    std::cout << "shell expirement run\n";
    
    for (unsigned N : sizes) {
        unsigned* original = new unsigned[N];
        unsigned* working = new unsigned[N];
        fill_random(original, N);

        // Шелл Классика
        copy_array(original, working, N);
        unsigned long long s1_swaps = 0;
        auto start = std::chrono::high_resolution_clock::now();
        shell_sort_classic(working, N, s1_swaps);
        auto end = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double, std::milli>(end - start).count();

        // Хиббард
        copy_array(original, working, N);
        unsigned long long s2_swaps = 0;
        start = std::chrono::high_resolution_clock::now();
        shell_sort_hibbard(working, N, s2_swaps);
        end = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double, std::milli>(end - start).count();

        // Фибоначчи
        copy_array(original, working, N);
        unsigned long long s3_swaps = 0;
        start = std::chrono::high_resolution_clock::now();
        shell_sort_fibonacci(working, N, s3_swaps);
        end = std::chrono::high_resolution_clock::now();
        double t3 = std::chrono::duration<double, std::milli>(end - start).count();

        file << N << "," << t1 << "," << s1_swaps << "," 
                   << t2 << "," << s2_swaps << "," << t3 << "," << s3_swaps << "\n";

        delete[] original;
        delete[] working;
        std::cout << "N=" << N << " processed.\n";
    }
}

int main() {
    shaker_run();
    // comb_run();
    // shell_run();

    return 0;
}