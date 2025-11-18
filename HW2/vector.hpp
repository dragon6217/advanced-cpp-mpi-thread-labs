/* vector.hpp */
#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include <cstdlib> // malloc, free
#include <new>     // placement new

// Constructor
template <typename T>
vector_t<T>::vector_t(void) :
    array(0),
    array_size(0),
    num_elements(0) {
    // Nothing to do
}

// Copy constructor
template <typename T>
vector_t<T>::vector_t(const vector_t<T> &m_vector) :
    array(0),
    array_size(m_vector.num_elements),
    num_elements(m_vector.num_elements) {
    // Copy constructor creates a copy of tight-fit array.
    array = (T*)malloc(sizeof(T) * array_size);
    for(size_t i = 0; i < num_elements; i++) {
        new (&array[i]) T(m_vector.array[i]);
    }
}

// Destructor
template <typename T>
vector_t<T>::~vector_t(void) {
    // Destruct all elements first, and then free the array.
    for(size_t i = 0; i < num_elements; i++) { array[i].~T(); }
    free(array);
}

// Get the number of elements in the array.
template <typename T>
inline size_t vector_t<T>::size(void) const { return num_elements; }

// Get the allocated size of array in unit of elements.
template <typename T>
inline size_t vector_t<T>::capacity(void) const { return array_size; }

// Get a reference of element at the given index.
template <typename T>
inline T& vector_t<T>::operator[](const size_t m_index) const { return array[m_index]; }

// Get an iterator pointing to the first element of array.
template <typename T>
inline typename vector_t<T>::iterator vector_t<T>::begin(void) const {
    return iterator_t<T>(array);
}

// Get an iterator pointing to the next of last element.
template <typename T>
inline typename vector_t<T>::iterator vector_t<T>::end(void) const {
    return iterator_t<T>(array+num_elements);
}

/*************************
 * EEE5501: Assignment 2 *
 *************************/

// Reserve an array space for the given number of elements.
template <typename T>
void vector_t<T>::reserve(size_t m_array_size) {
    // Only proceed if the requested size is greater than the current capacity.
    if (m_array_size > array_size) {
        // 1. Allocate new raw memory.
        T *new_array = (T*)malloc(sizeof(T) * m_array_size);
        
        // 2. Deep copy existing elements to the new memory using Placement New.
        for (size_t i = 0; i < num_elements; i++) {
            new (&new_array[i]) T(array[i]); // Copy construct
            array[i].~T();                   // Destruct old element
        }

        // 3. Free the old memory block.
        if (array != 0) {
            free(array);
        }

        // 4. Update pointers and capacity.
        array = new_array;
        array_size = m_array_size;
    }
}

// Remove all elements in the array.
template <typename T>
void vector_t<T>::clear(void) {
    // Destruct all elements explicitly.
    for (size_t i = 0; i < num_elements; i++) {
        array[i].~T();
    }
    // Reset size, but keep capacity (memory is not freed).
    num_elements = 0;
}

// Add a new element at the end of array.
template <typename T>
void vector_t<T>::push_back(const T &m_data) {
    // 1. Check if the array is full.
    if (num_elements == array_size) {
        // Double the capacity (start with 1 if empty).
        size_t new_capacity = (array_size == 0) ? 1 : array_size * 2;
        reserve(new_capacity);
    }

    // 2. Construct the new element at the end using Placement New.
    new (&array[num_elements]) T(m_data);
    
    // 3. Increment size.
    num_elements++;
}

// Remove the last element in the array.
template <typename T>
void vector_t<T>::pop_back(void) {
    if (num_elements > 0) {
        // 1. Decrement size first to get the index of the last element.
        num_elements--;
        
        // 2. Destruct the element explicitly.
        array[num_elements].~T();
    }
}

// Assign new contents to the array.
template <typename T>
vector_t<T>& vector_t<T>::operator=(const vector_t<T> &m_vector) {
    // 1. Check for self-assignment.
    if (this == &m_vector) {
        return *this;
    }

    // 2. Destruct all existing data (like clear).
    for (size_t i = 0; i < num_elements; i++) {
        array[i].~T();
    }

    // 3. Reallocate memory if necessary.
    // To simplify and match the "tight-fit" behavior of the copy constructor, 
    // we can just reallocate to match the source's size exactly.
    if (array_size < m_vector.num_elements) {
        if (array != 0) free(array);
        array = (T*)malloc(sizeof(T) * m_vector.num_elements);
        array_size = m_vector.num_elements;
    }

    // 4. Deep copy elements from source.
    num_elements = m_vector.num_elements;
    for (size_t i = 0; i < num_elements; i++) {
        new (&array[i]) T(m_vector.array[i]);
    }

    return *this;
}

// Add a new element at the location pointed by the iterator.
template <typename T>
typename vector_t<T>::iterator vector_t<T>::insert(vector_t<T>::iterator m_it, const T &m_data) {
    // Calculate the index from the iterator.
    // Note: iterator_t is a wrapper around a pointer.
    // Assuming m_it corresponds to a pointer inside our array range.
    size_t index = 0;
    // We need to manually calculate distance since we don't have direct pointer access via public API easily,
    // but looking at iterator.h (implied), it wraps T*.
    // Let's assume standard pointer arithmetic works on iterators or we use begin().
    // Since we are inside vector_t, we can't access m_it private members directly unless friend.
    // However, usually `m_it - begin()` works if operator- is defined.
    // Given the skeleton, we assume standard iterator behavior.
    
    // Workaround: Iterate to find index if operator- is missing, 
    // but typically for vector, `it - begin()` is standard.
    // Let's assume iterator_t supports pointer subtraction or we have access.
    // Looking at main.cc, it does `it++`.
    
    // Simple approach: Iterate from begin to find offset.
    iterator it = begin();
    while (it != m_it && it != end()) {
        it++;
        index++;
    }
    
    // 1. Check capacity.
    if (num_elements == array_size) {
        reserve(array_size == 0 ? 1 : array_size * 2);
    }

    // 2. Shift elements to the right (from back to index).
    // We must construct new elements using placement new (copy/move).
    for (size_t i = num_elements; i > index; i--) {
        new (&array[i]) T(array[i-1]); // Copy construct to new slot
        array[i-1].~T();               // Destruct old slot
    }

    // 3. Insert new element.
    new (&array[index]) T(m_data);
    num_elements++;

    // 4. Return iterator to the inserted element.
    return iterator(array + index);
}

// Erase an element at the location pointed by the iterator.
template <typename T>
typename vector_t<T>::iterator vector_t<T>::erase(vector_t<T>::iterator m_it) {
    // Calculate index.
    iterator it = begin();
    size_t index = 0;
    while (it != m_it && it != end()) {
        it++;
        index++;
    }

    // 1. Destruct the element at index.
    array[index].~T();

    // 2. Shift elements to the left (fill the gap).
    for (size_t i = index; i < num_elements - 1; i++) {
        new (&array[i]) T(array[i+1]); // Copy next into current
        array[i+1].~T();               // Destruct next
    }

    // 3. Decrement size.
    num_elements--;

    // 4. Return iterator to the element following the erased one (now at index).
    return iterator(array + index);
}

/*********************
 * End of Assignment *
 *********************/

#endif