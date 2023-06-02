#ifndef FST_H_
#define FST_H_

#include <concepts>
#include <cstdlib>

#include "cursed_macros.h"
#include "datatype_macros.hpp"

namespace fst {
    using usize = size_t;

    template<typename T, typename Ret, typename ...Args>
    concept Fn = requires(T fn, Args...args) {
        { fn(args...) } -> std::same_as<Ret>;
    };

    template<typename T>
    concept Index = requires(T indexable, usize index) {
        indexable[index];
    };

    struct str {
        const char *body;
        const usize length;
        str(const char *body, usize length);
        str(const str &);
        bool equals(const str &other);
        // Kinda shit but I don't care
        bool equals(const char *other);
        template<typename T>
        bool operator==(const T &other) {
            return this->equals(other);
        }
    };

    template<typename T, usize N>
    struct arr {
        T body[N];
        constexpr T operator[](usize idx) const {
            return body[idx];
        }
    };

    namespace error {
        class Error {
            const char *message;
        public:
            Error();
            Error(const char *message);
            const char *to_str();
        };
    };

    namespace option {
        DATA_INITIALIZER_TYPES((Some, T), (None));
        template<typename T>
        struct Option;
    };

    namespace result {
        DATA_INITIALIZER_TYPES((Ok, T), (Err, E));

        template<typename T, typename E>
        struct Result {
            DATA(Result, (Ok, T), (Err, E));
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
            DATA_INITIALIZER_TYPE_IMPL(Ok);
        };

        Ok() -> Ok<void>;

        template<typename E>
        struct Result<void, E> {
            using Ok = Ok<void>;
            DATA(Result, (Err, E), (Ok));
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
            option::Option<E> err() {
                if (this->is_err()) return option::Some(this->Err_val);
                return option::None();
            }
        };
    };

    namespace option {
        template<typename T>
        struct Option {
            DATA(Option, (Some, T), (None));
            T unwrap() {
                if (this->tag == Tag::Some) return this->Some_val;
                exit(1);
            }
            bool is_some() {
                return this->tag == Tag::Some ? true : false;
            }
            bool is_none() {
                return this->tag == Tag::None ? true : false;
            }
            template<typename E>
            result::Result<T, E> ok_or_else(Fn<E> auto err) {
                match(*this) {{
                    of(Some, (v)) {
                        return result::Ok(v);
                    }
                    of(None) {
                        return result::Err(err());
                    }
                }}
            }
        };
    };
};

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                                            \
    ({                                                                       \
        auto ___temp = EXPR;                                                 \
        if (___temp.is_err()) return fst::result::Err(___temp.unwrap_err()); \
        ___temp.unwrap();                                                    \
    })

#endif // !FST_H_
