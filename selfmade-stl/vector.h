#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>
#include <iterator>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
            : buffer_(Allocate(capacity))
            , capacity_(capacity) {
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& rhs) = delete;
    RawMemory(RawMemory&& other) noexcept
            : buffer_(other.buffer_)
            , capacity_(other.capacity_) {
        other.buffer_ = nullptr;
        other.capacity_ = 0;
    }
    RawMemory& operator=(RawMemory&& rhs) noexcept {
        if (this != &rhs) {
            Deallocate(buffer_);
            buffer_ = rhs.buffer_;
            capacity_ = rhs.capacity_;
            rhs.buffer_ = nullptr;
            rhs.capacity_ = 0;
        }
        return *this;
    }

    T* operator+(size_t offset) noexcept {
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;

    explicit Vector(size_t size)
            : data_(size)
            , size_(size)  //
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector& other)
            : data_(other.size_)
            , size_(other.size_)  //
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    }

    Vector(Vector&& other) noexcept {
        Swap(other);
    }

    Vector& operator=(const Vector& rhs) {
        if (this == &rhs) {
            return *this;
        }

        if (rhs.size_ > data_.Capacity()) {
            Vector rhs_copy(rhs);
            Swap(rhs_copy);
        } else {
            if (size_ > rhs.size_) {
                std::copy_n(rhs.data_.GetAddress(), rhs.size_, data_.GetAddress());
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
            } else {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i] = rhs.data_[i];
                }
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
            }
            size_ = rhs.size_;
        }
        return *this;
    }
    Vector& operator=(Vector&& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        data_.Swap(rhs.data_);
        std::swap(size_, rhs.size_);
        rhs.size_ = 0;
        return *this;
    }

    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }

        RawMemory<T> new_data(new_capacity);

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        } else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }

        data_.Swap(new_data);
        std::destroy_n(new_data.GetAddress(), new_data.Capacity());
    }
    void Resize(size_t new_size) {
        if (new_size == size_) {
            return;
        }
        if (new_size > size_) {
            if (new_size > Capacity()) {
                Reserve(new_size);
            }
            std::uninitialized_value_construct_n(data_ + size_, new_size - size_);
        } else {
            std::destroy_n(data_ + new_size, size_ - new_size);
        }
        size_ = new_size;
    }

    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        if (size_ == data_.Capacity()) {
            size_t new_capacity = size_ == 0 ? 1 : size_ * 2;
            RawMemory<T> new_data(new_capacity);
            try {
                new (new_data + size_) T(std::forward<Args>(args)...);
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
                } else {
                    std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
                }
            } catch (...) {
                new_data.GetAddress()[size_].~T();
                throw;
            }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        } else {
            new (data_ + size_) T(std::forward<Args>(args)...);
        }
        ++size_;
        return data_[size_ - 1];
    }
    void PushBack(const T& value) {
        EmplaceBack(value);
    }
    void PushBack(T&& value) {
        EmplaceBack(std::move(value));
    }
    void PopBack() noexcept {
        data_.GetAddress()[size_ - 1].~T();
        --size_;
    }

    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        const auto index = pos - begin();
        if (size_ == data_.Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            try {
                new (new_data.GetAddress() + index) T(std::forward<Args>(args)...);
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    std::uninitialized_move_n(data_.GetAddress(), index, new_data.GetAddress());
                    std::uninitialized_move_n(data_.GetAddress() + index, size_ - index, new_data.GetAddress() + index + 1);
                } else {
                    std::uninitialized_copy_n(data_.GetAddress(), index, new_data.GetAddress());
                    std::uninitialized_copy_n(data_.GetAddress() + index, size_ - index, new_data.GetAddress() + index + 1);
                }
            } catch (...) {
                new_data.GetAddress()[index].~T();
                throw;
            }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        } else {
            if (static_cast<size_t>(index) != size_) {
                T temp_value(std::forward<Args>(args)...);
                new (data_.GetAddress() + size_) T(std::move(data_[size_ - 1]));
                std::move_backward(data_.GetAddress() + index, data_.GetAddress() + size_ - 1, data_.GetAddress() + size_);
                data_[index] = std::move(temp_value);
            } else {
                new (data_.GetAddress() + index) T(std::forward<Args>(args)...);
            }
        }
        ++size_;
        return begin() + index;
    }
    iterator Insert(const_iterator pos, const T& value) {
            return Emplace(pos, value);
    }
    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::move(value));
    }
    iterator Erase(const_iterator pos) {
        const auto index = pos - data_.GetAddress();
        std::destroy_at(data_ + index);
        std::move(data_ + index + 1, data_ + size_, data_ + index);
        --size_;
        return data_ + index;
    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        return data_[index];
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

    iterator begin() noexcept { return data_.GetAddress(); }
    const_iterator begin() const noexcept { return data_.GetAddress(); }
    const_iterator cbegin() const noexcept { return data_.GetAddress(); }

    iterator end() noexcept { return data_.GetAddress() + size_; }
    const_iterator end() const noexcept { return data_.GetAddress() + size_; }
    const_iterator cend() const noexcept { return data_.GetAddress() + size_; }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};