#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <templates/property.hpp>

#include "object.hpp"
#include "table.hpp"
#include "thread.hpp"
#include "state.hpp"
#include "lua_tuple.hpp"

namespace gdrblx {

class RunService;

class TaskScheduler {
    friend class LuauState;
    LuauState* assigned_state;
protected:
    GDRBLX_INLINE RobloxVM* get_vm() const {
        return assigned_state->get_vm();
    }
    RunService* get_run_service() const;
    virtual bool get_synchronized() const {
        return assigned_state->synchronized();
    }
    constexpr static int SYNCHRONIZED = 0;
    constexpr static int DESYNCHRONIZED = 1;
    struct {
        LuaTable defer;
        LuaTable delay;
        LuaTable wait;
    } threads_pending[2];
    bool defer_resume(); // true if there are more to resume.
public:
    static int lua_spawn(lua_State *L);
    static int lua_defer(lua_State *L);
    static int lua_delay(lua_State *L);
    static int lua_synchronize(lua_State *L);
    static int lua_desynchronize(lua_State *L);
    static int lua_synchronized(lua_State *L);
    static int lua_wait(lua_State *L);
    static int lua_cancel(lua_State *L);

    static int lua_terminate(lua_State *L);
    static int lua_terminatek(lua_State *L, int status);

    virtual void frame_step(double delta);

    PROPERTY_READONLY(RobloxVM*, vm, TaskScheduler, get_vm);
    PROPERTY_READONLY(bool, synchronized, TaskScheduler, get_synchronized);

    template <typename... Args>
    LuaThread spawn(const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread spawn(const LuaThread& p_thr, Args... p_args);
    template <typename... Args>
    LuaThread defer(const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread defer(const LuaThread& p_thr, Args... p_args);
    template <typename... Args>
    LuaThread delay(double p_duration, const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread delay(double p_duration, const LuaThread& p_thr, Args... p_args);
    LuaThread spawn(const LuaFunction& p_func, LuaTuple p_args);
    LuaThread spawn(const LuaThread& p_thr, LuaTuple p_args);
    LuaThread defer(const LuaFunction& p_func, LuaTuple p_args);
    LuaThread defer(const LuaThread& p_thr, LuaTuple p_args);
    LuaThread delay(double p_duration, const LuaFunction& p_func, LuaTuple p_args);
    LuaThread delay(double p_duration, const LuaThread& p_thr, LuaTuple p_args);
    template <typename... Args>
    LuaThread spawn(bool desync, const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread spawn(bool desync, const LuaThread& p_thr, Args... p_args);
    template <typename... Args>
    LuaThread defer(bool desync, const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread defer(bool desync, const LuaThread& p_thr, Args... p_args);
    template <typename... Args>
    LuaThread delay(bool desync, double p_duration, const LuaFunction& p_func, Args... p_args);
    template <typename... Args>
    LuaThread delay(bool desync, double p_duration, const LuaThread& p_thr, Args... p_args);
    LuaThread spawn(bool desync, const LuaFunction& p_func, LuaTuple p_args);
    LuaThread spawn(bool desync, const LuaThread& p_thr, LuaTuple p_args);
    LuaThread defer(bool desync, const LuaFunction& p_func, LuaTuple p_args);
    LuaThread defer(bool desync, const LuaThread& p_thr, LuaTuple p_args);
    LuaThread delay(bool desync, double p_duration, const LuaFunction& p_func, LuaTuple p_args);
    LuaThread delay(bool desync, double p_duration, const LuaThread& p_thr, LuaTuple p_args);
    void cancel(const LuaThread& p_thr);
};

} // namespace gdrblx

#endif