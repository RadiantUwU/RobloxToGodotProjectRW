#ifndef INSTANCE_META_HPP
#define INSTANCE_META_HPP

#include <core/state.hpp>
#include <core/function.hpp>
#include <templates/option.hpp>

#include <type_traits>

namespace gdrblx {
class Instance;

template <class T>
concept InstanceClass = std::is_base_of_v<Instance, T>;

namespace internal {
template <class T>
concept InstanceClassInit = InstanceClass<T> && requires(LuauState *state) {T::class_init(state);};
class InstanceClassInternalInitializer {
public:
    virtual void initialize(LuauState* p_state) const = 0;
};

inline static Vec<InstanceClassInternalInitializer*> instance_initializers;
}

template <class T>
class InstanceClassInitializer : internal::InstanceClassInternalInitializer {
    InstanceClassInitializer() {
        static_assert(internal::InstanceClassInit<T>);
        internal::instance_initializers.push_back(this);
    }
    void initialize(LuauState* p_state) const override {
        T::class_init(p_state);
    }
};

#define INSTANCE_INITIALIZER(p_type)                                            \
    namespace internal {                                                        \
    inline static InstanceClassInitializer<T> _instance_initializer_##p_type;   \
    }

class InstanceMethods {
    HashMap<LuaString, LuaFunction, LuaStringHasher> methods;
public:
    InstanceMethods& register_method(const char* p_name, lua_CFunction p_method) {
        methods[p_name] = LuaFunction(p_method, p_name);
        return *this;
    }
    InstanceMethods& register_method(const char* p_name, lua_CFunction p_method, lua_Continuation p_cont) {
        methods[p_name] = LuaFunction(p_method, p_name, p_cont);
        return *this;
    }
    InstanceMethods& register_method(const char* p_name, const LuaFunction& p_method) {
        methods[p_name] = p_method;
        return *this;
    }
    Option<LuaFunction> get(LuaString p_name) {
        auto it = methods.find(p_name);
        if (it != methods.end()) 
            return it->value;
        else
            return nullptr;
    }

    Option<LuaFunction> operator[](LuaString p_name) const {
        auto it = methods.find(p_name);
        if (it != methods.end()) 
            return it->value;
        else
            return nullptr;
    }
};

} // namespace gdrblx

#endif