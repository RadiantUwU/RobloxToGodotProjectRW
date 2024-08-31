#ifndef STATE_HPP
#define STATE_HPP

#include <lua.h>

#include <godot_cpp/classes/rw_lock.hpp>

#include "object.hpp"
#include "thread.hpp"

namespace gdrblx {

class TaskScheduler;
class RobloxVM;

class LuauState final {
    friend class LuaObject;
    friend class TaskScheduler;

    RobloxVM *const vm;
    TaskScheduler *const scheduler;
    lua_Callbacks *callbacks;

    LuaObject stringf;

    lua_State *const L;

    ::godot::RWLock rwlock;
    LuauState(RobloxVM* p_vm, TaskScheduler* p_scheduler);
    LuaThread create_thread();

    void userthread(lua_State *LP, lua_State *L);
    

public:
    LuauState(RobloxVM* p_vm);
    ~LuauState();

    LuaThread create_thread(LuaFunction p_func);

    GDRBLX_INLINE TaskScheduler* get_scheduler() { return scheduler; }
    GDRBLX_INLINE RobloxVM* get_vm() { return vm; }
    GDRBLX_INLINE const LuaObject& get_stringf() { return stringf; }
    GDRBLX_INLINE lua_Callbacks *get_callbacks() const { return callbacks; }

    bool synchronized() const;

    void raise_oom_error() const;
};

} // namespace gdrblx

#endif // STATE_HPP