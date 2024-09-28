#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <type_traits>

#include <core/macros.hpp>

namespace gdrblx {

template <typename T>
class Property {
protected:
    using Type = T;
    void* self;
    Property(void *p_self) : self(p_self) {}
    Property(const Property& p_other) : self(p_other.self) {}

    virtual void set_func(const Type& p_other) = 0;
    virtual Type get_func() const = 0;
public:
    GDRBLX_INLINE Property& operator=(const Type& p_other) {
        set_func(p_other);
        return *this;
    }
    GDRBLX_INLINE operator Type() const {
        return get_func();
    }
    GDRBLX_INLINE bool operator==(const Type& p_other) const {
        return get_func() == p_other;
    }
    GDRBLX_INLINE bool operator!=(const Type& p_other) const {
        return get_func() != p_other;
    }
    GDRBLX_INLINE auto operator->() const requires requires (Type o) {o.operator->();}{
        return get_func();
    }
    GDRBLX_INLINE auto operator*() const requires requires (Type o) {*o;} {
        return *get_func();
    }

    GDRBLX_INLINE auto operator+(const Type& p_other) const requires requires (Type o) {o+o;} {
        return get_func()+p_other;
    }
    GDRBLX_INLINE auto operator-(const Type& p_other) const requires requires (Type o) {o-o;} {
        return get_func()-p_other;
    }
    
};
template <typename T> requires std::is_reference_v<T>
class Property<T> {
protected:
    using Type = std::remove_reference_t<T>;

    void* self;
    Property(void *p_self) : self(p_self) {}
    Property(const Property& p_other) : self(p_other.self) {}

    virtual void set_func(const Type& p_other) = 0;
    virtual Type& get_func() const = 0;
public:
    GDRBLX_INLINE Property& operator=(const Type& p_other) {
        set_func(p_other);
        return *this;
    }
    GDRBLX_INLINE operator Type() const {
        return get_func();
    }
    GDRBLX_INLINE operator Type&() const {
        return get_func();
    }
    GDRBLX_INLINE bool operator==(const Type& p_other) const {
        return get_func() == p_other;
    }
    GDRBLX_INLINE bool operator!=(const Type& p_other) const {
        return get_func() != p_other;
    }
    GDRBLX_INLINE auto operator->() const requires requires (Type o) {o.operator->();}{
        return get_func();
    }
    GDRBLX_INLINE auto operator*() const requires requires (Type o) {*o;} {
        return *get_func();
    }
    GDRBLX_INLINE const Type* operator&() const {
        return &get_func();
    }

    GDRBLX_INLINE auto operator+(const Type& p_other) const requires requires (Type o) {o+o;} {
        return get_func()+p_other;
    }
    GDRBLX_INLINE auto operator-(const Type& p_other) const requires requires (Type o) {o-o;} {
        return get_func()-p_other;
    }
    
};

#define PROPERTY(p_type, p_name, p_class_name, p_get_f, p_set_f)                                                         \
    private:                                                                                                             \
        friend class __PROPERTY_##p_name;                                                                                \
        class __PROPERTY_##p_name : public Property<p_type> {                                                            \
            friend class p_class_name;                                                                                   \
            __PROPERTY_##p_name (p_class_name* p_s) : Property(p_s) {}                                                   \
        protected:                                                                                                       \
            GDRBLX_INLINE void set_func(const Type& p_other) override {((p_class_name*)this->self)->p_set_f(p_other);}   \
            GDRBLX_INLINE p_type get_func() const override {return ((p_class_name*)this->self)->p_get_f();}              \
            GDRBLX_INLINE Property& operator=(const Type& p_other) {                                                     \
                set_func(p_other);                                                                                       \
                return *this;                                                                                            \
            }                                                                                                            \
        };                                                                                                               \
    public:                                                                                                              \
        __PROPERTY_##p_name p_name = this;
#define PROPERTY_READONLY(p_type, p_name, p_class_name, p_get_f)                                             \
    private:                                                                                                 \
        friend class __PROPERTY_##p_name;                                                                    \
        class __PROPERTY_##p_name : public Property<p_type> {                                                \
            friend class p_class_name;                                                                       \
            __PROPERTY_##p_name (p_class_name* p_s) : Property(p_s) {}                                       \
        protected:                                                                                           \
            GDRBLX_INLINE void set_func(const Type& p_other) override {}                                     \
            GDRBLX_INLINE p_type get_func() const override {return ((p_class_name*)this->self)->p_get_f();}  \
        };                                                                                                   \
    public:                                                                                                  \
        const __PROPERTY_##p_name p_name = this;

#define PROPERTY_READONLY_PROXY(p_type, p_name, p_class_name, p_private_name)                    \
    private:                                                                                     \
        GDRBLX_INLINE const p_type& __PROPERTY_##p_name##_GET () const {return p_private_name;}  \
        PROPERTY_READONLY(const p_type&, p_name, p_class_name, __PROPERTY_##p_name##_GET)         
        

}; // namespace gdrblx

#endif