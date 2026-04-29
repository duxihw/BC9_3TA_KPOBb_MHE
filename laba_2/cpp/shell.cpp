#include <iostream>

// Общая функция сортировки Шелла
void shell_sort_core(unsigned* arr, unsigned N, unsigned* gaps, unsigned gap_count, unsigned long long& swaps) {
    for (unsigned k = 0; k < gap_count; ++k) {
        unsigned gap = gaps[k];
        for (unsigned i = gap; i < N; i++) {
            unsigned temp = arr[i];
            unsigned j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];
                swaps++;
            }
            arr[j] = temp;
        }
    }
}

// 1. Стандартная последовательность: d_{i+1} = d_i / 2
void shell_sort_classic(unsigned* arr, unsigned N, unsigned long long& swaps) {
    unsigned count = 0;
    for (unsigned d = N / 2; d > 0; d /= 2) count++;
    
    unsigned* gaps = new unsigned[count];
    unsigned idx = 0;
    for (unsigned d = N / 2; d > 0; d /= 2) gaps[idx++] = d;

    shell_sort_core(arr, N, gaps, count, swaps);
    delete[] gaps;
}

// 2. Последовательность Хиббарда: 2^i - 1
void shell_sort_hibbard(unsigned* arr, unsigned N, unsigned long long& swaps) {
    unsigned count = 0;
    while (((1 << (count + 1)) - 1) <= N) count++;

    unsigned* gaps = new unsigned[count];
    for (unsigned i = 0; i < count; ++i) {
        gaps[count - i - 1] = (1 << (i + 1)) - 1;
    }

    shell_sort_core(arr, N, gaps, count, swaps);
    delete[] gaps;
}

// 3. Числа Фибоначчи
void shell_sort_fibonacci(unsigned* arr, unsigned N, unsigned long long& swaps) {
    unsigned* temp_fib = new unsigned[N + 2]; 
    temp_fib[0] = 1; temp_fib[1] = 2;
    unsigned count = 2;
    
    while (true) {
        unsigned next = temp_fib[count - 1] + temp_fib[count - 2];
        if (next > N) break;
        temp_fib[count++] = next;
    }

    unsigned* gaps = new unsigned[count];
    for (unsigned i = 0; i < count; ++i) {
        gaps[i] = temp_fib[count - i - 1];
    }

    shell_sort_core(arr, N, gaps, count, swaps);
    
    delete[] temp_fib;
    delete[] gaps;
}