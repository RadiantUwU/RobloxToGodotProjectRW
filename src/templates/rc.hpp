#ifndef RC_HPP
#define RC_HPP

#include <cstddef>
#include <type_traits>

#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/classes/rw_lock.hpp>
#include <godot_cpp/classes/mutex.hpp>

namespace gdrblx {

namespace internal {
template <typename A, typename B>
concept RcCanCastBetween = requires(A* ptr) {(B*)ptr;};
}

class KnowsRcSelf;
class KnowsArcSelf;

template <typename T>
class Rc;
template <typename T>
class Arc;

namespace internal {

template <typename T>
class ReadGuard;
template <typename T>
class WriteGuard;

template <typename T>
class RcHeader {
public:
    friend class ::gdrblx::KnowsRcSelf;
    RcHeader() : object((T*)((size_t)this+sizeof(RcHeader<T>))) {}
    virtual ~RcHeader() {
        object->~T();
    }
private:
    friend class ::gdrblx::Rc<T>;
    mutable size_t ref_count = 1;
    T *const object;
};

template <typename T>
class ArcHeader {
    friend class ::gdrblx::KnowsArcSelf;
    friend class ::gdrblx::Arc<T>;
    friend class ReadGuard<T>;
    friend class WriteGuard<T>;
public:
    ArcHeader() : object((T*)((size_t)this+sizeof(ArcHeader<T>))) {}
    virtual ~ArcHeader() {
        object->~T();
    }
private:
    mutable size_t ref_count = 1;
    ::godot::RWLock rwlock;
    mutable ::godot::Mutex mtx;
    T *const object;
};

template <typename T>
class ReadGuard {
    friend class ::gdrblx::Arc<T>;
    size_t offset = 0;
    ArcHeader<T> *const header;
    ReadGuard(ArcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
        header->rwlock.read_lock();
    }
public:
    ~ReadGuard() {
        header->rwlock.read_unlock();
        header->mtx.lock();
        if (--header->ref_count == 0) {
            header->mtx.unlock();
            ::godot::memdelete(header);
        }
        header->mtx.unlock();
    }
    const T* operator->() const {
        return (const T*)(((size_t)header->object)+offset);
    }
    operator const T*() const {
        return (const T*)(((size_t)header->object)+offset);
    }
    operator const T&() const {
        return *(const T*)(((size_t)header->object)+offset);
    }
};
template <typename T>
class WriteGuard {
    friend class ::gdrblx::Arc<T>;
    size_t offset = 0;
    ArcHeader<T> *const header;
    WriteGuard(ArcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
        header->rwlock.read_lock();
    }
public:
    ~WriteGuard() {
        header->rwlock.write_unlock();
        header->mtx.lock();
        if (--header->ref_count == 0) {
            header->mtx.unlock();
            ::godot::memdelete(header);
        }
        header->mtx.unlock();
    }
    T* operator->() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T*() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T&() const {
        return *(T*)(((size_t)header->object)+offset);
    }
};

} // namespace internal

template <typename T>
class Rc {
    friend class KnowsRcSelf;
    internal::RcHeader<T> *header;
    size_t offset = 0;
    Rc(internal::RcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {}
public:
    Rc() : header(new (memalloc(sizeof(internal::RcHeader<T>)+sizeof(T))) internal::RcHeader<T>()) {
        new (header->object) T();
    }
    Rc(const Rc<T>& p_other) : header(p_other.header), offset(p_other.offset) {
        header->ref_count++;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Rc(const Rc<DT>& p_other) : header(p_other.header), offset(((size_t)(T*)p_other.header->object) - ((size_t)p_other.header->object)+p_other.offset) {
        header->ref_count++;
    }
    template <typename... Args>
    Rc(Args... p_args) : header(new (memalloc(sizeof(internal::RcHeader<T>)+sizeof(T))) internal::RcHeader<T>()) {
        new (header->object) T(p_args...);
    }
    ~Rc() {
        if (--header->ref_count == 0)
            ::godot::memdelete(header);
    }
    T* operator->() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T*() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T&() const {
        return *(T*)(((size_t)header->object)+offset);
    }
    Rc<T>& operator=(const Rc<T>& p_other) {
        this->~Rc();
        new (this) Rc(p_other);
        return *this;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Rc<T>& operator=(const Rc<DT>& p_other) {
        this->~Rc();
        new (this) Rc(p_other);
        return *this;
    }

    bool operator==(const Rc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return true;
        return (T&)p_other == (T&)*this;
    }
    bool operator!=(const Rc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return false;
        return (T&)p_other != (T&)*this;
    }
    bool operator==(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return true;
        return p_other == (T&)*this;
    }
    bool operator!=(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return false;
        return p_other != (T&)*this;
    }
};

template <typename T>
class Arc {
    internal::ArcHeader<T> *const header;
    size_t offset = 0;
    Arc(internal::ArcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {}
public:
    Arc() : header(new (memalloc(sizeof(internal::ArcHeader<T>)+sizeof(T))) internal::ArcHeader<T>()) {
        new (header->object) T();
    }
    Arc(const Arc<T>& p_other) : header(p_other.header), offset(p_other.offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
    }
    template <typename... Args>
    Arc(Args... p_args) : header(new (memalloc(sizeof(internal::ArcHeader<T>)+sizeof(T))) internal::ArcHeader<T>()) {
        new (header->object) T(p_args...);
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Arc(const Rc<DT>& p_other) : header(p_other.header), offset(((size_t)(T*)p_other.header->object) - ((size_t)p_other.header->object)+p_other.offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
    }
    ~Arc() {
        header->mtx.lock();
        if (--header->ref_count == 0) {
            header->mtx.unlock();
            ::godot::memdelete(header);
        }
        header->mtx.unlock();
    }
    internal::WriteGuard<T> operator->() {
        return internal::WriteGuard<T>(header, offset);
    }
    operator internal::WriteGuard<T>() {
        return internal::WriteGuard<T>(header, offset);
    }
    internal::ReadGuard<T> operator->() const {
        return internal::ReadGuard<T>(header, offset);
    }
    operator internal::ReadGuard<T>() const {
        return internal::ReadGuard<T>(header, offset);
    }

    internal::WriteGuard<T> write() {
        return internal::WriteGuard<T>(header, offset);
    }
    internal::ReadGuard<T> read() const {
        return internal::ReadGuard<T>(header, offset);
    }
    T& unsafe_access() const {
        return *(T*)(((size_t)header->object)+offset);
    }

    Arc<T>& operator=(const Arc<T>& p_other) {
        this->~Arc();
        new (this) Arc(p_other);
        return *this;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Arc<T>& operator=(const Arc<DT>& p_other) {
        this->~Arc();
        new (this) Arc(p_other);
        return *this;
    }

    bool operator==(const Arc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return true;
        return (T&)p_other == (T&)*this;
    }
    bool operator!=(const Arc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return false;
        return (T&)p_other != (T&)*this;
    }
    bool operator==(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return true;
        return p_other == (T&)*this;
    }
    bool operator!=(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return false;
        return p_other != (T&)*this;
    }
};

class KnowsRcSelf {
    template <typename T>
    friend class Rc;
    internal::RcHeader<void*> *header_ptr;
protected:
    template <class T>
    Rc<T> get_rc(const T* p_this) const {
        const size_t offset = ((size_t)p_this - (size_t)header_ptr->object);
        return Rc<T>((internal::RcHeader<T>*)(void*)header_ptr, offset);
    }
};
class KnowsArcSelf {
    template <typename T>
    friend class Arc;
    internal::ArcHeader<void*> *header_ptr;
protected:
    template <class T>
    Arc<T> get_arc(const T* p_this) const {
        const size_t offset = ((size_t)p_this - (size_t)header_ptr->object);
        return Arc<T>((internal::ArcHeader<T>*)(void*)header_ptr, offset);
    }
};

template <typename T> requires internal::RcCanCastBetween<T, KnowsRcSelf>
class Rc<T> {
    friend class KnowsRcSelf;
    internal::RcHeader<T> *header;
    size_t offset = 0;
    Rc(internal::RcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {}
public:
    Rc() : header(new (memalloc(sizeof(internal::RcHeader<T>)+sizeof(T))) internal::RcHeader<T>()) {
        new (header->object) T();
        ((KnowsRcSelf*)header->object)->header_ptr = (internal::RcHeader<void*>*)(void*)header;
    }
    Rc(const Rc<T>& p_other) : header(p_other.header), offset(p_other.offset) {
        header->ref_count++;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Rc(const Rc<DT>& p_other) : header(p_other.header), offset(((size_t)(T*)p_other.header->object) - ((size_t)p_other.header->object)+p_other.offset) {
        header->ref_count++;
    }
    template <typename... Args>
    Rc(Args... p_args) : header(new (memalloc(sizeof(internal::RcHeader<T>)+sizeof(T))) internal::RcHeader<T>()) {
        new (header->object) T(p_args...);
        ((KnowsRcSelf*)header->object)->header_ptr = (internal::RcHeader<void*>*)(void*)header;
    }
    ~Rc() {
        if (--header->ref_count == 0)
            ::godot::memdelete(header);
    }
    T* operator->() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T*() const {
        return (T*)(((size_t)header->object)+offset);
    }
    operator T&() const {
        return *(T*)(((size_t)header->object)+offset);
    }
    Rc<T>& operator=(const Rc<T>& p_other) {
        this->~Rc();
        new (this) Rc(p_other);
        return *this;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Rc<T>& operator=(const Rc<DT>& p_other) {
        this->~Rc();
        new (this) Rc(p_other);
        return *this;
    }
    bool operator==(const Rc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return true;
        return (T&)p_other == (T&)*this;
    }
    bool operator!=(const Rc<T>& p_other) const {
        if ((T*)p_other == (T*)*this) 
            return false;
        return (T&)p_other != (T&)*this;
    }
    bool operator==(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return true;
        return p_other == (T&)*this;
    }
    bool operator!=(const T& p_other) const {
        if (&p_other == (T*)*this) 
            return false;
        return p_other != (T&)*this;
    }
};

template <typename T> requires internal::RcCanCastBetween<T, KnowsArcSelf>
class Arc<T> {
    friend class KnowsArcSelf;
    internal::ArcHeader<T> *const header;
    size_t offset = 0;
    Arc(internal::ArcHeader<T> *p_header, size_t p_offset) : header(p_header), offset(p_offset) {}
public:
    Arc() : header(new (memalloc(sizeof(internal::ArcHeader<T>)+sizeof(T))) internal::ArcHeader<T>()) {
        new (header->object) T();
        ((KnowsArcSelf*)header->object)->header_ptr = (internal::ArcHeader<void*>*)(void*)header;
    }
    Arc(const Arc<T>& p_other) : header(p_other.header), offset(p_other.offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
    }
    template <typename... Args>
    Arc(Args... p_args) : header(new (memalloc(sizeof(internal::ArcHeader<T>)+sizeof(T))) internal::ArcHeader<T>()) {
        new (header->object) T(p_args...);
        ((KnowsArcSelf*)header->object)->header_ptr = (internal::ArcHeader<void*>*)(void*)header;
    }
public:
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Arc(const Rc<DT>& p_other) : header(p_other.header), offset(((size_t)(T*)p_other.header->object) - ((size_t)p_other.header->object)+p_other.offset) {
        header->mtx.lock();
        header->ref_count++;
        header->mtx.unlock();
    }
    ~Arc() {
        header->mtx.lock();
        if (--header->ref_count == 0) {
            header->mtx.unlock();
            ::godot::memdelete(header);
        }
        header->mtx.unlock();
    }
    internal::WriteGuard<T> operator->() {
        return internal::WriteGuard<T>(header, offset);
    }
    operator internal::WriteGuard<T>() {
        return internal::WriteGuard<T>(header, offset);
    }
    internal::ReadGuard<T> operator->() const {
        return internal::ReadGuard<T>(header, offset);
    }
    operator internal::ReadGuard<T>() const {
        return internal::ReadGuard<T>(header, offset);
    }

    internal::WriteGuard<T> write() {
        return internal::WriteGuard<T>(header, offset);
    }
    internal::ReadGuard<T> read() const {
        return internal::ReadGuard<T>(header, offset);
    }
    T& unsafe_access() const {
        return *(T*)(((size_t)header->object)+offset);
    }

    Arc<T>& operator=(const Arc<T>& p_other) {
        this->~Arc();
        new (this) Arc(p_other);
        return *this;
    }
    template <typename DT> requires internal::RcCanCastBetween<DT, T>
    Arc<T>& operator=(const Arc<DT>& p_other) {
        this->~Arc();
        new (this) Arc(p_other);
        return *this;
    }

    bool operator==(const Arc<T>& p_other) const {
        if (&p_other.unsafe_access() == (T*)&unsafe_access()) 
            return true;
        return (T&)p_other == (T&)unsafe_access();
    }
    bool operator!=(const Arc<T>& p_other) const {
        if (&p_other.unsafe_access() == (T*)&unsafe_access()) 
            return false;
        return (T&)p_other != (T&)unsafe_access();
    }
    bool operator==(const T& p_other) const {
        if (&p_other.unsafe_access() == (T*)&unsafe_access()) 
            return true;
        return p_other == (T&)unsafe_access();
    }
    bool operator!=(const T& p_other) const {
        if (&p_other.unsafe_access() == (T*)&unsafe_access()) 
            return false;
        return p_other != (T&)unsafe_access();
    }
};

} // namespace gdrblx
#endif