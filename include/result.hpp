#ifndef RESULT_H_
#define RESULT_H_

#include <cstdlib>
#include <iostream>

#include "error.hpp"
#include "option.hpp"
#include "cursed_macros.h"
#include "datatype.hpp"

namespace result {
    enum Tag { Ok, Err };
    template<typename T, typename E>
    class Result {
        Result() = delete;
    public:
        union {
            T Ok;
            E Err;
        };
        enum Tag tag;
        Result(T ok) : tag(Tag::Ok), Ok(ok) {}
        Result(E err) : tag(Tag::Err), Err(err) {}
        Result(const Result &res) : tag(res.tag) {
            switch (res.tag) {
                case Tag::Ok:
                    new (&this->Ok) T(res.Ok);
                    break;
                case Tag::Err:
                    new (&this->Err) E(res.Err);
                    break;
            }
        }
        ~Result() {
            if (this->is_ok())
                this->Ok.~T();
            else
                this->Err.~E();
        }
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
        option::Option<T> ok() {
            if (this->is_ok()) return this->Ok;
            return None;
        }
        option::Option<E> err() {
            if (this->is_err()) return this->Err;
            return None;
        }
    };
}
using enum result::Tag;

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                            \
    ({                                                       \
        auto ___temp = EXPR;                                 \
        if (___temp.is_err()) return ___temp.err().unwrap(); \
        ___temp.unwrap();                                    \
    })

#endif // !RESULT_H_
