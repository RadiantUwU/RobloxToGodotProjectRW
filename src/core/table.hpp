#ifndef TABLE_HPP
#define TABLE_HPP

#include <utility>
#include <shared_mutex>

#include <lua.h>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "macros.hpp"
#include "object.hpp"

namespace gdrblx {

namespace internal {

class LuaTableProperty final {
    friend class ::gdrblx::LuaTable;
    LuaTable *tbl;
    ::gdrblx::LuaObject key;

    GDRBLX_INLINE LuaTableProperty(LuaTable *p_tbl) : tbl(p_tbl) {}

    GDRBLX_INLINE LuaTableProperty& assign(::gdrblx::LuaObject p_key) & {
        key = p_key;
        return *this;
    }
    LuaTableProperty(const LuaTableProperty&& p_other) : tbl(p_other.tbl), key(p_other.key) {}

public:

    operator LuaObject();
    void operator=(const LuaObject& p_value);

    LuaTableProperty& operator=(const LuaTableProperty&) = delete;
};

} // namespace internal

class LuaTable {
    HashMap<LuaObject, LuaObject, LuaObjectHasher> map;
    internal::LuaTableProperty property;
    bool frozen = false;

    friend class LuaIpairsIterator;

public:
    LuaTable() : property(this) {}
    LuaTable(const LuaTable& p_other) : property(this), map(p_other.map), frozen(p_other.frozen) {}
    LuaTable(const LuaTable&& p_other) : property(this), map(p_other.map), frozen(p_other.frozen) {}

    struct LuaTableIteration {
        friend class LuaTable;
        friend class LuaIpairsIterator;

        const LuaObject& key;
        const LuaObject& value;

        GDRBLX_INLINE bool valid() const { return idx != 0; }

        LuaTableIteration& operator=(const LuaTableIteration& p_other) & {
            this->~LuaTableIteration();
            new (this) LuaTableIteration(p_other.key, p_other.value, p_other.idx);
            return *this;
        }
    private:
        const size_t idx;
        GDRBLX_INLINE LuaTableIteration(const LuaObject& p_key, const LuaObject& p_value, size_t p_idx)
            : key(p_key), value(p_value), idx(p_idx) {}
    };

    class LuaPairsIterator {
        friend class LuaTable;
        const LuaTable* t;
        LuaTableIteration it;
        LuaPairsIterator(const LuaTable* p_t) : t(p_t), it(p_t->next()) {}
    public:
        GDRBLX_INLINE bool valid() const { return it.valid(); }

        GDRBLX_INLINE const LuaTableIteration& operator*() const {
            return it;
        }
        GDRBLX_INLINE const LuaTableIteration* operator->() const {
            return &it;
        }
        GDRBLX_INLINE LuaPairsIterator& operator++() {
            it = t->next(it);
            return *this;
        }
    };
    class LuaIpairsIterator {
    protected:
        friend class LuaTable;
        const LuaTable* t;
        LuaTableIteration it;
        LuaIpairsIterator(const LuaTable* p_t) : t(p_t), it(LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0)) {
            auto kv = t->map.find(1);
            if (kv != t->map.end())
                it = LuaTableIteration(kv->key, kv->value, 1);
        }
        LuaIpairsIterator(const LuaTable* p_t, lua_Integer p_start) : t(p_t), it(LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0)) {
            auto kv = t->map.find(p_start);
            if (kv != t->map.end())
                it = LuaTableIteration(kv->key, kv->value, p_start);
        }
    public:
        GDRBLX_INLINE bool valid() const { return it.valid(); }

        GDRBLX_INLINE const LuaTableIteration& operator*() const {
            return it;
        }
        GDRBLX_INLINE const LuaTableIteration* operator->() const {
            return &it;
        }
        GDRBLX_INLINE virtual LuaIpairsIterator& operator++() {
            lua_Integer new_idx = (it.idx != 0) ? it.idx+1 : 0;
            if (new_idx == 0)
                it = LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0);
            else {
                auto kv = t->map.find(new_idx);
                if (kv == t->map.end())
                    it = LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0);
                else
                    it = LuaTableIteration(kv->key, kv->value, new_idx);
            }
            return *this;
        }
    };

    GDRBLX_INLINE const LuaObject& operator[](const LuaObject& p_key) const {
        return get(p_key);
    }
    GDRBLX_INLINE internal::LuaTableProperty& operator[](const LuaObject& p_key) {
        property.assign(p_key);
        return property;
    }
    GDRBLX_INLINE virtual bool has(const LuaObject& p_key) const {
        return map.has(p_key);
    }
    GDRBLX_INLINE virtual const LuaObject& get(const LuaObject& p_key) const {
        const auto iterator = map.find(p_key);

        if (iterator != map.end())
            return iterator->value;

        return NIL_OBJECT_REF;
    }
    GDRBLX_INLINE virtual void set(const LuaObject& p_key, const LuaObject& p_value) {
        DEV_ASSERT(!frozen);
        ERR_FAIL_COND(!frozen);
        map.insert(p_key,p_value);    
    }
    GDRBLX_INLINE virtual size_t size() const {
        return map.size();
    }
    GDRBLX_INLINE virtual size_t arr_len() const {
        size_t i;
        for (i=1; has(i); i++) {}
        return i-1;
    }

    GDRBLX_INLINE virtual LuaTableIteration next() const {
        const auto kv = map.begin();
        if (kv != map.end()) {
            return LuaTableIteration(kv->key, kv->value, 1);
        }
        return LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0);
    }
    GDRBLX_INLINE virtual LuaTableIteration next(const LuaObject& p_key) const {
        const auto kv = map.begin();
        size_t idx = 1;
        while (kv != map.end()) {
            if (kv->key == p_key)
                return LuaTableIteration(kv->key, kv->value, idx);
            idx++;
        }
        return LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0);
    }
    GDRBLX_INLINE virtual LuaTableIteration next(const LuaTableIteration& p_last) const {
        auto kv = map.begin();
        size_t idx;
        for (idx = 1; idx <= p_last.idx; idx++)
            ++kv;
        if ((bool)kv) 
            return LuaTableIteration(kv->key, kv->value, idx);
        return LuaTableIteration(NIL_OBJECT_REF, NIL_OBJECT_REF, 0);
    }
    GDRBLX_INLINE virtual LuaPairsIterator pairs() const {
        return LuaPairsIterator(this);
    }
    GDRBLX_INLINE virtual LuaIpairsIterator ipairs() const {
        return LuaIpairsIterator(this);
    }
    GDRBLX_INLINE virtual LuaIpairsIterator ipairs(lua_Integer p_start) const {
        return LuaIpairsIterator(this, p_start);
    }
    

    GDRBLX_INLINE virtual void clear() {
        DEV_ASSERT(!frozen);
        ERR_FAIL_COND(!frozen);
        map.clear();
    }
    GDRBLX_INLINE LuaTable clone() const {
        LuaTable t;
        for (const auto& kv : this->map) {
            t[kv.key] = kv.value;
        }
        return std::move(t);
    }
    GDRBLX_INLINE LuaString concat(LuaString p_sep, lua_Integer p_i = 1) const {
        Vec<LuaString> strings;
        LuaIpairsIterator it = ipairs(p_i);

        size_t len = 0;
        while (it.valid()) {
            LuaString str = it->value.tostring();
            len += str.l;
            strings.push_back(::std::move(str));
            ++it;
        }
        if (strings.size() != 0)
            len += p_sep.l*(strings.size()-1);
        
        LuaString final_result = LuaString(len);
        size_t offset = 0;
        for (const LuaString& s : strings) {
            if (offset == 0) {
                memcpy(final_result.s,s.s,s.l);
                offset += s.l;
            } else {
                memcpy(final_result.s+offset,p_sep.s,p_sep.l);
                offset += p_sep.l;
                memcpy(final_result.s+offset,s.s,s.l);
                offset += s.l;
            }
        }
        return final_result;
    }
    GDRBLX_INLINE LuaString concat(LuaString p_sep, lua_Integer p_i, lua_Integer p_j) const {
        Vec<LuaString> strings;
        LuaIpairsIterator it = ipairs(p_i);

        size_t len = 0;
        size_t i = p_i-1;
        while (it.valid() && i++ != p_j) {
            LuaString str = it->value.tostring();
            len += str.l;
            strings.push_back(::std::move(str));
            ++it;
        }
        if (strings.size() != 0)
            len += p_sep.l*(strings.size()-1);
        
        LuaString final_result = LuaString(len);
        size_t offset = 0;
        for (const LuaString& s : strings) {
            if (offset == 0) {
                memcpy(final_result.s,s.s,s.l);
                offset += s.l;
            } else {
                memcpy(final_result.s+offset,p_sep.s,p_sep.l);
                offset += p_sep.l;
                memcpy(final_result.s+offset,s.s,s.l);
                offset += s.l;
            }
        }
        return final_result;
    }
    GDRBLX_INLINE static LuaTable create(lua_Integer p_count, const LuaObject& p_value) {
        LuaTable t;
        t.map.reserve(p_count);
        for (int i = 1; i <= p_count; i++) {
            t[i] = p_value;
        }
        return t;
    }
    GDRBLX_INLINE const LuaObject& find(const LuaObject& p_needle, lua_Integer p_init = 1) const {
        auto it = ipairs(p_init);
        while (it.valid()) {
            if (it->value == p_needle)
                return it->key;
            ++it;
        }
        return NIL_OBJECT_REF;
    }
    template <typename Function> 
    GDRBLX_INLINE void foreach(Function p_function) const {
        auto it = pairs();
        while (it.valid()) {
            p_function(it->key, it->value);
            ++it;
        }
    }
    template <typename Function> 
    GDRBLX_INLINE void foreachi(Function p_function) const {
        auto it = ipairs();
        while (it.valid()) {
            p_function(it->key, it->value);
            ++it;
        }
    }
    GDRBLX_INLINE virtual void freeze() {
        frozen = true;
    }
    GDRBLX_INLINE virtual void unfreeze() {
        frozen = false;
    }
    GDRBLX_INLINE lua_Integer getn() const {
        return size();
    }
    GDRBLX_INLINE void insert(lua_Integer p_pos, const LuaObject& p_value) {
        lua_Integer idx = p_pos;
        LuaObject to_move = p_value;
        bool moved = false;
        while (has(idx) || !moved) {
            LuaObject m = get(idx);
            set(idx, to_move);
            idx++;
            moved = true;
            to_move = std::move(m);
        }
    }
    GDRBLX_INLINE void insert(const LuaObject& p_value) {
        set(arr_len()+1,p_value);
    }
    GDRBLX_INLINE virtual bool isfrozen() const {
        return frozen;
    }
    GDRBLX_INLINE lua_Number maxn() const {
        lua_Number n = 0;
        foreach([&n](const LuaObject& k, const LuaObject&) {
            if (k.is_type(LuaObject::NUMBER))
                n = (lua_Number)k;
        });
        return n;
    }
    GDRBLX_INLINE LuaTable& move(const LuaTable& p_src, lua_Integer p_a, lua_Integer p_b, lua_Integer p_t) {
        for (size_t src_idx = p_a; src_idx <= p_b; src_idx++) {
            set(src_idx-p_a+p_t, p_src.get(src_idx));
        }
        return *this;
    }
    GDRBLX_INLINE LuaObject remove(lua_Integer p_pos) {
        LuaObject obj;
        lua_Integer idx = p_pos;

        while (has(idx)) {
            if (idx == p_pos) {
                obj = get(idx);
            }
            set(idx,get(1+idx));
            idx++;
        }
        return obj;
    }
    GDRBLX_INLINE void sort() {
        Vec<LuaObject> vec;
        foreachi([&vec](const LuaObject& k, const LuaObject& v) {
            vec.push_back(v);
        });
        vec.sort();
        for (lua_Integer i = 0; i < vec.size(); i++)
            set(i+1, vec[i]);
    }
    template <typename Comparator>
    GDRBLX_INLINE void sort(Comparator comparator) {
        Vec<LuaObject> vec;
        foreachi([&vec](const LuaObject& k, const LuaObject& v) {
            vec.push_back(v);
        });
        vec.sort(comparator);
        for (lua_Integer i = 0; i < vec.size(); i++)
            set(i+1, vec[i]);
    }
}; // class LuaTable

class SharedTable : protected LuaTable {
    friend class LuaIpairsIteratorThreaded;
    mutable std::shared_mutex lock;

    class LuaIpairsIteratorThreaded : public LuaIpairsIterator {
        friend class SharedTable;
        LuaIpairsIteratorThreaded(const LuaTable* p_t) : LuaIpairsIterator(p_t) {}
        LuaIpairsIteratorThreaded(const LuaTable* p_t, lua_Integer p_start) : LuaIpairsIterator(p_t, p_start) {}
    public:
        GDRBLX_INLINE LuaIpairsIterator& operator++() override {
            dynamic_cast<const SharedTable*>(t)->lock.lock_shared();
            LuaIpairsIterator::operator++();
            dynamic_cast<const SharedTable*>(t)->lock.unlock_shared();
            return *this;
        }
    };

public:
    SharedTable() : LuaTable() {}
    SharedTable(const LuaTable& p_t) : LuaTable(p_t) {}
    SharedTable(const LuaTable&& p_t) : LuaTable(p_t) {}

    GDRBLX_INLINE const LuaObject& operator[](const LuaObject& p_key) const { return LuaTable::operator[](p_key); };
    GDRBLX_INLINE internal::LuaTableProperty& operator[](const LuaObject& p_key) { return LuaTable::operator[](p_key); };
    
    GDRBLX_INLINE bool has(const LuaObject& p_key) const override {
        lock.lock_shared();
        const bool b = LuaTable::has(p_key);
        lock.unlock_shared();
        return b;
    }
    GDRBLX_INLINE const LuaObject& get(const LuaObject& p_key) const override {
        lock.lock_shared();
        const LuaObject& b = LuaTable::get(p_key);
        lock.unlock_shared();
        return b;
    }
    GDRBLX_INLINE void set(const LuaObject& p_key, const LuaObject& p_value) override {
        lock.lock();
        LuaTable::set(p_key, p_value);
        lock.unlock();
    }
    GDRBLX_INLINE size_t size() const override {
        lock.lock_shared();
        const LuaObject& size = LuaTable::size();
        lock.unlock_shared();
        return size;
    }
    GDRBLX_INLINE size_t arr_len() const override {
        lock.lock_shared();
        const LuaObject& arr_len = LuaTable::arr_len();
        lock.unlock_shared();
        return arr_len;
    }

    GDRBLX_INLINE LuaTableIteration next() const override {
        lock.lock_shared();
        LuaTableIteration it = LuaTable::next();
        lock.unlock_shared();
        return std::move(it);
    }
    GDRBLX_INLINE LuaTableIteration next(const LuaObject& p_key) const override {
        lock.lock_shared();
        LuaTableIteration it = LuaTable::next(p_key);
        lock.unlock_shared();
        return std::move(it);
    }
    GDRBLX_INLINE LuaTableIteration next(const LuaTableIteration& p_last) const override {
        lock.lock_shared();
        LuaTableIteration it = LuaTable::next(p_last);
        lock.unlock_shared();
        return std::move(it);
    }
    GDRBLX_INLINE LuaPairsIterator pairs() const override {
        lock.lock_shared();
        LuaPairsIterator it = LuaTable::pairs();
        lock.unlock_shared();
        return std::move(it);
    }
    GDRBLX_INLINE LuaIpairsIterator ipairs() const override {
        lock.lock_shared();
        LuaIpairsIterator it = LuaIpairsIteratorThreaded(this);
        lock.unlock_shared();
        return std::move(it);
    }
    GDRBLX_INLINE LuaIpairsIterator ipairs(lua_Integer p_start) const override {
        lock.lock_shared();
        LuaIpairsIterator it = LuaIpairsIteratorThreaded(this, p_start);
        lock.unlock_shared();
        return std::move(it);
    }
    

    GDRBLX_INLINE void clear() override {
        lock.lock();
        LuaTable::clear();
        lock.unlock();
    }
    template <typename T = LuaTable>
    GDRBLX_INLINE T clone() const {
        static_assert(std::is_base_of_v<LuaTable, T> || std::is_same_v<LuaTable, T>, "T must be LuaTable or derived.");
        lock.lock_shared();
        T t = LuaTable::clone();
        lock.unlock_shared();
        return std::move(t);
    }
    GDRBLX_INLINE LuaString concat(LuaString p_sep, lua_Integer p_i = 1) const { return LuaTable::concat(p_sep, p_i);}
    GDRBLX_INLINE LuaString concat(LuaString p_sep, lua_Integer p_i, lua_Integer p_j) const { return LuaTable::concat(p_sep, p_i, p_j);}
    GDRBLX_INLINE static SharedTable create(lua_Integer p_count, const LuaObject& p_value) {
        return SharedTable(LuaTable::create(p_count, p_value));
    }
    GDRBLX_INLINE const LuaObject& find(const LuaObject& p_needle, lua_Integer p_init = 1) const { return LuaTable::find(p_needle, p_init); }
    template <typename Function> 
    GDRBLX_INLINE void foreach(Function p_function) const { return LuaTable::foreach(p_function); }
    template <typename Function> 
    GDRBLX_INLINE void foreachi(Function p_function) const { return LuaTable::foreachi(p_function); }
    GDRBLX_INLINE void freeze() override {
        lock.lock();
        LuaTable::freeze();
        lock.unlock();
    }
    GDRBLX_INLINE void unfreeze() override {
        lock.lock();
        LuaTable::unfreeze();
        lock.unlock();
    }
    GDRBLX_INLINE lua_Integer getn() const { return LuaTable::getn(); }
    GDRBLX_INLINE void insert(lua_Integer p_pos, const LuaObject& p_value) { return LuaTable::insert(p_pos, p_value); }
    GDRBLX_INLINE void insert(const LuaObject& p_value) {return LuaTable::insert(p_value); }
    GDRBLX_INLINE bool isfrozen() const override {
        lock.lock_shared();
        bool b = LuaTable::isfrozen();
        lock.unlock_shared();
        return b;
    }
    GDRBLX_INLINE lua_Number maxn() const { return LuaTable::maxn(); }
    GDRBLX_INLINE LuaTable& move(const LuaTable& p_src, lua_Integer p_a, lua_Integer p_b, lua_Integer p_t) { return LuaTable::move(p_src, p_a, p_b, p_t); }
    GDRBLX_INLINE LuaObject remove(lua_Integer p_pos) { return LuaTable::remove(p_pos); }
    GDRBLX_INLINE void sort() { return LuaTable::sort(); }
    template <typename Comparator>
    GDRBLX_INLINE void sort(Comparator comparator) { return LuaTable::sort(comparator); }
}; // class SharedTable

} // namespace gdrblx

#endif // TABLE_HPP