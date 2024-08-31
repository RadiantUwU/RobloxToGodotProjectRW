#ifndef OPTION_HPP
#define OPTION_HPP

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
    constexpr Option() : exists(false) {}
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

    T& get_object() {
        CRASH_COND(!exists);
        return object;
    }
};

} // namespace gdrblx

#endif