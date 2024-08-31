#ifndef LUA_TUPLE_HPP
#define LUA_TUPLE_HPP

#include "object.hpp"

namespace gdrblx {

class LuaTuple final {
    Vec<LuaObject> objects;

    template <typename T, typename... Args>
    constexpr void push_objects(T p_o, Args... p_args) {
        objects.push_back(p_o);
        push_objects(p_args...);
    }
    template <typename T>
    constexpr void push_objects(T p_o) {
        objects.push_back(p_o);
    }
    constexpr void push_objects() {}
public:
    template <typename... Args>
    LuaTuple(Args... p_args) {push_objects(p_args...);}

    LuaTuple(const LuaTuple& p_other) : objects(p_other.objects) {}
    LuaTuple(LuaTuple&& p_other) : objects(std::move(p_other.objects)) {}
    
    LuaTuple(const Vec<LuaObject>& p_objects) : objects(p_objects) {}
    LuaTuple(Vec<LuaObject>&& p_objects) : objects(p_objects) {}

    size_t get_size() const {
        return objects.size();
    }
    const Vec<LuaObject>& get_objects() const & {
        return objects;
    }
    Vec<LuaObject> get_objects() && {
        return std::move(objects);
    }
    const LuaObject& get(size_t idx) const & {
        return objects.get(idx-1);
    }
    const LuaObject& operator[](size_t idx) const & {
        return objects.get(idx-1);
    }
};

}

#endif