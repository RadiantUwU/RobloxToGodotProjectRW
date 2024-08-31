#ifndef STRING_HPP
#define STRING_HPP

#include <cstdint>
#include <cstring>

#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/char_string.hpp>

#include "macros.hpp"

namespace gdrblx {

class LuaString {
public:
    char *s;
    int l;
    LuaString(::godot::String p_s) {
        ::godot::CharString c = p_s.ascii();
        l = c.size();
        this->s = (char*)memalloc((l+1)*sizeof(char));
        if (c.ptr() == nullptr) {
            this->s[0] = '\0';
        } else {
            memcpy(this->s,c.ptr(),(l+1)*sizeof(char));
        }
    }
    LuaString() {
        s = nullptr;
        l = 0;
    }
    LuaString(std::nullptr_t) {
        s = nullptr;
        l = 0;
    }
    explicit LuaString(int p_len) {
        l = p_len;
        s = (char*)memalloc((l+1)*sizeof(char));
    }
    LuaString(const char* p_cs) {
        auto slen = strlen(p_cs);
        l = slen;
        s = (char*)memalloc((l+1)*sizeof(char));
        if (p_cs == nullptr) {
            s[0] = 0;
        } else {
            strcpy(s, p_cs);
        }
    }
    LuaString(const char* p_cs, size_t len) {
        l = len;
        s = (char*)memalloc((l+1)*sizeof(char));
        if (p_cs == nullptr) {
            s[0] = 0;
        } else {
            memcpy(s,p_cs,(l+1)*sizeof(char));
        }
    }
    LuaString(const LuaString& p_o) {
        l = p_o.l;
        s = (char*)memalloc((l+1)*sizeof(char));
        if (p_o.s == nullptr) {
            s[0] = 0;
        } else {
            memcpy(s,p_o.s,(l+1)*sizeof(char));
        }
    }
    ~LuaString() {
        if (s != nullptr) memfree(s);
    }
    LuaString& operator=(const LuaString& p_o) {
        l = p_o.l;
        s = (char*)memalloc((l+1)*sizeof(char));
        if (p_o.s == nullptr) {
            s[0] = 0;
        } else {
            memcpy(s,p_o.s,(l+1)*sizeof(char));
        }
        return *this;
    }
    operator const char* () const {
        return s;
    }
    bool operator==(std::nullptr_t) const {
        return s == nullptr;
    }
    bool operator==(const char* p_s) const {
        return strcmp(p_s, this->s) == 0;
    }
    bool operator==(const LuaString& p_o) const {
        return p_o.l==l and memcmp(p_o.s, s, l) == 0;
    }
    bool operator==(const LuaString&& p_o) const {
        return p_o.l==l and memcmp(p_o.s, s, l) == 0;
    }
    bool operator!=(std::nullptr_t) const {
        return s != nullptr;
    }
    bool operator!=(const char* p_s) const {
        return strcmp(p_s, this->s) != 0;
    }
    bool operator!=(const LuaString& p_o) const {
        return p_o.l!=l or memcmp(p_o.s, s, l) != 0;
    }
    bool operator!=(const LuaString&& p_o) const {
        return p_o.l!=l or memcmp(p_o.s, s, l) != 0;
    }

    LuaString operator+(const LuaString& p_o) const {
        LuaString str = LuaString(l+p_o.l);
        memcpy(str.s,s,l);
        memcpy(str.s+l,p_o.s,p_o.l);
        return str;
    }
}; // struct LuaString

class LuaStringHasher {
public:
    GDRBLX_INLINE static uint32_t hash(const LuaString& p_s) {
        return ::godot::hash_djb2_buffer((const uint8_t*)p_s.s,sizeof(char)*p_s.l);
    }
}; // class LuaStringHasher

}; // namespace gdrblx

#endif // STRING_HPP