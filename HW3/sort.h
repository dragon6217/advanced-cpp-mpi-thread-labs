/* sort.h */
#ifndef __SORT_H__
#define __SORT_H__

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <thread>
#include <vector>
#include <iostream>

// Helper function: Merges two sorted subarrays into a temporary buffer, then copies back.
template <typename T>
void merge(T *array, T *temp, size_t left, size_t mid, size_t right) {
    size_t i = left;    // Index for left subarray
    size_t j = mid + 1; // Index for right subarray
    size_t k = left;    // Index for temp array

    // Compare and merge
    while (i <= mid && j <= right) {
        if (array[i] <= array[j]) {
            temp[k++] = array[i++];
        } else {
            temp[k++] = array[j++];
        }
    }

    // Copy remaining elements of left subarray
    while (i <= mid) {
        temp[k++] = array[i++];
    }

    // Copy remaining elements of right subarray
    while (j <= right) {
        temp[k++] = array[j++];
    }

    // Copy merged elements back to original array
    for (size_t l = left; l <= right; l++) {
        array[l] = temp[l];
    }
}

// Helper function: Serial Merge Sort (Base case or single thread)
template <typename T>
void merge_sort_serial(T *array, T *temp, size_t left, size_t right) {
    if (left < right) {
        size_t mid = left + (right - left) / 2;
        merge_sort_serial(array, temp, left, mid);
        merge_sort_serial(array, temp, mid + 1, right);
        merge(array, temp, left, mid, right);
    }
}

// Helper function: Parallel Merge Sort using std::thread
template <typename T>
void merge_sort_parallel(T *array, T *temp, size_t left, size_t right, unsigned num_threads) {
    // Base case: If only 1 thread is available or range is small, use serial sort.
    if (num_threads <= 1) {
        merge_sort_serial(array, temp, left, right);
        return;
    }

    if (left < right) {
        size_t mid = left + (right - left) / 2;

        // Split threads: Give half to the left child, half to the right.
        // If num_threads is odd, logic handles it, but assignment says power of 2.
        unsigned threads_left = num_threads / 2;
        unsigned threads_right = num_threads - threads_left;

        // Create a thread for the left half
        std::thread t([=]() {
            merge_sort_parallel(array, temp, left, mid, threads_left);
        });

        // Current thread handles the right half
        merge_sort_parallel(array, temp, mid + 1, right, threads_right);

        // Wait for the left thread to finish
        t.join();

        // Merge the two sorted halves
        merge(array, temp, left, mid, right);
    }
}

// Main entry point
template <typename T>
void sort(T *array, const size_t num_data, const unsigned num_threads) {
    if (num_data <= 1) return;

    // 1. Allocate a temporary buffer ONCE to avoid overhead during recursion.
    // Using new[] directly since we cannot use std::vector easily with pointer arithmetic 
    // in the recursive steps without passing iterators. 
    // We manage memory manually for performance and simplicity in this context.
    T *temp = new T[num_data];

    // 2. Start parallel merge sort.
    // range is [0, num_data - 1]
    merge_sort_parallel(array, temp, 0, num_data - 1, num_threads);

    // 3. Deallocate temporary buffer.
    delete[] temp;
}

#endif