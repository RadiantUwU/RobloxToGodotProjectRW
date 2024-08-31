#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <templates/tuple.hpp>

#include <core/object.hpp>
#include <core/context.hpp>
#include <core/function.hpp>
#include <core/userdata.hpp>

namespace gdrblx {

class RBXScriptSignal;

class RBXScriptConnection final : private LuaUserdataIndex, private LuaUserdataToString {
    friend class RBXScriptSignal;
    LuaObject ref;
    bool desync = false;
    Arc<RBXScriptSignal> sign;
    static int lua_Disconnect(lua_State *L);
    virtual LuaObject lua_get(lua_State *L, LuaObject p_key) const override {
        if (!p_key.is_type(LuaObject::STRING)) {
            LuauFnCtx ctx = L;
            ctx.errorf("expected argument #2 to be of type string, got %s", p_key.get_typename());
        } else if (p_key == "Connected") {
            return ref.get_type() != LuaObject::NIL;
        } else if (p_key == "Disconnect") {
            return LuaFunction(lua_Disconnect, "RBXScriptConnection::Disconnect");
        } else {
            LuauFnCtx ctx = L;
            ctx.error("invalid index provided to argument #2.");
        }
    }
    operator LuaString() const override {
        return "RBXScriptConnection";
    }
}; 

class RBXScriptSignal final : private LuaUserdataIndex, private LuaUserdataToString {
    friend class RBXScriptConnection;
    HashMap<LuauState*, LocalVec<tuple<bool, LuaObject>>> connected_functions;

    Arc<RBXScriptConnection> _connect(LuauState &p_state, const Arc<RBXScriptSignal>& p_self, bool p_desynchronized, const LuaObject& p_func);

    static int lua_Connect(lua_State *L);
    static int lua_ConnectParallel(lua_State *L);
    static int lua_Wait(lua_State *L);
    static int lua_Once(lua_State *L);

    virtual LuaObject lua_get(lua_State *L, LuaObject p_key) const override {
        if (!p_key.is_type(LuaObject::STRING)) {
            LuauFnCtx ctx = L;
            ctx.errorf("expected argument #2 to be of type string, got %s", p_key.get_typename());
        } else if (p_key == "Connect") {
            return LuaFunction(lua_Connect, "RBXScriptSignal::Connect");
        } else if (p_key == "Wait") {
            return LuaFunction(lua_Wait, "RBXScriptSignal::Wait");
        } else if (p_key == "ConnectParallel") {
            return LuaFunction(lua_ConnectParallel, "RBXScriptSignal::ConnectParallel");
        } else if (p_key == "Once") {
            return LuaFunction(lua_Once, "RBXScriptSignal::Once");
        } else {
            LuauFnCtx ctx = L;
            ctx.error("invalid index provided to argument #2.");
        }
    }
public:
    operator LuaString() const override {
        return "RBXScriptSignal";
    }
    void Fire(LuaTuple p_args) const;
    template <typename... Args>
    GDRBLX_INLINE void Fire(Args... p_args) const {Fire(LuaTuple(p_args...));}
    void FireNow(LuaTuple p_args) const;
    template <typename... Args>
    GDRBLX_INLINE void FireNow(Args... p_args) const {Fire(LuaTuple(p_args...));}
};

USERDATA_INITIALIZER(RBXScriptConnection, UD_RBXSCRIPTCONNECTION);
USERDATA_INITIALIZER(RBXScriptSignal, UD_RBXSCRIPTSIGNAL);

} // namespace gdrblx

#endif
