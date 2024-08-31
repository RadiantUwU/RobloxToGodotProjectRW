#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <utility>

#include <lua.h>
#include <lualib.h>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include <templates/rc.hpp>

#include "macros.hpp"
#include "string.hpp"
#include "userdata_types.hpp"

namespace gdrblx {

template <typename T>
using Vec = ::godot::Vector<T>;
template <typename T>
using LocalVec = ::godot::LocalVector<T>;
template <typename TKey, typename TValue,
		typename Hasher = ::godot::HashMapHasherDefault,
		typename Comparator = ::godot::HashMapComparatorDefault<TKey>,
		typename Allocator = ::godot::DefaultTypedAllocator<::godot::HashMapElement<TKey, TValue>>>
using HashMap = ::godot::HashMap<TKey, TValue, Hasher, Comparator, Allocator>;

namespace internal {

class LuaObjectHeader;

}; // gdrblx::internal

class LuauState;
class LuaTable;
class LuaBuffer;
class LuaFunction;
class LuaThread;
class SharedTable;
class LuauCtx;
class LuaUserdata;

class LuaObject final { // mark as final till required to inherit. no virtual destructor here
    friend class internal::LuaObjectHeader;
    friend class LuaThread;
    friend class LuauCtx;
    friend class LuauFnCtx;

    union {
        internal::LuaObjectHeader *header = nullptr;
        const bool boolean;
        const lua_Integer integer;
        const Arc<internal::LuaUserdataBase> userdata;
        const lua_Number number;
        const LuaString str;
        const void* light_userdata;
        struct {
            lua_State *const local_stack_coro;
            const size_t local_stack_pos;
        };
    };
public:
    enum Type {
        NIL,
        BOOLEAN,

        INTEGER,
        NUMBER,
        STRING,

        LIGHTUSERDATA,

        USERDATA,
        TABLE,
        BUFFER,
        FUNCTION,
        THREAD,
        GLOBALENV,

        LOCAL,
        REF,
    };
private:
    const Type type;

    constexpr static const char* static_get_typename(Type p_type) {
        switch (p_type) {
        case NIL:
            return "nil";
        case BOOLEAN:
            return "boolean";
        case INTEGER:
        case NUMBER:
            return "number";
        case STRING:
            return "string";
        case LIGHTUSERDATA:
            return "light userdata";
        case USERDATA:
            return "userdata";
        case TABLE:
            return "table";
        case BUFFER:
            return "buffer";
        case FUNCTION:
            return "function";
        case THREAD:
            return "thread";
        case GLOBALENV:
            return "global environment";
        default:
            return "<unknown>";
        }
    };
    GDRBLX_INLINE static const char* static_get_typename(UserdataType p_type) {
        return internal::LuaUserdataBase::get_type_to_string(p_type);
    }

    static Type get_type_from_ref(LuauState* p_state, size_t p_ref);
    static void set_ref_to_nil(LuauState* p_state, size_t p_ref);

    void push_to_stack(LuauState* p_state, lua_State* p_L) const;

    GDRBLX_INLINE LuaObject(lua_State *p_L, size_t stack_pos) : local_stack_coro(p_L), local_stack_pos(stack_pos), type(LOCAL) {}
    static LuaObject convert(lua_State *p_L, size_t stack_pos);
    LuaObject(LuauState* p_L, size_t ref_pos);
    LuaObject(Type t, internal::LuaObjectHeader *header);
public:
    Type get_type() const;
    const char *get_typename() const;

    GDRBLX_INLINE LuaObject() : type(NIL) {}
    GDRBLX_INLINE LuaObject(bool p_bool) : type(BOOLEAN), boolean(p_bool) {}
    GDRBLX_INLINE LuaObject(LuaString p_str) : type(STRING), str(p_str) {}
    GDRBLX_INLINE LuaObject(const char *p_str) : type(STRING), str(p_str) {}
    GDRBLX_INLINE LuaObject(::godot::String p_str) : type(STRING), str(p_str) {}
    GDRBLX_INLINE LuaObject(void *p_ptr) : type(LIGHTUSERDATA), light_userdata(p_ptr) {}

    GDRBLX_INLINE LuaObject(long long p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(unsigned long long p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(uint8_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(int8_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(uint16_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(int16_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(uint32_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(int32_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(uint64_t p_int) : type(INTEGER), integer(p_int) {}
    GDRBLX_INLINE LuaObject(int64_t p_int) : type(INTEGER), integer(p_int) {}
    
    GDRBLX_INLINE LuaObject(float p_num) : type(NUMBER), number(p_num) {}
    GDRBLX_INLINE LuaObject(double p_num) : type(NUMBER), number(p_num) {}

    LuaObject(const LuaFunction& p_func);
    LuaObject(const LuaThread& p_thr);
    LuaObject(LuauState *p_st);
    LuaObject(LuaBuffer p_buf);
    LuaObject(LuaTable p_tbl);
    LuaObject(SharedTable p_tbl);
    LuaObject(const LuaObject& other);

    LuaObject(Type p_type);
    
    template <class T> requires IsUserdata<T>
    LuaObject(const Arc<T>& p_userdata);
    
    ~LuaObject();

    operator long long() const;
    operator unsigned long long() const;
    operator uint8_t() const;
    operator int8_t() const;
    operator uint16_t() const;
    operator int16_t() const;
    operator uint32_t() const;
    operator int32_t() const;
    operator uint64_t() const;
    operator int64_t() const;

    operator bool() const;
    operator lua_Number() const;
    operator LuaString() const;
    operator ::godot::String() const;
    operator void*() const;
    
    template <typename T>
    Arc<T>& operator->();

    operator lua_CFunction() const;
    operator lua_Continuation() const;

    operator LuaBuffer&() const;
    operator LuaTable&() const;
    operator LuaFunction&() const;
    operator SharedTable&() const;
    operator LuaThread() const;
    
    UserdataType get_userdata_type() const;
    template <class T> requires IsUserdata<T>
    Arc<T>& as_userdata() const;

    bool is_type(Type p_t) const {
        switch (p_t) {
            case NIL:
            case BOOLEAN:
            case LIGHTUSERDATA:
            case USERDATA:
            case BUFFER:
            case FUNCTION:
            case THREAD:
            case GLOBALENV:
                return p_t == get_type();
            case STRING:
            case INTEGER:
            case NUMBER:
                return get_type() >= INTEGER or get_type() <= STRING;
            case LOCAL:
            case REF:
                DEV_ASSERT(false);
            case TABLE: {
                Type obj_type = get_type();
                return obj_type == p_t || obj_type == GLOBALENV;
            }
            
        }
    };
    bool is_type(UserdataType p_t) const {
        if (!is_type(USERDATA)) return false;
        return userdata->get_userdata_type() == p_t;
    };

    void push(lua_State *p_L);
    LuaObject as_local(lua_State *p_L) const;
    bool can_clone() const;
    LuaObject clone() const &;
    GDRBLX_INLINE LuaObject clone() const && {
        return std::move(*this);
    }; // skip over clone call if its on the stack.
    GDRBLX_INLINE bool is_stack() const {
        return type == LOCAL or type == REF;
    }
    bool can_cross_state_boundary() const;
    LuaObject clone_in(LuauState* state) const;
    bool knows_luau_state() const;
    LuauState* get_luau_state() const;

    LuaObject operator  +(const LuaObject& p_other) const;
    LuaObject operator  -(const LuaObject& p_other) const;
    LuaObject operator  *(const LuaObject& p_other) const;
    LuaObject operator  /(const LuaObject& p_other) const;
    LuaObject operator  -()                         const;
    LuaObject operator  %(const LuaObject& p_other) const;
    LuaObject pow        (const LuaObject& p_other) const;
    LuaObject idiv       (const LuaObject& p_other) const;

    LuaObject operator  &(const LuaObject& p_other) const;
    LuaObject operator  |(const LuaObject& p_other) const;
    LuaObject operator  ^(const LuaObject& p_other) const;
    LuaObject operator  ~()                         const;
    LuaObject operator <<(const LuaObject& p_other) const;
    LuaObject operator >>(const LuaObject& p_other) const;

    bool operator &&(const LuaObject& p_other) const;
    bool operator ||(const LuaObject& p_other) const;
    bool operator  !()                         const;

    LuaObject operator ==(const LuaObject& p_other) const;
    GDRBLX_INLINE LuaObject operator ==(const char *p_other) { return *this==LuaObject(p_other); }
    LuaObject operator <=(const LuaObject& p_other) const;
    LuaObject operator  <(const LuaObject& p_other) const;
    GDRBLX_INLINE LuaObject operator >=(const LuaObject& p_other) const { return !(*this  < p_other); }
    GDRBLX_INLINE LuaObject operator !=(const LuaObject& p_other) const { return !(*this == p_other); }
    GDRBLX_INLINE LuaObject operator  >(const LuaObject& p_other) const { return !(*this <= p_other); }

    LuaObject concat     (const LuaObject& p_other) const;
    LuaObject len        ()                         const;
    LuaObject tostring   ()                         const;
    
    LuaObject rawget     (const LuaObject& p_key  ) const;
    LuaObject rawlen     ()                         const;

    LuaObject operator [](const LuaObject& p_key  ) const;

    LuaObject&  operator  =(const LuaObject& p_other) &;
    LuaObject&  operator [](const LuaObject& p_key  );

    void rawset(const LuaObject& p_key, const LuaObject& p_value);

    template <typename... Args>
    LuaObject operator ()(const LuauCtx& ctx, Args... p_args) const;

    template <typename... Args>
    GDRBLX_INLINE LuaObject call(const LuauCtx& ctx, Args... p_args) const { return (*this)(ctx, p_args...); }
    template <typename... Args>
    Vec<LuaObject> call_v(const LuauCtx& ctx, Args... p_args) const;
    template <typename... Args>
    GDRBLX_INLINE LuaObject method_call(const LuauCtx& ctx, Args... p_args) const { return (*this)(ctx, p_args...); }
    template <typename... Args>
    Vec<LuaObject> method_call_v(const LuauCtx& ctx, Args... p_args) const;

    uint32_t hash() const;

    GDRBLX_INLINE bool is_null() const { return get_type() == NIL; }
}; // class LuaObject

static const LuaObject NIL_OBJECT_REF = LuaObject();

class LuaObjectHasher {
public:
    GDRBLX_INLINE static uint32_t hash(const LuaObject& p_o) {
        return p_o.hash();
    }
};

}; // gdrblx

#endif //OBJECT_HPP