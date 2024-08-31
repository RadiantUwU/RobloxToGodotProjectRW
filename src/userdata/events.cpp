#include "events.hpp"

namespace gdrblx {

int RBXScriptConnection::lua_Disconnect(lua_State *L) {
    LuauFnCtx ctx = L;
    ctx.expect_argn(1);
    auto conn = ctx.expect(1, UD_RBXSCRIPTCONNECTION).as_userdata<RBXScriptConnection>().write(); // RBXScriptConnection*
    if (conn->ref.is_type(LuaObject::NIL))
        return ctx.return_call(); // reference already removed.
    auto signal = conn->sign.write(); // RBXScriptSignal*
    signal->connected_functions.get(conn->ref.get_luau_state()).erase(tuple<bool, LuaObject>(conn->desync,conn->ref)); // remove the connection
    conn->ref = NIL_OBJECT_REF; // mark as disconnected.
    return ctx.return_call();
}

Arc<RBXScriptConnection> RBXScriptSignal::_connect(LuauState &p_state, const Arc<RBXScriptSignal>& p_self, bool p_desynchronized, const LuaObject& p_func) {
    auto it = connected_functions.find(&p_state);
    if (it == connected_functions.end()) {
        connected_functions[&p_state].push_back(tuple<bool, LuaObject>(p_desynchronized, p_func));
    } else {
        it->value.push_back(tuple<bool, LuaObject>(p_desynchronized, p_func));
    }

    Arc<RBXScriptConnection> protected_connection = RBXScriptConnection();
    RBXScriptConnection& connection = protected_connection.unsafe_access();
    connection.desync = p_desynchronized;
    connection.sign = p_self;
    connection.ref = p_func;
    return protected_connection;
}

int RBXScriptSignal::lua_Connect(lua_State *L) {
    LuauFnCtx ctx = L;
    LuauState& state = ctx.state;

    ctx.expect_argn(2);
    Arc<RBXScriptSignal> protected_signal = ctx.expect(1, UD_RBXSCRIPTSIGNAL).as_userdata<RBXScriptSignal>();
    auto signal = protected_signal.write(); // RBXScriptSignal*
    LuaObject func = ctx.expect(2, LuaObject::FUNCTION);

    return ctx.return_call(signal->_connect(state, protected_signal, false, func));
}

int RBXScriptSignal::lua_ConnectParallel(lua_State *L) {
    LuauFnCtx ctx = L;
    LuauState& state = ctx.state;

    ctx.expect_argn(2);
    Arc<RBXScriptSignal> protected_signal = ctx.expect(1, UD_RBXSCRIPTSIGNAL).as_userdata<RBXScriptSignal>();
    auto signal = protected_signal.write(); // RBXScriptSignal*
    LuaObject func = ctx.expect(2, LuaObject::FUNCTION);

    return ctx.return_call(signal->_connect(state, protected_signal, true, func));
}

int RBXScriptSignal::lua_Wait(lua_State *L) {
    LuauFnCtx ctx = L;
    LuauState& state = ctx.state;
    bool desynchronized = !state.synchronized();

    ctx.expect_argn(1);
    Arc<RBXScriptSignal> protected_signal = ctx.expect(1, UD_RBXSCRIPTSIGNAL).as_userdata<RBXScriptSignal>();
    auto signal = protected_signal.write(); // RBXScriptSignal*
    LuaObject func = LuaObject(ctx.create_lambda((lua_CFunction)[](lua_State *inner_L){
        LuauFnCtx inner_ctx = inner_L;
        inner_ctx.get_upvalue(2).rawget("Disconnect").method_call(inner_ctx);
        ((LuauState&)inner_ctx.state).get_scheduler()->spawn((LuaThread)inner_ctx.get_upvalue(1), inner_ctx.get_args());
        return inner_ctx.return_call();
    }, L, NIL_OBJECT_REF)).clone_in(&state);

    LuaFunction(func).set_upvalue(2, signal->_connect(state, protected_signal, desynchronized, func));
    return ctx.yield();
}

int RBXScriptSignal::lua_Once(lua_State *L) {
    LuauFnCtx ctx = L;
    LuauState& state = ctx.state;
    bool desynchronized = !state.synchronized();

    ctx.expect_argn(2);
    Arc<RBXScriptSignal> protected_signal = ctx.expect(1, UD_RBXSCRIPTSIGNAL).as_userdata<RBXScriptSignal>();
    auto signal = protected_signal.write(); // RBXScriptSignal*
    LuaObject passed_func = ctx.expect(2, LuaObject::FUNCTION);
    LuaObject func = LuaObject(ctx.create_lambda([](lua_State *L){
        LuauFnCtx ctx = L;
        ctx.get_upvalue(2).rawget("Disconnect").method_call(ctx);
        ((LuauState&)ctx.state).get_scheduler()->spawn(LuaFunction(ctx.get_upvalue(1)), ctx.get_args());
        return ctx.return_call();
    }, passed_func, NIL_OBJECT_REF)).clone_in(&state);

    LuaFunction(func).set_upvalue(2, signal->_connect(state, protected_signal, desynchronized, func));
    return ctx.yield();
}

}