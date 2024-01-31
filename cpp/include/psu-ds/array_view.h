// A wrapper around an array similar to std::string_view
// This allows you to use standard library methods (begin(), end(), size()...) on a
// portion of an array. Modifying this array will modify the original array
//
// this does not handle memory management in any way, and will not copy any data. It is up to the user to
// ensure that the array does not fall out of scope while this object is in scope
//
// Created by Max Norfolk on 1/31/24.

#pragma once
#include <cassert>
#include <utility>

namespace psudb {
    template<typename T>
    class ArrayView {
    public:
        typedef T *iterator;

        ArrayView() : arr(nullptr), size_(0) {}

        ArrayView(T *start, size_t size) : arr(start), size_(size) {}

        [[nodiscard]] inline T &operator[](size_t index) { return arr[index]; }

        [[nodiscard]] inline const T &operator[](size_t index) const { return arr[index]; }

        // at will check at runtime that the index is in bounds! it does this by an assertion check
        [[nodiscard]] inline T &at(size_t index) {
            assert(index >= 0 && index < size_ && "Index out of bounds!");
            return arr[index];
        }

        [[nodiscard]] inline const T &at(size_t index) const {
            assert(index >= 0 && index < size_ && "Index out of bounds!");
            return arr[index];
        }

        [[nodiscard]] inline T &front() { return arr[0]; }

        [[nodiscard]] inline T &back() { return arr[size_ - 1]; }

        [[nodiscard]] inline const T &front() const { return arr[0]; }

        [[nodiscard]] inline const T &back() const { return arr[size_ - 1]; }

        [[nodiscard]] inline T *data() { return arr; }

        [[nodiscard]] inline const T *data() const { return arr; }

        [[nodiscard]] inline bool empty() const { return size_ == 0; }

        [[nodiscard]] inline size_t size() const { return size_; }

        [[nodiscard]] inline size_t max_size() const { return size_; }

        [[nodiscard]] inline iterator begin() const { return arr; }

        [[nodiscard]] inline iterator end() const { return arr + size_; }

        void fill(const T &val) {
            for (int i = 0; i < size_; ++i)
                arr[i] = val;
        }

        void swap(ArrayView<T> &swap_with) noexcept {
            std::swap(arr, swap_with.arr);
            std::swap(size_, swap_with.size_);
        }

    private:
        T *arr;
        size_t size_{};
    };
}
