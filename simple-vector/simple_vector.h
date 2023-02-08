#pragma once
#include <array>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj() = default;
    explicit ReserveProxyObj(size_t capacity_to_reserve) {
        capacity_ = capacity_to_reserve;
    }
    size_t getCapacity() const {
        return capacity_;
    }
private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : SimpleVector(size, std::move(Type{})) {}

    SimpleVector(size_t size, const Type& value) :
            items_(size),
            size_(size),
            capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) :
            items_(init.size()),
            size_(init.size()),
            capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }


    SimpleVector(const SimpleVector& other) :
            items_(other.size_),
            size_(other.size_),
            capacity_(other.capacity_) {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)  noexcept {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        std::swap(items_, other.items_);
    }

    explicit SimpleVector(const ReserveProxyObj& proxyObj) {
                 Reserve(proxyObj.getCapacity());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (rhs.items_.Get() != this->items_.Get()) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
    }
        return *this;
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_++] = item;
        } else {
            SimpleVector<Type> tmp(std::max(size_ + 1, capacity_ * 2));
            std::copy(begin(), end(), tmp.begin());
            tmp[size_] = item;
            swap(tmp);
            size_ = ++tmp.size_;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_++] = std::move(item);
        } else {
            ArrayPtr<Type> tmp(std::max(size_ + 1, capacity_ * 2));
            capacity_ = std::max(size_ + 1, capacity_ * 2);
            std::move(begin(), end(), &tmp[0]);
            tmp[size_++] = std::move(item);
            items_.swap(tmp);
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(begin() <= pos && pos <= end());
        if (size_ < capacity_) {
            std::copy_backward(const_cast<Iterator>(pos),end(), end() + 1);
            items_[pos-begin()] = value;
            ++size_;
            return pos == nullptr ? begin() : const_cast<Iterator>(pos);
        } else {
            size_t shift = pos-begin();
            SimpleVector<Type> tmp(std::max(size_ + 1, capacity_ * 2));
            tmp.size_ = size_;
            std::copy(begin(), end(), tmp.begin());
            std::copy_backward(const_cast<Iterator>(pos),end(), tmp.end() + 1);
            tmp.items_[shift] = value;
            swap(tmp);
            ++size_;
            return begin() + shift;
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(begin() <= pos && pos <= end());
        if (size_ < capacity_) {
            std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)),std::make_move_iterator(end()), end() + 1);
            items_[pos - begin()] = std::move(value);
            ++size_;
            return pos == nullptr ? begin() : const_cast<Iterator>(pos);
        } else {
            size_t shift = pos - begin();
            ArrayPtr<Type> tmp(std::max(size_ + 1, capacity_ * 2));
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), &tmp[0]);
            std::copy_backward(std::make_move_iterator(const_cast<Iterator>(pos)),std::make_move_iterator(end()), &tmp[size_] + 1);
            tmp[shift] = std::move(value);
            items_.swap(tmp);
            capacity_ = std::max(size_ + 1, capacity_ * 2);
            ++size_;
            return begin() + shift;
        }
    }

    void PopBack() noexcept {
        assert(size_ != 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        assert(size_ != 0);
        size_t shift = pos - begin();
        std::move(std::make_move_iterator(const_cast<Iterator>(pos) + 1),std::make_move_iterator(end()), const_cast<Iterator>(pos));
        --size_;
        return begin() + shift;
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    Type& At(size_t index) {
        return index >= size_ ? throw std::out_of_range("out of range") : items_[index];
    }

    const Type& At(size_t index) const {
        return index >= size_ ? throw std::out_of_range("out of range") : items_[index];
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity < capacity_) {
            return;
        }
        SimpleVector<Type> tmp(new_capacity);
        std::copy(begin(), end(), tmp.begin());
        swap(tmp);
        size_ = tmp.size_;
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else if (new_size > capacity_) {
            ArrayPtr<Type> tmp(std::max(new_size, capacity_ * 2));
            std::move(begin(), end(), &tmp[0]);
            size_ = new_size;
            capacity_ = capacity_ * 2 > new_size ? capacity_ * 2 : new_size;
            items_.swap(tmp);
        } else if (new_size < capacity_) {
            size_t distance = new_size - size_;
            for (auto it = end(); it != end() + distance; ++it) {
                *it = std::move(Type{});
            }
            size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs == lhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs || lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs || lhs == rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
