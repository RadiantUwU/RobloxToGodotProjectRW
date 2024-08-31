#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <godot_cpp/core/error_macros.hpp>

#ifndef NDEBUG
#define GDRBLX_INLINE
#define GDRBLX_CONSTEXPR
#include <iostream>
namespace gdrblx {
namespace internal {
    template <typename T>
    inline void GDRBLX_PRINT_DEBUG_(T p_arg) {
        ::std::cout << p_arg;
    }
    inline void GDRBLX_PRINT_DEBUG_() {
        ::std::cout << "\n";
    }
    template <typename T, typename... Args>
    inline void GDRBLX_PRINT_DEBUG_(T p_o, Args... p_args) {
        ::std::cout << p_o;
        GDRBLX_PRINT_DEBUG_(p_args...);
    }
} // namespace internal
template <typename... Args>
inline void GDRBLX_PRINT_DEBUG(Args... p_args) {
    internal::GDRBLX_PRINT_DEBUG_(p_args...);
    ::std::cout << "\n";
}
}
#ifdef __GNUC__
#define GDRBLX_NOINLINE [[gnu::noinline]]
#else
#ifdef _MSC_VER
#define GDRBLX_NOINLINE __declspec(noinline)
#else
#ifdef __clang__
#define GDRBLX_NOINLINE [[clang::noinline]]
#endif
#endif
#endif
#else // NDEBUG
// Dont even make it a function, let the compiler completely skip over it
#define GDRBLX_PRINT_DEBUG(...)
#define GDRBLX_INLINE inline
#define GDRBLX_NOINLINE inline // inline anyway.
#define GDRBLX_CONSTEXPR constexpr
#endif

#define UNREACHABLE() CRASH_NOW()

#endif // DEBUG_HPP