#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstddef>
#include <cstring>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/math.hpp>

#include "macros.hpp"
#include "string.hpp"

namespace gdrblx {

class LuaBuffer {
    uint8_t *data = nullptr;
    size_t size = 0;
public:
    GDRBLX_INLINE LuaBuffer() {}
    GDRBLX_INLINE LuaBuffer(size_t p_size) : size(p_size) {
        data = (uint8_t*)memalloc(p_size+8);
        memset(data, 0, p_size+8);
    }
    GDRBLX_INLINE LuaBuffer(const LuaBuffer& p_buf) : size(p_buf.size) {
        data = (uint8_t*)memalloc(size+8);
        memcpy(data, p_buf.data, p_buf.size+8);
    }
    GDRBLX_INLINE ~LuaBuffer() {
        memfree(data);
    }

    GDRBLX_INLINE LuaBuffer(const LuaString& p_str) : size(p_str.l) {
        data = (uint8_t*)memalloc(size+8);
        memcpy(data, p_str.s, size);
        memset(data+size, 0, 8);
    }
    GDRBLX_INLINE operator LuaString() const {
        return LuaString((const char*)(void*)data, size);
    }

    GDRBLX_INLINE size_t len() const { return size; }

    GDRBLX_INLINE int8_t readi8(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(int8_t*)(data+p_offset);
    }
    GDRBLX_INLINE uint8_t readu8(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(data+p_offset);
    }
    GDRBLX_INLINE int16_t readi16(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(int16_t*)(data+p_offset);
    }
    GDRBLX_INLINE uint16_t readu16(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(uint16_t*)(data+p_offset);
    }
    GDRBLX_INLINE int32_t readi32(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(int32_t*)(data+p_offset);
    }
    GDRBLX_INLINE uint32_t readu32(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(uint32_t*)(data+p_offset);
    }
    GDRBLX_INLINE int64_t readi64(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(int64_t*)(data+p_offset);
    }
    GDRBLX_INLINE uint64_t readu64(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(uint64_t*)(data+p_offset);
    }
    GDRBLX_INLINE float readf32(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(float*)(data+p_offset);
    }
    GDRBLX_INLINE double readf64(size_t p_offset) const {
        DEV_ASSERT(p_offset < size);
        return *(double*)(data+p_offset);
    }

    GDRBLX_INLINE void writei8(size_t p_offset, int8_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(int8_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writeu8(size_t p_offset, uint8_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writei16(size_t p_offset, int16_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(int16_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writeu16(size_t p_offset, uint16_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(uint16_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writei32(size_t p_offset, int32_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(int32_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writeu32(size_t p_offset, uint32_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(uint32_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writei64(size_t p_offset, int64_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(int64_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writeu64(size_t p_offset, uint64_t p_val) {
        DEV_ASSERT(p_offset < size);
        *(uint64_t*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writef32(size_t p_offset, float p_val) {
        DEV_ASSERT(p_offset < size);
        *(float*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE void writef64(size_t p_offset, double p_val) {
        DEV_ASSERT(p_offset < size);
        *(double*)(data+p_offset) = p_val;
    }
    GDRBLX_INLINE LuaString readstring(size_t p_offset, size_t p_length) const {
        size_t str_size = MAX(0,MIN(p_length,(int64_t)size-p_offset));
        if (str_size == 0) {
            return LuaString();
        }
        return LuaString(data+p_offset,str_size);
    }
    GDRBLX_INLINE void writestring(size_t p_offset, const LuaString& p_string) {
        size_t str_size = MAX(0,MIN(p_length,(int64_t)size-p_offset-p_string.l));
        if (str_size == 0)
            return;
        memcpy(data+p_offset,p_string.s,str_size);
    }
    GDRBLX_INLINE void writestring(size_t p_offset, const LuaString& p_string, size_t p_count) {
        DEV_ASSERT(p_string.l >= p_count);
        size_t str_size = MAX(0,MIN(MIN(p_count,p_string.l),(int64_t)size-p_offset));
        if (str_size == 0)
            return;
        memcpy(data+p_offset,p_string.s,str_size);
    }
    GDRBLX_INLINE void copy(size_t p_dest_offset, const LuaBuffer& p_src, size_t p_src_offset = 0) {
        size_t buf_size = MAX(0, MIN((int64_t)size-p_dest_offset,(int64_t)p_src.size-p_src_offset));
        if (buf_size == 0)
            return;
        memcpy(data+p_dest_offset,p_src.data+p_src_offset,buf_size);
    }
    GDRBLX_INLINE void copy(size_t p_dest_offset, const LuaBuffer& p_src, size_t p_src_offset, size_t p_count) {
        size_t buf_size = MAX(0, MIN(MIN((int64_t)size-p_dest_offset,(int64_t)p_src.size-p_src_offset),p_count));
        if (buf_size == 0)
            return;
        memcpy(data+p_dest_offset,p_src.data+p_src_offset,buf_size);
    }
    GDRBLX_INLINE void fill(size_t p_offset, uint8_t p_value) {
        size_t buf_size = MAX(0,(int64_t)size-p_offset);
        if (buf_size == 0)
            return;
        memset(data, p_value, buf_size);
    }
    GDRBLX_INLINE void fill(size_t p_offset, uint8_t p_value, size_t p_count) {
        size_t buf_size = MAX(0,MIN(p_count,(int64_t)size-p_offset));
        if (buf_size == 0)
            return;
        memset(data, p_value, buf_size);
    }
};

} // namespace gdrblx
#endif