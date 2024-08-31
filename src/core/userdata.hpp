#ifndef USERDATA_HPP
#define USERDATA_HPP

#include "userdata_types.hpp"
#include "object.hpp"

namespace gdrblx {

class LuaUserdata : protected internal::LuaUserdataBase {
public:
    virtual ~LuaUserdata() {}
};

class LuaUserdataIndex : virtual public LuaUserdata {
    static int L_get(lua_State *L);
protected:
    LuaUserdataIndex() {
        this->userdata_flags |= UD_INDEX;
    }
    virtual LuaObject lua_get(lua_State *L, LuaObject p_key) const = 0;
};

class LuaUserdataSetIndex : virtual public LuaUserdata {
    static int L_set(lua_State *L);
protected:
    LuaUserdataSetIndex() {
        this->userdata_flags |= UD_INDEXSET;
    }
    virtual void lua_set(lua_State *L, LuaObject p_key, LuaObject p_value) = 0;
};

class LuaUserdataToString : virtual public LuaUserdata {
    static int L_tostring(lua_State *L);
protected:
    LuaUserdataToString() {
        this->userdata_flags |= UD_STRING;
    }
    virtual operator LuaString() const = 0;
};

class LuaUserdataMath : virtual public LuaUserdata {
    static int L_add(lua_State *L);
    static int L_sub(lua_State *L);
    static int L_mul(lua_State *L);
    static int L_div(lua_State *L);
    static int L_idiv(lua_State *L);
    static int L_mod(lua_State *L);
    static int L_pow(lua_State *L);
    static int L_unm(lua_State *L);
protected:
    LuaUserdataMath() {
        this->userdata_flags |= UD_MATH;
    }
    virtual LuaObject lua_add(LuaObject p_other) const = 0;
    virtual LuaObject lua_sub(LuaObject p_other) const = 0;
    virtual LuaObject lua_mul(LuaObject p_other) const = 0;
    virtual LuaObject lua_div(LuaObject p_other) const = 0;
    virtual LuaObject lua_idiv(LuaObject p_other) const = 0;
    virtual LuaObject lua_mod(LuaObject p_other) const = 0;
    virtual LuaObject lua_pow(LuaObject p_other) const = 0;
    virtual LuaObject lua_unm() const = 0;
};

class LuaUserdataCall : virtual public LuaUserdata {
    static int L_call(lua_State *L);
protected:
    LuaUserdataCall() {
        this->userdata_flags |= UD_CALL;
    }
    virtual void call(lua_State *L) = 0;
};

namespace internal {
    template <class T>
    concept LuaUserdataInitable = requires(LuauState *s) { T::lua_init(s); };
}

template <class T>
class LuaUserdataInit : virtual public LuaUserdata {
protected:
    LuaUserdataInit() {
        static_assert(::std::is_base_of<LuaUserdataInit, T>::value);
        static_assert(internal::LuaUserdataInitable<T>);
        this->userdata_flags |= UD_INIT;
    }
};

class LuaUserdataLength : virtual public LuaUserdata {
    static int L_len(lua_State *L);
protected:
    LuaUserdataLength() {
        this->userdata_flags |= UD_LENGTH;
    }
    virtual LuaObject len(lua_State *L) = 0;
};

namespace internal {

template <class T>
concept UserdataClass = std::is_base_of<LuaUserdata, T>::value;

class LuaUserdataInternalInitializer {
public:
    virtual void initialize(LuauState* p_State) const = 0;
};
inline static Vec<LuaUserdataInternalInitializer*> userdata_initializers;

template <class T>
class LuaUserdataType {
public:
    static constexpr size_t ud_type = 0;
};

template <UserdataClass T>
class LuaUserdataTable final : public LuaUserdataInternalInitializer {
public:
    LuaUserdataTable() {
        userdata_initializers.push_back((LuaUserdataInternalInitializer*)this);
    }
    void initialize(LuauState* p_State) const override;
};

} // namespace internal

#define USERDATA_INITIALIZER(p_class_name, p_ud_type)                           \
    namespace internal {                                                        \
    inline static LuaUserdataTable<p_class_name> _initializer_##p_class_name ;  \
    template <>                                                                 \
    class LuaUserdataType<p_class_name> {                                       \
    public:                                                                     \
        static constexpr size_t ud_type = p_ud_type;                            \
    };                                                                          \
    } // namespace internal
#define GET_USERDATA_TYPE(p_class_name) (::gdrblx::internal::LuaUserdataType<p_class_name>::ud_type)

}

#endif