#ifndef OPTION_HPP
#define OPTION_HPP

#include <cstddef>
#include <godot_cpp/core/error_macros.hpp>

#include <utility>

namespace gdrblx {

template <typename T>
class Option {
    union {
        T object;
        bool b = false;
    };
public:
    constexpr Option() : exists(true), object() {}
    constexpr Option(std::nullptr_t) : exists(false) {}
    template <typename... Args>
    constexpr Option(Args... p_args) : exists(true), object(p_args...) {}
    
    constexpr Option(const Option& p_other) : exists(p_other.exists) {
        if (p_other.exists)
            new (&object) T(p_other.object);
    }
    constexpr Option(Option&& p_other) : exists(p_other.exists) {
        if (p_other.exists)
            new (&object) T(std::move(p_other.object));
    }
    ~Option() {
        if (exists)
            object.~T();
    }

    const bool exists;

    T& unwrap() {
        CRASH_COND(!exists);
        return object;
    }
    const T& unwrap() const {
        CRASH_COND(!exists);
        return object;
    }
    inline explicit operator bool() {
        return exists;
    }

    Option& operator=(const Option& p_other) {
        this->~Option();
        new (this) Option(p_other);
        return *this;
    }
    Option& operator=(T&& p_other) {
        this->~Option();
        new (this) Option(p_other);
        return *this;
    }
    Option& operator=(const T& p_other) {
        this->~Option();
        new (this) Option(p_other);
        return *this;
    }
    Option& operator=(Option&& p_other) {
        this->~Option();
        new (this) Option(p_other);
        return *this;
    }

    bool operator==(const Option<T>& p_other) const {
        if (p_other.exists == exists && !exists) return true;
        if (p_other.exists != exists) return false;
        return unwrap()==p_other.unwrap();
    }
    bool operator==(const T& p_other) const {
        if (!exists) return false;
        return unwrap()==p_other;
    }
    bool operator!=(const Option<T>& p_other) const {
        if (p_other.exists == exists && !exists) return false;
        if (p_other.exists != exists) return true;
        return unwrap()!=p_other.unwrap();
    }
    bool operator!=(const T& p_other) const {
        if (!exists) return true;
        return unwrap()!=p_other;
    }
};

} // namespace gdrblx

#endif