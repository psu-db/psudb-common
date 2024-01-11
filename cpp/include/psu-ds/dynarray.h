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
        typedef T* iterator;

        DynamicArray() : arr(nullptr), size_(0) {}

        explicit DynamicArray(const size_t& size) : arr(new T[size]), size_(size) {}

        DynamicArray(std::initializer_list<T> init) : arr(new T[init.size()]), size_(init.size()) {
            auto it = init.begin();
            for (size_t i = 0; i < size_; ++i) {
                arr[i] = *it;
                ++it;
            }
        }

        // copy constructor
        DynamicArray(const DynamicArray<T>& orig) : arr(new T[orig.size_]), size_(orig.size_) {
            for (size_t i = 0; i < size_; ++i) {
                this->arr[i] = orig.arr[i];
            }
        }

        // copy assignment operators
        // move & copy operator based on https://codereview.stackexchange.com/a/211253
        // and https://stackoverflow.com/a/3652138
        DynamicArray<T>& operator=(const DynamicArray<T>& other) noexcept {
            DynamicArray<T> temp{other};
            swap(temp);
            return *this;
        }

        // move constructor
        DynamicArray(DynamicArray<T>&& move_me) noexcept: arr(move_me.arr), size_(move_me.size_) {
            move_me.arr = nullptr;
        }

        // move assignment operator
        DynamicArray<T>& operator=(DynamicArray<T>&& other) noexcept {
            swap(other);
            return *this;
        }

        virtual ~DynamicArray() {
            delete[] arr;
        }

        [[nodiscard]] inline T& operator[](size_t index) { return arr[index]; }

        [[nodiscard]] inline const T& operator[](size_t index) const { return arr[index]; }

        // at will check at runtime that the index is in bounds! it does this by an assertion check
        [[nodiscard]] inline T& at(size_t index) {
            assert(index >= 0 && index < size_ && "Index out of bounds!");
            return arr[index];
        }

        [[nodiscard]] inline const T& at(size_t index) const {
            assert(index >= 0 && index < size_ && "Index out of bounds!");
            return arr[index];
        }

        [[nodiscard]] inline T& front() { return arr[0]; }

        [[nodiscard]] inline T& back() { return arr[size_ - 1]; }

        [[nodiscard]] const inline T& front() const { return arr[0]; }

        [[nodiscard]] const inline T& back() const { return arr[size_ - 1]; }

        [[nodiscard]] inline T* data() { return arr; }

        [[nodiscard]] inline const T* data() const { return arr; }

        [[nodiscard]] inline bool empty() const { return size_ == 0; }

        [[nodiscard]] inline size_t size() const { return size_; }

        [[nodiscard]] inline size_t max_size() const { return size_; }

        [[nodiscard]] inline iterator begin() const { return arr; }

        [[nodiscard]] inline iterator end() const { return arr + size_; }

        void fill(const T& val) {
            for (int i = 0; i < size_; ++i)
                arr[i] = val;
        }

        void swap(DynamicArray<T>& swap_with) {
            std::swap(arr, swap_with.arr);
            std::swap(size_, swap_with.size_);
        }

    private:
        // pointer to constant
        T* arr;
        size_t size_{};
    };
}
