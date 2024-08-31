#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <templates/property.hpp>
#include <templates/option.hpp>

#include <core/object.hpp>
#include <core/string.hpp>
#include <core/userdata.hpp>

#include <userdata/events.hpp>

#include "instance_meta.hpp"

namespace gdrblx {

class RBXScriptSignal;

#define INSTANCE_SIGNAL(p_name, p_class_name)                                                   \
    protected:                                                                                  \
        Arc<RBXScriptSignal> _PRIVATE_##p_name;                                                 \
    public:                                                                                     \
        PROPERTY_READONLY_PROXY(Arc<RBXScriptSignal>, p_name, p_class_name, _PRIVATE_##p_name);

#define INSTANCE_SIGNAL_EMIT(p_name, ...)  \
    (_PRIVATE_##p_name)->Fire(__VA_ARGS__)

class Instance : private LuaUserdataIndex, private LuaUserdataSetIndex, private LuaUserdataToString, private LuaUserdataInit<Instance> {
    GDRBLX_INLINE UserdataType get_userdata_type() const override {
        return UD_INSTANCE;
    }
protected:
    virtual LuaString get_class_name() const {
        return "Instance";
    }
private:
    static constexpr uint64_t INVALID_UNIQUEID = 0;
    uint64_t uniqueid = INVALID_UNIQUEID;

    Vector<Arc<Instance>> children;
    Option<Arc<Instance>> parent;

    Arc<Instance> get_parent() const;
    void set_parent(Arc<Instance> p_other);

    HashMap<LuaString, LuaObject, LuaStringHasher> attributes;
    HashMap<LuaString, Arc<RBXScriptSignal>, LuaStringHasher> attribute_changed;
    HashMap<LuaString, Arc<RBXScriptSignal>, LuaStringHasher> property_changed;
public:
    GDRBLX_INLINE Instance() {}
    GDRBLX_INLINE Instance(uint64_t p_uniqueid) : uniqueid(p_uniqueid) {}
    virtual ~Instance() {}

    bool Archivable = false;
    PROPERTY_READONLY(LuaString, ClassName, Instance, get_class_name);
    LuaString Name = get_class_name();
    PROPERTY(Arc<Instance>, Parent, Instance, get_parent, set_parent);
    PROPERTY_READONLY_PROXY(uint64_t, UniqueId, Instance, uniqueid);

    static int AddTag(lua_State *L);
    static int ClearAllChildren(lua_State *L);
    static int Clone(lua_State *L);
    static int Destroy(lua_State *L);
    static int FindFirstAncestor(lua_State *L);
    static int FindFirstAncestorOfClass(lua_State *L);
    static int FindFirstAncestorWhichIsA(lua_State *L);
    static int FindFirstChild(lua_State *L);
    static int FindFirstChildOfClass(lua_State *L);
    static int FindFirstChildWhichIsA(lua_State *L);
    static int FindFirstDescendant(lua_State *L);
    static int GetActor(lua_State *L);
    static int GetAttribute(lua_State *L);
    static int GetAttributeChangedSignal(lua_State *L);
    static int GetAttributes(lua_State *L);
    static int GetChildren(lua_State *L);
    static int GetDescendants(lua_State *L);
    static int GetFullName(lua_State *L);
    static int GetPropertyChangedSignal(lua_State *L);
    static int GetTags(lua_State *L);
    static int HasTag(lua_State *L);
    static int IsA(lua_State *L);
    static int IsAncestorOf(lua_State *L);
    static int IsDescendantOf(lua_State *L);
    static int RemoveTag(lua_State *L);
    static int SetAttribute(lua_State *L);
    static int WaitForChild(lua_State *L);

    INSTANCE_SIGNAL(AncestryChanged, Instance);
    INSTANCE_SIGNAL(AttributeChanged, Instance);
    INSTANCE_SIGNAL(Changed, Instance);
    INSTANCE_SIGNAL(ChildAdded, Instance);
    INSTANCE_SIGNAL(ChildRemoved, Instance);
    INSTANCE_SIGNAL(DescendantAdded, Instance);
    INSTANCE_SIGNAL(DescendantRemoving, Instance);
    INSTANCE_SIGNAL(Destroying, Instance);

private:
    inline static InstanceMethods methods = InstanceMethods()
                    .register_method("AddTag",&AddTag)
                    .register_method("ClearAllChildren",&ClearAllChildren)
                    .register_method("Clone",&Clone)
                    .register_method("Destroy",&Destroy)
                    .register_method("FindFirstAncestor",&FindFirstAncestor)
                    .register_method("FindFirstAncestorOfClass",&FindFirstAncestorOfClass)
                    .register_method("FindFirstAncestorWhichIsA",&FindFirstAncestorWhichIsA)
                    .register_method("FindFirstChild",&FindFirstChild)
                    .register_method("FindFirstChildOfClass",&FindFirstChildOfClass)
                    .register_method("FindFirstChildWhichIsA",&FindFirstChildWhichIsA)
                    .register_method("FindFirstDescendant",&FindFirstDescendant)
                    .register_method("GetActor",&GetActor)
                    .register_method("GetAttribute",&GetAttribute)
                    .register_method("GetAttributeChangedSignal",&GetAttributeChangedSignal)
                    .register_method("GetAttributes",&GetAttributes)
                    .register_method("GetChildren",&GetChildren)
                    .register_method("GetDescendants",&GetDescendants)
                    .register_method("GetFullName",&GetFullName)
                    .register_method("GetPropertyChangedSignal",&GetPropertyChangedSignal)
                    .register_method("GetTags",&GetTags)
                    .register_method("HasTag",&HasTag)
                    .register_method("IsA",&IsA)
                    .register_method("IsAncestorOf",&IsAncestorOf)
                    .register_method("IsDescendantOf",&IsDescendantOf)
                    .register_method("RemoveTag",&RemoveTag)
                    .register_method("SetAttribute",&SetAttribute)
                    .register_method("WaitForChild",&WaitForChild);

protected:
    LuaObject lua_get(lua_State *L, LuaObject p_key) const override;
    void lua_set(lua_State *L, LuaObject p_key, LuaObject p_value) override;
    virtual operator LuaString() const override;

protected:
    virtual bool instance_mro_get(LuauFnCtx& p_ctx, LuaObject p_key) const;
    virtual bool instance_mro_set(LuauFnCtx& p_ctx, LuaObject p_key, LuaObject p_value) const;
private:
    bool _instance_mro_get(LuauFnCtx& p_ctx, LuaObject p_key) const;
    bool _instance_mro_set(LuauFnCtx& p_ctx, LuaObject p_key, LuaObject p_value) const;

public:
    static void lua_init(LuauState* p_state);
};

USERDATA_INITIALIZER(Instance, UD_INSTANCE);

}

#endif