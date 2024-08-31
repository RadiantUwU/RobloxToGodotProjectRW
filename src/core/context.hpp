#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <lua.h>
#include <luacode.h>

#include <godot_cpp/templates/vector.hpp>
#include <templates/result.hpp>
#include <templates/property.hpp>

#include "object.hpp"
#include "state.hpp"
#include "scheduler.hpp"
#include "function.hpp"
#include "userdata.hpp"
#include "lua_tuple.hpp"

namespace gdrblx {

class LuauCtx {
    friend class LuaObject;
    friend class TaskScheduler;

    LuauState *const pv_state;
protected:
    lua_State *const L;
private:
    size_t last_stack_size = 0;
private:
    GDRBLX_INLINE void set_stack_size(size_t size) const {
        lua_settop(L, size);
    }
    GDRBLX_INLINE void pop(size_t size) const {
        lua_pop(L, size);
    }
protected:
    GDRBLX_INLINE void dont_clear_stack() {
        last_stack_size = 0;
    }
    GDRBLX_INLINE size_t get_stack_size() const {
        return last_stack_size;
    }
    GDRBLX_INLINE LuauState& get_state() const {
        return *pv_state;
    }
    GDRBLX_INLINE RobloxVM& get_vm() const {
        return *pv_state->get_vm();
    }
    GDRBLX_INLINE TaskScheduler& get_scheduler() const {
        return *pv_state->get_scheduler();
    }
public:
    GDRBLX_INLINE LuauCtx(lua_State *p_L) : L(p_L), pv_state(([](lua_State *p_L) {
        lua_getfield(p_L, LUA_REGISTRYINDEX, "luau_state");
        LuauState* state = (LuauState*)lua_tolightuserdata(p_L, -1);
        lua_pop(p_L, 1);
        return state;
    })(p_L)) {
        last_stack_size = get_stack_size();
    }
    ~LuauCtx() {
        if (last_stack_size != 0)
            set_stack_size(last_stack_size); // resize stack
    }
protected:
    // Creates a new local
    GDRBLX_INLINE LuaObject as_local(size_t stack_pos) const {
        return LuaObject(L, stack_pos);
    }
    template <typename T, typename... Args>
    GDRBLX_INLINE size_t push_objects(T p_obj, Args... p_args) const {
        ((LuaObject)p_obj).push_to_stack(pv_state, L);
        return 1 + push_objects(p_args...);
    }
    template <typename T>
    GDRBLX_INLINE size_t push_objects(T p_obj) const {
        ((LuaObject)p_obj).push_to_stack(pv_state, L);
        return 1;
    }
    GDRBLX_INLINE size_t push_objects() const {
        return 0;
    }
    GDRBLX_INLINE size_t push_objects(const LuaTuple& p_tuple) const {
        for (int i = 1; i <= p_tuple.get_size(); i++) {
            p_tuple[i].push_to_stack(pv_state, L);
        }
        return p_tuple.get_size();
    }

    GDRBLX_INLINE LuaObject get_globals() const {
        return as_local(LUA_GLOBALSINDEX);
    }
    GDRBLX_INLINE LuaObject get_registry() const {
        return as_local(LUA_REGISTRYINDEX);
    }
    GDRBLX_INLINE void set_globals(const LuaObject& p_other) {
        LuaObject _G = get_globals();
        _G = p_other;
    }
    
    GDRBLX_INLINE static void push_function(LuauState* p_state, lua_State* L, const LuaFunction& p_func) {
        switch (p_func.type) {
            case LuaFunction::CFUNC:
                lua_pushcclosurek(L, p_func.cf.cfunc, p_func.cf.cfunc_name, p_func.cf.upvalues.get_size(), p_func.cf.cont);
                for (int i = 1; i <= p_func.cf.upvalues.get_size(); i++) {
                    p_func.cf.upvalues.get(i).push_to_stack(p_state, L);
                    lua_setupvalue(L, -2, i);
                }
                break;
            case LuaFunction::LCFUNC:
                p_func.lc.env.push_to_stack(p_state, L);
                luau_load(L, p_func.lc.chunkname, p_func.lc.bytecode, p_func.lc.bytecode.l, -1);
                lua_remove(L, -2);
                break;
            case LuaFunction::SFUNC:
                p_func.l.push_to_stack(p_state, L);
                break;
        }
    }
public:
    PROPERTY_READONLY(LuauState&, state, LuauCtx, get_state);
    PROPERTY_READONLY(RobloxVM&, vm, LuauCtx, get_vm);
    PROPERTY_READONLY(TaskScheduler&, task, LuauCtx, get_scheduler);
    PROPERTY(LuaObject, globals, LuauCtx, get_globals, set_globals);
    PROPERTY_READONLY(LuaObject, registry, LuauCtx, get_registry);

    GDRBLX_INLINE LuauCtx(const LuauCtx& p_other) : pv_state(p_other.pv_state), L(p_other.L) {
        last_stack_size = lua_gettop(L);
    }
    // Creates a new local
    GDRBLX_INLINE LuaObject make_local() const {
        lua_pushnil(L);
        return LuaObject(L, lua_gettop(L));
    }
    // Creates a new local
    GDRBLX_INLINE LuaObject make_local(const LuaObject& p_assigned) const {
        lua_pushnil(L);
        LuaObject o(L, lua_gettop(L));
        o = p_assigned;
        return o;
    }

    operator LuaThread() const {
        LuaObject l = make_local();
        lua_pop(L, 1);
        lua_pushthread(L);
        return LuaThread(std::move(l).clone_in(pv_state));
    }

    double clock() const {
        return lua_clock();
    }

    // RETURN THIS VALUE INSIDE THE C FUNCTION
    // If you want continuation add a continuation function while pushing the C function.
    template <typename... Args>
    GDRBLX_INLINE auto yield(Args... p_args) const {
        size_t args_pushed = push_objects(p_args...);
        return lua_yield(L, args_pushed);
    }
    GDRBLX_INLINE auto yield(LuaTuple p_args) const {
        size_t args_pushed = push_objects(p_args);
        return lua_yield(L, args_pushed);
    }
    GDRBLX_INLINE auto wait(double p_duration) const {
        push_objects(LuaFunction(get_scheduler().lua_wait,"TaskScheduler::wait"), p_duration);
        lua_call(L, 1, 1);
        return LUA_YIELD;
    }
    GDRBLX_INLINE [[noreturn]] void terminate() const {
        push_objects(get_scheduler().lua_terminate);
        lua_call(L, 0, 0);

        UNREACHABLE();
    }
    template <typename... Args>
    LuaObject resume(const LuaThread& p_thread, Args... p_args) const {
        LuaObject thread = p_thread.native.as_local(L);
        {
            LuaObject::Type t = thread.get_type();
            if (t != LuaObject::THREAD)
                errorf("resume(): expected type thread, got %s", LuaObject::static_get_typename(t));
        }
        lua_State *thr = lua_tothread(L, thread.local_stack_pos);
        LuauCtx ctx = thr;
        int nargs,nres;
        nargs = ctx.push_objects(p_args...);
        nres = lua_resume(thr, L, nargs);
        if (nres > 1) {
            lua_pop(thr, nres-1);
        } else if (nres == 0) {
            lua_pushnil(thr);
        }
        return LuaObject::convert(thr, -1);
    }
    template <typename... Args>
    LuaTuple resume_v(const LuaThread& p_thread, Args... p_args) const {
        LuaObject thread = p_thread.native.as_local(L);
        {
            LuaObject::Type t = thread.get_type();
            if (t != LuaObject::THREAD)
                errorf("resume(): expected type thread, got %s", LuaObject::static_get_typename(t));
        }
        lua_State *thr = lua_tothread(L, thread.local_stack_pos);
        LuauCtx ctx = thr;
        int nargs,nres;
        nargs = ctx.push_objects(p_args...);
        nres = lua_resume(thr, L, nargs);
        Vec<LuaObject> vec;
        for (int i = 1; i <= nres; i++)
            vec.push_back(LuaObject::convert(thr, -nres));
        return std::move(vec);
    }
    LuaObject resume(const LuaThread& p_thread, LuaTuple p_args) const {
        LuaObject thread = p_thread.native.as_local(L);
        {
            LuaObject::Type t = thread.get_type();
            if (t != LuaObject::THREAD)
                errorf("resume(): expected type thread, got %s", LuaObject::static_get_typename(t));
        }
        lua_State *thr = lua_tothread(L, thread.local_stack_pos);
        LuauCtx ctx = thr;
        int nargs,nres;
        nargs = ctx.push_objects(p_args);
        nres = lua_resume(thr, L, nargs);
        if (nres > 1) {
            lua_pop(thr, nres-1);
        } else if (nres == 0) {
            lua_pushnil(thr);
        }
        return LuaObject::convert(thr, -1);
    }
    template <typename... Args>
    LuaTuple resume_v(const LuaThread& p_thread, LuaTuple p_args) const {
        LuaObject thread = p_thread.native.as_local(L);
        {
            LuaObject::Type t = thread.get_type();
            if (t != LuaObject::THREAD)
                errorf("resume(): expected type thread, got %s", LuaObject::static_get_typename(t));
        }
        lua_State *thr = lua_tothread(L, thread.local_stack_pos);
        LuauCtx ctx = thr;
        int nargs,nres;
        nargs = ctx.push_objects(p_args);
        nres = lua_resume(thr, L, nargs);
        Vec<LuaObject> vec;
        for (int i = 1; i <= nres; i++)
            vec.push_back(LuaObject::convert(thr, -nres));
        return std::move(vec);
    }

    GDRBLX_INLINE void gc(int p_what, int p_data) const {
        lua_gc(L, p_what, p_data);
    }

    template <typename... Args>
    GDRBLX_INLINE LuaObject call(const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args...), 1);
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        return std::move(o);
    }
    template <typename... Args>
    GDRBLX_INLINE LuaTuple call(size_t nres, const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args...), nres);
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        return std::move(vec);
    }
    template <typename... Args>
    GDRBLX_INLINE LuaTuple call_v(const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        size_t stack_size = get_stack_size();
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args...), LUA_MULTRET);
        size_t nres = get_stack_size()-nres;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        return std::move(vec);
    }
    GDRBLX_INLINE LuaObject call(const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args), 1);
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        return std::move(o);
    }
    GDRBLX_INLINE LuaTuple call(size_t nres, const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args), nres);
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        return std::move(vec);
    }
    GDRBLX_INLINE LuaTuple call_v(const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        size_t stack_size = get_stack_size();
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        lua_call(L, push_objects(p_args), LUA_MULTRET);
        size_t nres = get_stack_size()-nres;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        return std::move(vec);
    }

    GDRBLX_INLINE bool is_null(const LuaFunction &p_func) const {
        if (!p_func.valid()) return true;
        switch (p_func.type) {
            case LuaFunction::SFUNC:
                return p_func.l.get_luau_state() != pv_state;
            default:
                return false;
        }
    }

    template <typename... Args>
    GDRBLX_INLINE Result<LuaObject, LuaObject> pcall(const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), 1, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        if (status == LUA_OK) 
            return Result<LuaObject, LuaObject>::create_result(std::move(o));
        else
            return Result<LuaObject, LuaObject>::create_error(std::move(o));
    }
    template <typename... Args>
    GDRBLX_INLINE Result<LuaTuple, LuaObject> pcall(size_t nres, const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), nres, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }
    template <typename... Args>
    GDRBLX_INLINE Result<LuaTuple, LuaObject> pcall_v(const LuaFunction& p_func, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        size_t stack_size = get_stack_size();
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), LUA_MULTRET, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        size_t nres = get_stack_size()-nres;
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }

    template <typename... Args>
    GDRBLX_INLINE Result<LuaObject, LuaObject> xpcall(const LuaFunction& p_func, const LuaFunction& p_errh, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), 1, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        if (status == LUA_OK) 
            return Result<LuaObject, LuaObject>::create_result(std::move(o));
        else
            return Result<LuaObject, LuaObject>::create_error(std::move(o));
    }
    template <typename... Args>
    GDRBLX_INLINE Result<LuaTuple, LuaObject> xpcall(size_t nres, const LuaFunction& p_func, const LuaFunction& p_errh, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), nres, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }
    template <typename... Args>
    GDRBLX_INLINE Result<LuaTuple, LuaObject> xpcall_v(const LuaFunction& p_func, const LuaFunction& p_errh, Args... p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        size_t stack_size = errh_pos;
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args...), LUA_MULTRET, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        size_t nres = get_stack_size()-nres;
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }

    GDRBLX_INLINE Result<LuaObject, LuaObject> pcall(const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), 1, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        if (status == LUA_OK) 
            return Result<LuaObject, LuaObject>::create_result(std::move(o));
        else
            return Result<LuaObject, LuaObject>::create_error(std::move(o));
    }
    GDRBLX_INLINE Result<LuaTuple, LuaObject> pcall(size_t nres, const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), nres, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }
    GDRBLX_INLINE Result<LuaTuple, LuaObject> pcall_v(const LuaFunction& p_func, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        size_t stack_size = get_stack_size();
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), LUA_MULTRET, 0);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        size_t nres = get_stack_size()-nres;
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }

    GDRBLX_INLINE Result<LuaObject, LuaObject> xpcall(const LuaFunction& p_func, const LuaFunction& p_errh, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), 1, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        LuaObject o = LuaObject::convert(L, -1);
        lua_pop(L, 1);
        if (status == LUA_OK) 
            return Result<LuaObject, LuaObject>::create_result(std::move(o));
        else
            return Result<LuaObject, LuaObject>::create_error(std::move(o));
    }
    GDRBLX_INLINE Result<LuaTuple, LuaObject> xpcall(size_t nres, const LuaFunction& p_func, const LuaFunction& p_errh, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), nres, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }
    GDRBLX_INLINE Result<LuaTuple, LuaObject> xpcall_v(const LuaFunction& p_func, const LuaFunction& p_errh, LuaTuple p_args) const {
        DEV_ASSERT(p_func.valid());
        DEV_ASSERT(p_errh.valid());
        push_function(pv_state, L, p_errh);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        size_t errh_pos = lua_gettop(L);
        size_t stack_size = errh_pos;
        push_function(pv_state, L, p_func);
        if (lua_type(L, -1) == LUA_TNIL) {
            lua_pushstring(L, "cannot call nil value.");
            lua_error(L);
        }
        int status = lua_pcall(L, push_objects(p_args), LUA_MULTRET, errh_pos);
        lua_remove(L, errh_pos);
        if (status == LUA_ERRMEM) pv_state->raise_oom_error();
        size_t nres = get_stack_size()-nres;
        bool success = status == LUA_OK;
        Vec<LuaObject> vec;
        for (int i = 0; i < nres; i++) {
            vec.push_back(LuaObject::convert(L, -nres+i));
        }
        lua_pop(L, nres);
        if (success) 
            return Result<LuaTuple, LuaObject>::create_result(std::move(vec));
        else
            return Result<LuaTuple, LuaObject>::create_error(std::move(vec)[0]);
    }

    GDRBLX_INLINE [[noreturn]] void error(const LuaObject& p_err) const {
        push_objects(p_err);
        lua_error(L);
    }
    template <typename... Args>
    GDRBLX_INLINE LuaString stringf(const LuaString& p_str, Args... p_args) const {
        push_objects(pv_state->get_stringf());
        lua_call(L, push_objects(p_str, p_args...), 1);
        size_t len;
        const char* str = lua_tolstring(L, -1, &len);
        LuaString s = LuaString(str, len);
        lua_pop(L, 1);
        return s;
    }
    template <typename... Args>
    GDRBLX_INLINE [[noreturn]] void errorf(const LuaString& p_str, Args... p_args) const {
        //error(stringf(p_str, p_args...));
        push_objects(pv_state->get_stringf());
        lua_call(L, push_objects(p_str, p_args...), 1);
        lua_error(L);
    }

    template <typename ... Args>
    GDRBLX_INLINE LuaFunction create_lambda(lua_CFunction p_cf, Args... p_args) {
        return LuaFunction(p_cf, "<C++ lambda>", LuaTuple(p_args...));
    }
    template <typename ... Args>
    GDRBLX_INLINE LuaFunction create_lambda(lua_CFunction p_cf, lua_Continuation p_cont, Args... p_args) {
        return LuaFunction(p_cf, "<C++ lambda>", LuaTuple(p_args...), p_cont);
    }

    Result<LuaFunction, LuaString> compile_release(const LuaString& p_source, const LuaString& p_name, const LuaObject& p_env = NIL_OBJECT_REF) const;
    Result<LuaFunction, LuaString> compile_debug(const LuaString& p_source, const LuaString& p_name, const LuaObject& p_env = NIL_OBJECT_REF) const;
};

class LuauFnCtx : public LuauCtx {
public:
    GDRBLX_INLINE LuauFnCtx(lua_State *L) : LuauCtx(L) {
        dont_clear_stack();
    }
    GDRBLX_INLINE LuaObject get_arg(int p_arg_n) const {
        return as_local(p_arg_n);
    }
    GDRBLX_INLINE int get_args_count() const {
        return get_stack_size();
    }
    GDRBLX_INLINE LuaTuple get_args(int from = 1) const {
        int size = get_args_count();
        Vec<LuaObject> vec;
        for (int i = from; i <= size; i++) {
            vec.push_back(get_arg(i));
        }
        return std::move(vec);
    }
    GDRBLX_INLINE LuaObject expect(int p_arg_n, LuaObject::Type p_type) const {
        LuaObject expected = get_arg(p_arg_n);
        if (!expected.is_type(p_type)) {
            errorf(
                "expected argument #%d to be of type %s, got %s",
                p_arg_n,
                LuaObject::static_get_typename(p_type),
                lua_typename(L, lua_type(L, p_arg_n))
            );
        }
        return std::move(expected);
    }
    GDRBLX_INLINE LuaObject expect(int p_arg_n, UserdataType p_type) const {
        LuaObject expected = get_arg(p_arg_n);
        if (!expected.is_type(p_type)) {
            errorf(
                "expected argument #%d to be of type %s, got %s",
                p_arg_n,
                LuaObject::static_get_typename(p_type),
                lua_typename(L, lua_type(L, p_arg_n))
            );
        }
        return std::move(expected);
    }
    template <typename... Args>
    GDRBLX_INLINE int return_call(Args... args) const {
        return push_objects(args...);
    }
    GDRBLX_INLINE void expect_argn(int p_arg_min) const {
        if (get_args_count() != p_arg_min)
            errorf("expected %d arguments, got %d", p_arg_min, get_args_count());
    }
    GDRBLX_INLINE void expect_argn(int p_arg_min, int p_arg_max) const {
        if (get_args_count() < p_arg_min or get_args_count() > p_arg_max)
            errorf("expected between %d and %d arguments, got %d", p_arg_min, p_arg_max, get_args_count());
    }
    GDRBLX_INLINE void expect_argn_v(int p_arg_min) const {
        if (get_args_count() < p_arg_min) {
            errorf("expected a minimum of %d arguments, got %d", p_arg_min, get_args_count());
        }
    }
    GDRBLX_INLINE LuaObject get_upvalue(int p_up_n) const {
        return as_local(lua_upvalueindex(p_up_n));
    }
};

} // namespace gdrblx

#endif