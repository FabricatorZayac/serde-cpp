#ifndef RESULT_H_
#define RESULT_H_

#include <cstdlib>

#include "error.hpp"
#include "option.hpp"
#include "cursed_macros.h"
#include "datatype_macros.hpp"

namespace result {
    DATA_INITIALIZER_TYPES((Ok, T), (Err, E));

    template<typename T, typename E>
    struct Result {
        data(Result, (Ok, T), (Err, E));
        bool is_ok() {
            return this->tag == Tag::Ok ? true : false;
        }
        bool is_err() {
            return this->tag == Tag::Err ? true : false;
        }
        T unwrap() {
            if (this->is_ok()) {
                return this->Ok_val;
            }
            exit(1);
        }
        E unwrap_err() {
            if (this->is_err()) {
                return this->Err_val;
            }
            exit(1);
        }
        option::Option<T> ok() {
            if (this->is_ok()) return option::Some(this->Ok_val);
            return option::None();
        }
        option::Option<E> err() {
            if (this->is_err()) return option::Some(this->Err_val);
            return option::None();
        }
    };

    template<>
    struct Ok<void> {
        DATA_INITIALIZER_TYPE_IMPL(Ok)
    };

    Ok() -> Ok<void>;

    template<typename E>
    struct Result<void, E> {
        Result(Ok<void> &&Ok) : tag(Tag ::Ok) {}
        Result(Err<E> &&Err) : tag(Tag ::Err), Err_val(Err.Err_value) {}
        Result(const Result &source) : tag(source.tag) {
            switch (source.tag) {
            case Tag ::Ok:
                break;
            case Tag ::Err:
                new (&this->Err) E(source.Err_val);
                break;
            }
        }
        ~Result() {
            switch (this->tag) {
            case Tag ::Ok:
                break;
            case Tag ::Err:
                this->Err_val.~E();
                break;
            }
        }
        enum class Tag {
          Ok,
          Err,
        } tag;
        union {
            E Err_val;
        };
        bool is_ok() {
            return this->tag == Tag::Ok ? true : false;
        }
        bool is_err() {
            return this->tag == Tag::Err ? true : false;
        }
        void unwrap() {
            if (this->is_ok()) {
                return;
            }
            exit(1);
        }
        E unwrap_err() {
            if (this->is_err()) {
                return this->Err_val;
            }
            exit(1);
        }
        // option::Option<void> ok() {
        //     if (this->is_ok()) return option::Some(this->Ok_val);
        //     return option::None();
        // }
        option::Option<E> err() {
            if (this->is_err()) return option::Some(this->Err_val);
            return option::None();
        }
    };
}

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                               \
    ({                                                          \
        auto ___temp = EXPR;                                    \
        if (___temp.is_err()) return Err(___temp.unwrap_err()); \
        ___temp.unwrap();                                       \
    })

#endif // !RESULT_H_
