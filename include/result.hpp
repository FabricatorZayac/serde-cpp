#ifndef RESULT_H_
#define RESULT_H_

#include <cstdlib>
#include <assert.h>
#include <iostream>

#include "error.hpp"
#include "option.hpp"
#include "cursed_macros.h"
#include "datatype.hpp"

namespace result {
    enum class Tag { Ok, Err };

    template<typename T>
    struct Ok {
        const T ok_value;

        Ok() = default;
        ~Ok() = default;

        Ok(Ok&&) = delete;
        Ok(const Ok&) = delete;

        Ok(const T &value) : ok_value(value) {}
        template <typename... Args>
        Ok(Args&&... args) : ok_value(static_cast<Args&&>(args)...) { }
    };

    template<typename E>
    struct Err {
        E Err_value;

        Err() = default;
        ~Err() = default;

        Err(Err&&) = delete;
        Err(const Err&) = delete;

        Err(const E &value) : Err_value(value) {}
    };

    template<typename T, typename E>
    struct Result {
        data(Result, (Ok, T), (Err, E))

        Result(const enum Tag tag, T Ok) {
            assert(tag == Tag::Ok);
            this->tag = Tag::Ok;
            new(&this->Ok) T(Ok);
        }
        Result(const enum Tag tag, E Err) {
            assert(tag == Tag::Err);
            this->tag = Tag::Err;
            new(&this->Err) E(Err);
        }

        template<typename T1 = T>
        Result(result::Ok<T1> &&Ok) : tag(Tag::Ok), Ok(static_cast<T1>(Ok.ok_value)) {}

        bool is_ok() {
            return this->tag == Tag::Ok ? true : false;
        }
        bool is_err() {
            return this->tag == Tag::Err ? true : false;
        }
        T unwrap() {
            if (this->is_ok()) {
                return this->Ok;
            }
            exit(1);
        }
        E unwrap_err() {
            if (this->is_err()) {
                return this->Err;
            }
            exit(1);
        }
        // option::Option<T> ok() {
        //     if (this->is_ok()) return {Some, this->Ok};
        //     return None;
        // }
        // option::Option<E> err() {
        //     if (this->is_err()) return {Some, this->Err};
        //     return None;
        // }
    };
}
using enum result::Tag;

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                                 \
    ({                                                            \
        auto ___temp = EXPR;                                      \
        if (___temp.is_err()) return {Err, ___temp.unwrap_err()}; \
        ___temp.unwrap();                                         \
    })

#endif // !RESULT_H_
