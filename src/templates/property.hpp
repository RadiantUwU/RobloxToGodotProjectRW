#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <type_traits>

#include <core/macros.hpp>

namespace gdrblx {

#define PROPERTY(p_type, p_name, p_class_name, p_get_f, p_set_f)               \
    private:                                                                   \
        friend class __PROPERTY_##p_name;                                      \
        class __PROPERTY_##p_name {                                            \
            friend class p_class_name;                                         \
            __PROPERTY_##p_name (p_class_name* p_s) : s(p_s) {}                \
            __PROPERTY_##p_name (const __PROPERTY_##p_name& p_o) : s(p_o.s) {} \
            p_class_name *s;                                                   \
        public:                                                                \
            GDRBLX_INLINE p_type operator=(const p_type& p_other) {            \
                s->p_set_f(p_other);                                           \
                return *this;                                                  \
            }                                                                  \
            GDRBLX_INLINE operator p_type () const {                           \
                return s->p_get_f();                                           \
            }                                                                  \
        };                                                                     \
    public:                                                                    \
        __PROPERTY_##p_name p_name = this;
#define PROPERTY_PTR(p_type, p_name, p_class_name, p_get_f, p_set_f)           \
    private:                                                                   \
        friend class __PROPERTY_##p_name;                                      \
        class __PROPERTY_##p_name {                                            \
            friend class p_class_name;                                         \
            __PROPERTY_##p_name (p_class_name* p_s) : s(p_s) {}                \
            __PROPERTY_##p_name (const __PROPERTY_##p_name& p_o) : s(p_o.s) {} \
            p_class_name *const s;                                             \
        public:                                                                \
            GDRBLX_INLINE p_type operator=(p_type p_other) {                   \
                s->p_set_f(p_other);                                           \
                return *this;                                                  \
            }                                                                  \
            GDRBLX_INLINE operator p_type () const {                           \
                return s->p_get_f();                                           \
            }                                                                  \
        };                                                                     \
    public:                                                                    \
        __PROPERTY_##p_name p_name = this;

#define PROPERTY_READONLY(p_type, p_name, p_class_name, p_get_f)               \
    private:                                                                   \
        friend class __PROPERTY_##p_name;                                      \
        class __PROPERTY_##p_name {                                            \
            friend class p_class_name;                                         \
            __PROPERTY_##p_name (const p_class_name* p_s) : s(p_s) {}          \
            __PROPERTY_##p_name (const __PROPERTY_##p_name& p_o) : s(p_o.s) {} \
            const p_class_name *const s;                                       \
        public:                                                                \
            GDRBLX_INLINE operator p_type () const {                           \
                return s->p_get_f();                                           \
            }                                                                  \
        };                                                                     \
    public:                                                                    \
        __PROPERTY_##p_name p_name = this;

#define PROPERTY_READONLY_PROXY(p_type, p_name, p_class_name, p_private_name)             \
    private:                                                                              \
        GDRBLX_INLINE p_type __PROPERTY_##p_name##_GET () const {return p_private_name;}  \
        PROPERTY_READONLY(p_type, p_name, p_class_name, __PROPERTY_##p_name##_GET)        \
        

}; // namespace gdrblx

#endif