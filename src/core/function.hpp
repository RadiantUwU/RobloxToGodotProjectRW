#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <lua.h>

#include <godot_cpp/templates/vector.hpp>

#include "object.hpp"
#include "lua_tuple.hpp"
#include "table.hpp"

namespace gdrblx {

class LuaFunction {
    friend class LuaObject;
    friend class LuauCtx;

    struct CompiledLua {
        LuaObject env;
        LuaString bytecode;
        LuaString chunkname;
    };
    struct CFunc {
        LuaTuple upvalues;
        lua_CFunction cfunc;
        LuaString cfunc_name;
        lua_Continuation cont;
    };
    union {
        CompiledLua lc;
        CFunc cf;
        LuaObject l;
    };
public:
    enum Type {
        LCFUNC,
        SFUNC,
        CFUNC,
    };

private:
    Type type;
    GDRBLX_INLINE LuaFunction(const LuaObject& p_local, int _) : type(SFUNC), l(p_local) {}
public:
    GDRBLX_INLINE LuaFunction() : type(CFUNC) {
        this->cf.cfunc = nullptr;
        new (&this->cf.upvalues) LuaTuple();
    }
    GDRBLX_INLINE LuaFunction(lua_CFunction p_cfunc) : type(CFUNC) {
        this->cf.cfunc = p_cfunc;
        new (&this->cf.cfunc_name) LuaString("<C function>");
        new (&this->cf.upvalues) LuaTuple();
        this->cf.cont = nullptr;
    }
    GDRBLX_INLINE LuaFunction(lua_CFunction p_cfunc, LuaString p_name) : type(CFUNC) {
        this->cf.cfunc = p_cfunc;
        new (&this->cf.cfunc_name) LuaString(p_name);
        new (&this->cf.upvalues) LuaTuple();
        this->cf.cont = nullptr;
    }
    GDRBLX_INLINE LuaFunction(lua_CFunction p_cfunc, LuaString p_name, const LuaTuple& p_upvalues) : type(CFUNC) {
        this->cf.cfunc = p_cfunc;
        new (&this->cf.cfunc_name) LuaString(p_name);
        new (&this->cf.upvalues) LuaTuple(p_upvalues);
        this->cf.cont = nullptr;
    }
    GDRBLX_INLINE LuaFunction(lua_CFunction p_cfunc, LuaString p_name, lua_Continuation p_cont) : type(CFUNC) {
        this->cf.cfunc = p_cfunc;
        new (&this->cf.cfunc_name) LuaString(p_name);
        new (&this->cf.upvalues) LuaTuple();
        this->cf.cont = p_cont;
    }
    GDRBLX_INLINE LuaFunction(lua_CFunction p_cfunc, LuaString p_name, const LuaTuple& p_upvalues, lua_Continuation p_cont) : type(CFUNC) {
        this->cf.cfunc = p_cfunc;
        new (&this->cf.cfunc_name) LuaString(p_name);
        new (&this->cf.upvalues) LuaTuple(p_upvalues);
        this->cf.cont = p_cont;
    }
    GDRBLX_INLINE LuaFunction(LuaString p_chunkname, LuaString p_bytecode, const LuaObject& p_env = NIL_OBJECT_REF) : type(LCFUNC) {
        new (&this->lc.env) LuaObject(p_env);
        new (&this->lc.bytecode) LuaString(p_bytecode);
        new (&this->lc.chunkname) LuaString(p_chunkname);
    }
    GDRBLX_INLINE LuaFunction(const LuaFunction& p_other) : type(p_other.type) {
        switch (type) {
        case LCFUNC:
            new (&lc) CompiledLua(p_other.lc);
            break;
        case CFUNC:
            new (&cf) CFunc(p_other.cf);
            break;
        case SFUNC:
            new (&l) LuaObject(p_other.l);
            break;
        }
    }
    GDRBLX_INLINE LuaFunction(const LuaObject& p_local) : type(SFUNC), l(p_local) {}
    ~LuaFunction() {
        switch (type) {
            case LCFUNC:
                lc.~CompiledLua();
                break;
            case SFUNC:
                l.~LuaObject();
                break;
            case CFUNC:
                cf.~CFunc();
                break;
        }
    }

    GDRBLX_INLINE bool valid() const {
        switch (type) {
            case CFUNC:
                return cf.cfunc != nullptr;
            case LCFUNC:
                return true;
            case SFUNC:
                return l.is_type(LuaObject::FUNCTION);
        }
    }
    
    size_t get_n_upvalues() const;
    Vec<LuaString> get_upvalue_names() const;
    LuaObject get_upvalue(size_t p_idx) const;
    LuaObject get_upvalue(LuaString p_name) const;
    void set_upvalue(size_t p_idx, LuaObject p_val);
    void set_upvalue(LuaString p_name, LuaObject p_val);

    LuaFunction& operator=(const LuaFunction& p_other) {
        this->~LuaFunction();
        new (this) LuaFunction(p_other);
        return *this;
    }
};

}; // namespace gdrblx

#endif // FUNCTION_HPP