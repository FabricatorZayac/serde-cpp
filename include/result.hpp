#ifndef RESULT_H_
#define RESULT_H_

#include <cstdlib>
#include <iostream>

#include "option.hpp"
#include "cursed_macros.h"
#include "datatype.hpp"

namespace result {
    enum Tag { Ok, Err };
    template<typename T, typename E>
    class Result {
        Result();
    public:
        union Body {
            T Ok;
            E Err;
            Body(){}
        } body;
        enum Tag tag;
        Result(T Ok) {
            this->tag = Tag::Ok;
            this->body.Ok = Ok;
        }
        Result(E Err) {
            this->tag = Tag::Err;
            this->body.Err = Err;
        }
        ~Result() {
            // replace with pattern matching
            if (this->is_ok()) {
                this->body.Ok.~T();
            }
            this->body.Err.~E();
        }
        bool is_ok() {
            return this->tag == Tag::Ok ? true : false;
        }
        bool is_err() {
            return this->tag == Tag::Err ? true : false;
        }
        T unwrap() {
            if (this->is_ok()) {
                return this->body.Ok;
            }
            exit(1);
        }
        option::Option<T> ok() {
            if (this->is_ok()) return this->body.Ok;
            return None;
        }
        option::Option<E> err() {
            if (this->is_err()) return this->body.Err;
            return None;
        }
    };
}
using enum result::Tag;

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(res)                                     \
    ({                                               \
        if (res.is_err()) return res.err().unwrap(); \
        res.unwrap();                                \
    })

#endif // !RESULT_H_
