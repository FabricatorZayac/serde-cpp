#ifndef RESULT_H_
#define RESULT_H_

#include <cstdlib>

#include "error.hpp"
#include "option.hpp"
#include "cursed_macros.h"
#include "datatype.hpp"

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
}

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                                 \
    ({                                                            \
        auto ___temp = EXPR;                                      \
        if (___temp.is_err()) return Err(___temp.unwrap_err()); \
        ___temp.unwrap();                                         \
    })

#endif // !RESULT_H_
