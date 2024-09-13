#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value) {
        new(data_) T(value);
        is_initialized_ = true;
    }

    Optional(T&& value) {
        new(data_) T(std::move(value));
        is_initialized_ = true;
    }

    Optional(const Optional& other) {
        if (other.is_initialized_) {
            new(data_) T(*other);
            is_initialized_ = true;
        }
    }

    Optional(Optional&& other) {
        if (other.is_initialized_) {
            new(data_) T(std::move(*other));
            is_initialized_ = true;
        }
    }

    Optional& operator=(const T& value) {
        if (is_initialized_) {
            **this = value;
        } else {
            new(data_) T(value);
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (is_initialized_) {
            **this = std::move(rhs);
        } else {
            new(data_) T(std::move(rhs));
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            **this = *rhs;
        } else if (is_initialized_ && !rhs.is_initialized_) {
            Reset();
        } else if (!is_initialized_ && rhs.is_initialized_) {
            new(data_) T(*rhs);
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            **this = std::move(*rhs);
//            rhs.Reset();
        } else if (is_initialized_ && !rhs.is_initialized_) {
            Reset();
        } else if (!is_initialized_ && rhs.is_initialized_) {
            new(data_) T(std::move(*rhs));
            is_initialized_ = true;
//            rhs.Reset();
        }
        return *this;
    }

    ~Optional() {
        Reset();
    }

    template <typename... Ts>
    void Emplace(Ts&&... vs) {
        if (is_initialized_) {
            (**this).~T();
        }
        new(data_) T(std::forward<Ts>(vs)...);
        is_initialized_ = true;
    }

    bool HasValue() const {
        return is_initialized_;
    }

    T& operator*() & {
        return *reinterpret_cast<T*>(data_);
    }
    T&& operator*() && {
        return std::move(*reinterpret_cast<T*>(data_));
    }

    const T& operator*() const& {
        return *reinterpret_cast<const T*>(data_);
    }
    T* operator->() {
        return reinterpret_cast<T*>(data_);
    }
    const T* operator->() const {
        return reinterpret_cast<const T*>(data_);
    }

    T& Value() & {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return **this;
    }
    T&& Value() && {
        return std::move(**this);
    }
    const T& Value() const& {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return **this;
    }

    void Reset() {
        if (is_initialized_) {
            (**this).~T();
            is_initialized_ = false;
        }
    }

private:
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};