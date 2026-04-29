#include <iostream>

// Проверка порядка с заданным шагом
bool comb_step(unsigned* arr, unsigned const begin_idx, unsigned const end_idx, unsigned step, unsigned long long& swaps) {
    bool swapped = false;
    for (unsigned i = begin_idx; i + step < end_idx; ++i) {
        if (arr[i] > arr[i + step]) {
            unsigned temp = arr[i];
            arr[i] = arr[i + step];
            arr[i + step] = temp;
            swaps++;
            swapped = true;
        }
    }
    return swapped;
}

void comb_sort(unsigned* arr, unsigned const begin_idx, unsigned const end_idx, unsigned long long& swaps) {
    unsigned N = end_idx - begin_idx;
    unsigned step = N / 2;
    bool swapped = true;

    while (step > 1 || swapped) {
        if (step < 1) step = 1;
        swapped = comb_step(arr, begin_idx, end_idx, step, swaps);
        if (step > 1) step /= 2;
    }
}