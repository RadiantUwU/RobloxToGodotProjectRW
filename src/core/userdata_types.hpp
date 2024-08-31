#ifndef USERDATA_TYPES_HPP
#define USERDATA_TYPES_HPP

#include <godot_cpp/core/error_macros.hpp>
#include <type_traits>

#include "macros.hpp"

namespace gdrblx {
enum UserdataType : char {
    UD_INVALID,
    UD_INSTANCE,
    UD_RBXSCRIPTSIGNAL,
    UD_RBXSCRIPTCONNECTION,
    UD_SHAREDTABLE
};

namespace internal {

class LuaUserdataBase {
protected:
    short userdata_flags = 0;

    static constexpr short UD_MATH = 0x1;
    static constexpr short UD_INDEX = 0x2;
    static constexpr short UD_INDEXSET = 0x4;
    static constexpr short UD_STRING = 0x8;
    static constexpr short UD_ITER = 0x10;
    static constexpr short UD_CALL = 0x20;
    static constexpr short UD_INIT = 0x40;
    static constexpr short UD_LENGTH = 0x80;
public:
    GDRBLX_INLINE char get_userdata_flags() const { return userdata_flags; };
    virtual UserdataType get_userdata_type() const { CRASH_NOW(); return UD_INVALID; };
    static const char* get_type_to_string(UserdataType p_type) {
        switch (p_type) {
            case UD_INVALID:
                return "<invalid>";
            case UD_INSTANCE:
                return "Instance";
            case UD_RBXSCRIPTSIGNAL:
                return "RBXScriptSignal";
            case UD_RBXSCRIPTCONNECTION:
                return "RBXScriptConnection";
            case UD_SHAREDTABLE:
                return "SharedTable";
        }
    }
};

}; // gdrblx::internal

template<class T>
concept IsUserdata = std::is_base_of_v<internal::LuaUserdataBase, T>;

} // namespace gdrblx

#endif // USERDATA_TYPES_HPP