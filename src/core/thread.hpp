#ifndef THREAD_HPP
#define THREAD_HPP

#include <lua.h>

#include "macros.hpp"
#include "object.hpp"
#include "security.hpp"

namespace gdrblx {

class BaseScript;

class ThreadIdentity {
public:
    virtual ThreadIdentityType get_identity() = 0;
    GDRBLX_INLINE SecurityContext get_security_contexts() {
        return (SecurityContext)getSecurityContextForIdentity(get_identity());
    }
};

class LuaThread {
    friend class LuaObject;
    friend class LuauCtx;
    LuaObject native;
    LuaThread(const LuaObject& p_native, int _) : native(p_native) {}
public:
    LuaThread(const LuaObject& p_native) : native(p_native) {
        if (!p_native.is_type(LuaObject::THREAD)) {
            native = NIL_OBJECT_REF;
        }
    }
    void close();
    int vm_state() const;
    lua_CoStatus status() const;
    ThreadIdentity* get_identity() const;
    Arc<BaseScript> get_script() const;
    void set_identity(ThreadIdentity* p_iden);
    GDRBLX_INLINE ThreadIdentityType get_identity_enum() const {
        ThreadIdentity* iden = get_identity();
        if (iden == nullptr)
            return ThreadIdentityType::IDEN_ANON;
        else
            return iden->get_identity();
    }
    GDRBLX_INLINE SecurityContext get_security() const {
        return (SecurityContext)getSecurityContextForIdentity(get_identity_enum());
    }

    template <typename... Args>
    void spawn(Args... p_args);
    template <typename... Args>
    void defer(Args... p_args);
    template <typename... Args>
    void delay(double p_duration, Args... p_args);
    GDRBLX_INLINE void cancel() { close(); }
};

} // namespace gdrblx

#endif // THREAD_HPP