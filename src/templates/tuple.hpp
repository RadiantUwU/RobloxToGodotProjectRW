#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <cstddef>
#include <utility>

namespace gdrblx {

template <typename T = void, typename... Args>
class Tuple {
    const T o;
    const Tuple<Args...> p_next;
public:
    static constexpr size_t size = 1+sizeof...(Args);

    constexpr Tuple(const Tuple<T, Args...>& p_other) : o(p_other.o), p_next(p_other.p_next) {}

    constexpr Tuple() : o() {}
    constexpr Tuple(T p_o) : o(p_o) {}
    constexpr Tuple(T p_o, Args... p_args) : o(p_o), p_next(p_args...) {}

    template <size_t idx, typename RetT>
    constexpr RetT& get() & {
        return p_next.template get<idx-1>();
    }
    template <>
    constexpr T& get<0, T>() & {
        return o;
    }
    template <size_t idx, typename RetT>
    constexpr const RetT& get() const & {
        return p_next.template get<idx-1>();
    }
    template <>
    constexpr const T& get<0, T>() const & {
        return o;
    }
    template <size_t idx, typename RetT>
    constexpr RetT&& get() && {
        return std::move(p_next.template get<idx-1>());
    }
    template <>
    constexpr T&& get<0, T>() && {
        return std::move(o);
    }

    constexpr bool operator==(const Tuple<T, Args...>& p_other) const {
        return o == p_other.o && p_next == p_other.p_next;
    }
    constexpr bool operator!=(const Tuple<T, Args...>& p_other) const {
        return o != p_other.o || p_next != p_other.p_next;
    }

    constexpr Tuple<T, Args...>& operator=(const Tuple<T, Args...>& p_other) {
        this->~Tuple();
        new (this) Tuple(p_other);
        return *this;
    }
};

template <>
class Tuple<> {
public:
    static constexpr size_t size = 0;   
    template <size_t idx>
    constexpr void get() {
        static_assert(false, "cannot index out of bounds");
    }

    constexpr bool operator==(const Tuple<>& p_other) const {
        return true;
    }
    constexpr bool operator!=(const Tuple<>& p_other) const {
        return false;
    }


};

template <typename T>
class Tuple<T> {
    const T o;
public:
    static constexpr size_t size = 1;

    constexpr Tuple(const Tuple<T>& p_other) : o(p_other.o) {}

    constexpr Tuple() : o() {}
    constexpr Tuple(T p_o) : o(p_o) {}

    template <size_t idx>
    constexpr T&& get() && {
        static_assert(idx == 0, "cannot index out of bounds.");
        return std::move(o);
    }
    template <size_t idx>
    constexpr T& get() & {
        static_assert(idx == 0, "cannot index out of bounds.");
        return o;
    }
    template <size_t idx>
    constexpr const T& get() const & {
        static_assert(idx == 0, "cannot index out of bounds.");
        return o;
    }

    constexpr bool operator==(const Tuple<T>& p_other) const {
        return o == p_other.o;
    }
    constexpr bool operator!=(const Tuple<T>& p_other) const {
        return o != p_other.o;
    }

    constexpr Tuple<T>& operator=(const Tuple<T>& p_other) {
        this->~Tuple();
        new (this) Tuple(p_other);
        return *this;
    }
};


} // namespace gdrblx
#endif