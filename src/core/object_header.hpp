#ifndef OBJECT_HEADER_HPP
#define OBJECT_HEADER_HPP

#include <godot_cpp/core/safe_refcount.hpp>

#include "macros.hpp"
#include "object.hpp"
#include "buffer.hpp"
#include "function.hpp"
#include "table.hpp"

namespace gdrblx {

namespace internal {

class LuaObjectHeader {
    friend class ::gdrblx::LuaObject;

    union {
        LuaBuffer b;
        LuaFunction f;
        LuaTable t;

        struct {
            LuauState* ref_owner;
            size_t ref_pos;
        };
    };
    ::godot::SafeRefCount ref_count;
    enum {
        TYPE_BUF,
        TYPE_FUNC,
        TYPE_TBL,
        TYPE_REF
    } type;

    GDRBLX_INLINE LuaObjectHeader(const LuaBuffer& p_b) : b(p_b), type(TYPE_BUF) {
        ref_count.init();
    }
    GDRBLX_INLINE LuaObjectHeader(const LuaFunction& p_f) : f(p_f), type(TYPE_FUNC) {
        ref_count.init();
    }
    GDRBLX_INLINE LuaObjectHeader(const LuaTable& p_t) : t(p_t), type(TYPE_TBL) {
        ref_count.init();
    }
    GDRBLX_INLINE LuaObjectHeader(LuauState* p_ref_owner, size_t p_ref_pos) : ref_pos(p_ref_pos), ref_owner(p_ref_owner), type(TYPE_REF) {
        ref_count.init();
    };
    GDRBLX_INLINE ~LuaObjectHeader() {
        switch (type) {
            TYPE_BUF:
                b.~LuaBuffer();
                break;
            TYPE_FUNC:
                f.~LuaFunction();
                break;
            TYPE_TBL:
                t.~LuaTable();
                break;
            TYPE_REF:
                LuaObject::set_ref_to_nil(ref_owner, ref_pos);
                break;
        }
    }

    GDRBLX_INLINE LuaObject::Type get_type() const {
        switch (type) {
            TYPE_BUF:
                return LuaObject::BUFFER;
            TYPE_FUNC:
                return LuaObject::FUNCTION;
            TYPE_TBL:
                return LuaObject::TABLE;
            TYPE_REF:
                return LuaObject::get_type_from_ref(ref_owner, ref_pos);
        }
    }

    GDRBLX_INLINE LuaObject::Type get_header_type() const {
        switch (type) {
            TYPE_BUF:
                return LuaObject::BUFFER;
            TYPE_FUNC:
                return LuaObject::FUNCTION;
            TYPE_TBL:
                return LuaObject::TABLE;
            TYPE_REF:
                return LuaObject::REF;
        }
    }

    GDRBLX_INLINE void incref() {
        ref_count.ref();
    }

    GDRBLX_INLINE void decref() {
        if (ref_count.unref()) 
            ::godot::memdelete(this);
    }
};

} // namespace internal

} // namespace gdrblx

#endif