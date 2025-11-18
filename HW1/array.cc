#include "array.h"
#include <cstdlib> // For malloc, free
#include <new>     // For placement new
#include <algorithm> // For std::min

// Class constructor
array_t::array_t() :
ptr(0),
num_elements(0),
array_size(0) {
// Nothing to do
}

/**
 * @brief Constructs an array_t by performing a deep copy of the source array.
 * @param m_array The source array_t instance to copy from.
 */
array_t::array_t(const array_t& m_array) :
ptr(0),
num_elements(0),
array_size(0) {
    if (m_array.array_size > 0) {
        // 1. Allocate raw memory space using malloc().
        ptr = (data_t*)malloc(sizeof(data_t) * m_array.array_size);
        if (ptr == nullptr) {
            // Handle allocation failure if necessary, though typical for assignments is to assume success.
        }

        // 2. Deep copy elements using placement new (copy constructor of data_t).
        for (size_t i = 0; i < m_array.num_elements; ++i) {
            new (&ptr[i]) data_t(m_array.ptr[i]);
        }

        // 3. Update metadata.
        num_elements = m_array.num_elements;
        array_size = m_array.array_size;
    }
}

/**
 * @brief Destroys the array_t instance and deallocates memory.
 *
 * All data_t instances must be explicitly destructed before deallocating the raw memory.
 */
array_t::~array_t() {
    if (ptr != nullptr) {
        // Explicitly call the destructor for each constructed element.
        for (size_t i = 0; i < num_elements; ++i) {
            ptr[i].~data_t();
        }
        // Deallocate the raw memory block allocated by malloc().
        free(ptr);
    }
    // No need to reset members, as the object is being destroyed.
}


/**
 * @brief Allocates a memory space for the specified number of elements.
 * @param m_array_size The new capacity to reserve.
 *
 * Uses malloc/free and placement new for element copying, ensuring data_t life-cycle management.
 */
void array_t::reserve(const size_t m_array_size) {
    // Only proceed if requested size is greater than current capacity.
    if (m_array_size > array_size) {
        // 1. Allocate new raw memory space.
        data_t *new_ptr = (data_t*)malloc(sizeof(data_t) * m_array_size);
        if (new_ptr == nullptr) {
            // Error handling for memory allocation failure (e.g., throw exception or exit)
            return;
        }

        // 2. Copy elements from old array to new array using data_t's copy constructor (Placement New).
        // Only copy up to the minimum of the old element count and the new size (though new size is > old capacity,
        // we only copy 'num_elements').
        for (size_t i = 0; i < num_elements; ++i) {
            new (&new_ptr[i]) data_t(ptr[i]);
        }

        // 3. Handle old memory block.
        if (ptr != nullptr) {
            // Explicitly call the destructor for elements in the old block.
            for (size_t i = 0; i < num_elements; ++i) {
                ptr[i].~data_t();
            }
            // Deallocate the old raw memory block.
            free(ptr);
        }

        // 4. Update member variables.
        ptr = new_ptr;
        array_size = m_array_size;
    }
}

/**
 * @brief Adds a new element at the end of the array.
 * @param m_value The data_t value to insert.
 *
 * Checks capacity and calls reserve() to double the size if full.
 */
void array_t::push_back(const data_t m_value) {
    // 1. Check if the array is full.
    if (num_elements == array_size) {
        size_t new_size = array_size;
        
        // Determine the next capacity.
        if (array_size == 0) {
            new_size = 1; // Initial capacity is 1.
        } else {
            new_size *= 2; // Double the capacity.
        }
        
        // Reserve the new capacity.
        reserve(new_size);
    }

    // 2. Construct the new element in the reserved space using Placement New (Copy Construction).
    // The element m_value is copied into the raw memory at index num_elements.
    new (&ptr[num_elements]) data_t(m_value);

    // 3. Update the number of elements.
    num_elements++;
}