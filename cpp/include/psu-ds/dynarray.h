// A wrapper arounds arrays that allow a dynamic size, and supports the typical std library things you would like to use
//
// Created by Max Norfolk on 12/21/23.


#pragma once

#include <cstddef>
#include <cassert>

namespace psudb {
    template<typename T>
    class DynamicArray {
    public:
        typedef T *iterator;

        DynamicArray() : arr(nullptr), size_(0) {}

        explicit DynamicArray(const size_t &size) : arr(new T[size]), size_(size) {}

        virtual ~DynamicArray() {
            delete arr;
        }

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

        [[nodiscard]]  inline T &front() { return arr[0]; }

        [[nodiscard]] inline T &back() { return arr[size_ - 1]; }

        [[nodiscard]] const inline T &front() const { return arr[0]; }

        [[nodiscard]] const inline T &back() const { return arr[size_ - 1]; }

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

    private:
        T *arr;
        size_t size_;
    };

}