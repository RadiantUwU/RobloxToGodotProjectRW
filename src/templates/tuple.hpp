#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <cstddef>

namespace gdrblx {

template <typename T = void, typename... Args>
class tuple {
    const T o;
    const tuple<Args...> p_next;
public:
    static constexpr size_t size = 1+sizeof...(Args);

    constexpr tuple(const tuple<T, Args...>& p_other) : o(p_other.o), p_next(p_other.p_next) {}

    constexpr tuple() : o() {}
    constexpr tuple(T p_o) : o(p_o) {}
    constexpr tuple(T p_o, Args... p_args) : o(p_o), p_next(p_args...) {}

    template <size_t idx, typename RetT>
    constexpr RetT get() {
        return p_next.template get<idx-1>();
    }
    template <>
    constexpr T get<0, T>() {
        return o;
    }

    constexpr bool operator==(const tuple<T, Args...>& p_other) const {
        return o == p_other.o && p_next == p_other.p_next;
    }
    constexpr bool operator!=(const tuple<T, Args...>& p_other) const {
        return o != p_other.o || p_next != p_other.p_next;
    }

    constexpr tuple<T, Args...>& operator=(const tuple<T, Args...>& p_other) {
        this->~tuple();
        new (this) tuple(p_other);
        return *this;
    }
};

template <>
class tuple<> {
public:
    static constexpr size_t size = 0;   
    template <size_t idx>
    constexpr void get() {
        static_assert(false, "cannot index out of bounds");
    }

    constexpr bool operator==(const tuple<>& p_other) const {
        return true;
    }
    constexpr bool operator!=(const tuple<>& p_other) const {
        return false;
    }


};

template <typename T>
class tuple<T> {
    const T o;
public:
    static constexpr size_t size = 1;

    constexpr tuple(const tuple<T>& p_other) : o(p_other.o) {}

    constexpr tuple() : o() {}
    constexpr tuple(T p_o) : o(p_o) {}

    template <size_t idx>
    constexpr T get() {
        static_assert(idx == 0, "cannot index out of bounds.");
        return o;
    }

    constexpr bool operator==(const tuple<T>& p_other) const {
        return o == p_other.o;
    }
    constexpr bool operator!=(const tuple<T>& p_other) const {
        return o != p_other.o;
    }

    constexpr tuple<T>& operator=(const tuple<T>& p_other) {
        this->~tuple();
        new (this) tuple(p_other);
        return *this;
    }
};


} // namespace gdrblx
#endif