/* dft.h */
#ifndef __DFT_H__
#define __DFT_H__

#include <complex>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include "abort.h"
#include "data.h"

// Constant for PI
const float PI = 3.141592653589793238460;

// ---------------------------------------------------------------------
// Helper: Perform Bit-Reversal Permutation and Iterative FFT
// Implements Cooley-Tukey Algorithm (O(N log N))
// ---------------------------------------------------------------------
template <typename T>
void fft_1d_iterative(std::complex<T>* data, int n) {
    // 1. Bit-Reversal Permutation
    int j = 0;
    for (int i = 0; i < n; ++i) {
        if (i < j) {
            std::swap(data[i], data[j]);
        }
        int m = n >> 1;
        while (m >= 1 && j & m) {
            j ^= m;
            m >>= 1;
        }
        j ^= m;
    }

    // 2. Iterative FFT (Danielson-Lanczos Lemma)
    // m: size of the current sub-DFT (2, 4, 8, ... n)
    for (int m = 2; m <= n; m <<= 1) {
        T theta = -2.0 * PI / m;
        std::complex<T> w_m(std::cos(theta), std::sin(theta)); // Complex weight base
        
        // Process each block of size m
        for (int k = 0; k < n; k += m) {
            std::complex<T> w(1.0, 0.0); // w starts at 1
            for (int x = 0; x < m / 2; ++x) {
                // Butterfly Operation
                // u = Even part, t = w * Odd part
                std::complex<T> t = w * data[k + x + m / 2];
                std::complex<T> u = data[k + x];

                data[k + x] = u + t;
                data[k + x + m / 2] = u - t;

                w *= w_m; // Update weight
            }
        }
    }
}

// ---------------------------------------------------------------------
// Helper: Matrix Transpose
// Transposes the matrix held in 'data' (width x height).
// Since we sync data globally, everyone holds the full matrix to transpose.
// ---------------------------------------------------------------------
template <typename T>
void transpose(std::complex<T>* data, unsigned width, unsigned height) {
    std::complex<T>* temp = new std::complex<T>[width * height];

    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            // temp[row][col] = data[col][row]
            // Input is row-major: index = y * width + x
            // Transposed index: x * height + y
            temp[x * height + y] = data[y * width + x];
        }
    }

    std::memcpy(data, temp, sizeof(std::complex<T>) * width * height);
    delete[] temp;
}

// ---------------------------------------------------------------------
// Helper: Custom Allgather using MPI_Irecv and MPI_Send
// Synchronizes partial results from all ranks to construct the full matrix.
// ---------------------------------------------------------------------
template <typename T>
void collect_results(std::complex<T>* data, unsigned width, unsigned height, 
                     int num_ranks, int my_rank, 
                     int my_start_row, int my_num_rows) {
    
    std::vector<MPI_Request> requests(num_ranks);
    
    // Calculate layout for all ranks to determine offsets
    int rows_per_rank = height / num_ranks;
    int remainder = height % num_ranks;
    int current_offset = 0;

    // 1. Post Non-blocking Receives (Irecv)
    for (int r = 0; r < num_ranks; ++r) {
        int r_rows = rows_per_rank + (r < remainder ? 1 : 0);
        int r_count = r_rows * width;

        // Receive chunk from rank 'r'
        // Note: MPI_COMPLEX matches std::complex<float> layout
        MPI_Irecv(&data[current_offset], r_count, MPI_COMPLEX, r, 0, MPI_COMM_WORLD, &requests[r]);
        
        current_offset += r_count;
    }

    // 2. Broadcast (Send) my computed chunk to ALL ranks
    int my_data_count = my_num_rows * width;
    int my_offset_idx = my_start_row * width;

    for (int r = 0; r < num_ranks; ++r) {
        MPI_Send(&data[my_offset_idx], my_data_count, MPI_COMPLEX, r, 0, MPI_COMM_WORLD);
    }

    // 3. Wait for completion
    MPI_Waitall(num_ranks, requests.data(), MPI_STATUSES_IGNORE);
}

// ---------------------------------------------------------------------
// Main Function: 2-D Discrete Fourier Transform
// ---------------------------------------------------------------------
template <typename T>
void dft2d(std::complex<T> *data, const unsigned width, const unsigned height,
           const int num_ranks, const int rank_id) {
    
    // --- 1. Load Balancing Calculation ---
    // Determine which rows this rank is responsible for.
    int rows_per_rank = height / num_ranks;
    int remainder = height % num_ranks;
    
    // Calculate start row index
    int my_start_row = rows_per_rank * rank_id + (rank_id < remainder ? rank_id : remainder);
    // Calculate number of rows to process
    int my_num_rows = rows_per_rank + (rank_id < remainder ? 1 : 0);

    // --- Step a: Row-wise 1D DFT ---
    for (int r = 0; r < my_num_rows; ++r) {
        int global_row_idx = my_start_row + r;
        // Pointer to the start of the current row
        std::complex<T>* row_ptr = &data[global_row_idx * width];
        
        // Perform FFT on this row in-place
        fft_1d_iterative(row_ptr, width);
    }

    // Sync: Gather all row-wise results
    collect_results(data, width, height, num_ranks, rank_id, my_start_row, my_num_rows);

    // --- Step b: Transpose ---
    // Now everyone has the full Row-FFT matrix. Transpose it.
    // After transpose, 'width' becomes 'height' logically, but buffer size is same.
    transpose(data, width, height);

    // --- Step c: Row-wise 1D DFT (on Transposed Matrix) ---
    // Since we transposed, we are now technically processing columns of the original matrix.
    // Dimensions are swapped: Width <-> Height
    int t_width = height;
    int t_height = width;

    // Recalculate load balancing for the transposed dimensions
    rows_per_rank = t_height / num_ranks;
    remainder = t_height % num_ranks;
    my_start_row = rows_per_rank * rank_id + (rank_id < remainder ? rank_id : remainder);
    my_num_rows = rows_per_rank + (rank_id < remainder ? 1 : 0);

    for (int r = 0; r < my_num_rows; ++r) {
        int global_row_idx = my_start_row + r;
        std::complex<T>* row_ptr = &data[global_row_idx * t_width];
        
        fft_1d_iterative(row_ptr, t_width);
    }

    // Sync: Gather all results again
    collect_results(data, t_width, t_height, num_ranks, rank_id, my_start_row, my_num_rows);

    // --- Step d: Transpose Back ---
    transpose(data, t_width, t_height);
}

#endif