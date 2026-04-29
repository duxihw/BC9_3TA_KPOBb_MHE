#include <iostream>

// Прямой проход: перемещает максимум в конец диапазона
void forward_step(unsigned* arr, unsigned const begin_idx, unsigned const end_idx) {
    for (unsigned i = begin_idx; i < end_idx; ++i) {
        if (arr[i] > arr[i + 1]) {
            unsigned temp = arr[i];
            arr[i] = arr[i + 1];
            arr[i + 1] = temp;
        }
    }
}

// Обратный проход: перемещает минимум в начало диапазона
void backward_step(unsigned* arr, unsigned const begin_idx, unsigned const end_idx) {
    for (unsigned i = end_idx; i > begin_idx; --i) {
        if (arr[i - 1] > arr[i]) {
            unsigned temp = arr[i];
            arr[i] = arr[i - 1];
            arr[i - 1] = temp;
        }
    }
}

// Основная функция шейкерной сортировки
void shaker_sort(unsigned* arr, unsigned const begin_idx, unsigned const end_idx) {
    if (begin_idx >= end_idx) return;

    unsigned left = begin_idx;
    unsigned right = end_idx;
    bool swapped = true;

    while (left < right && swapped) {
        swapped = false;
        
        forward_step(arr, left, right);
        right--;

        backward_step(arr, left, right);
        left++;
    }
}