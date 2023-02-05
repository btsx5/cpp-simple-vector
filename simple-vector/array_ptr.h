#include <cstdlib>

template <typename Type>
class ArrayPtr {
public:

    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) {
        Type* new_array = new Type[size]();
        raw_ptr_ = size == 0 ? nullptr : new_array;
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr&) = delete;

    ArrayPtr(ArrayPtr&& other) {
         raw_ptr_ = std::move(other.raw_ptr_);
    }

    ~ArrayPtr() {
        delete []raw_ptr_;
        raw_ptr_ = nullptr;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;

    ArrayPtr& operator=(ArrayPtr&& other) {
        assert(this->raw_ptr_ != other.raw_ptr_);
        raw_ptr_ = std::move(other.raw_ptr_);
        return *this;
    }

    [[nodiscard]] Type* Release() noexcept {
        Type* raw_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return raw_ptr;
    }

    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    explicit operator bool() const {
        return raw_ptr_ == nullptr ? nullptr : raw_ptr_;
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        Type* tmp = other.raw_ptr_;
        other.raw_ptr_ = raw_ptr_;
        raw_ptr_ = tmp;
    }

private:
    Type* raw_ptr_ = nullptr;
};