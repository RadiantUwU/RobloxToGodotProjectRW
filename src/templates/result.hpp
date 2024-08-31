#ifndef RESULT_HPP
#define RESULT_HPP

#include <godot_cpp/core/error_macros.hpp>

#include "property.hpp"

namespace gdrblx {

template<typename T, typename E>
class Result {
    union {
        T result;
        E error;
    };
    const bool pv_success;
    Result(T p_Result) : result(p_Result), pv_success(true) {}
    Result(E p_Error, const int& _) : error(p_Error), pv_success(false) {}
public:
    ~Result() {
        if (pv_success)
            result.~T();
        else 
            error.~E();
    }
    PROPERTY_READONLY_PROXY(bool, success, Result, pv_success);

    static Result create_error(const E& p_Error) {
        return Result(p_Error, 0);
    }
    static Result create_result(const T& p_Result) {
        return Result(p_Result);
    }

    bool is_ok() const {
        return pv_success;
    }
    bool is_err() const {
        return !pv_success;
    }
    T& get_result() {
        DEV_ASSERT(pv_success);
        return result;
    }
    E& get_error() {
        DEV_ASSERT(pv_success);
        return error;
    }
    T unwrap() && {
        DEV_ASSERT(pv_success);
        return std::move(result);
    }
    
};
template<typename T>
class Result<T, void> {
    union {
        T result;
        bool _f = false;
    };
    bool pv_success;
    Result(T p_Result) : result(p_Result), pv_success(true) {}
    Result() : pv_success(false) {}
public:
    ~Result() {
        if (pv_success)
            result.~T();
    }
    PROPERTY_READONLY_PROXY(bool, success, Result, pv_success);


    static Result create_error() {
        return Result();
    }
    static Result create_result(const T& p_Result) {
        return Result(p_Result);
    }

    bool is_ok() const {
        return pv_success;
    }
    bool is_err() const {
        return !pv_success;
    }
    T& get_result() {
        DEV_ASSERT(pv_success);
        return result;
    }
    T unwrap() && {
        DEV_ASSERT(pv_success);
        return std::move(result);
    }
    
};
} //namespace gdrblx

#endif