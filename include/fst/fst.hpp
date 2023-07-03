#ifndef FST_H_
#define FST_H_

#include <concepts>
#include <cstdlib>
#include <ostream>
#include <string>
#include <type_traits>
#include <cstring>

#include "cursed_macros.h"
#include "datatype_macros.hpp"

namespace fst {
    using usize = size_t;

    template<typename T>
    void drop(T &value) {
        value.~T();
    }

    template<typename T, typename Ret, typename ...Args>
    concept Fn = requires(T fn, Args...args) {
        { fn(args...) } -> std::same_as<Ret>;
    };

    struct str {
        constexpr str() : length(0) {}
        constexpr str(const char *body, const usize length) :
            body(body),
            length(length) {}
        constexpr str(const char *c_str) :
            body(c_str),
            length(std::char_traits<char>::length(c_str)) {}
        constexpr str(const str &other) :
            body(other.body),
            length(other.length) {}
        usize len() { return length; }
        bool equals(const str &other) {
            if (other.length != this->length) return false;
            for (usize i = 0; i < this->length; i++) {
                if (this->body[i] != other.body[i]) return false;
            }
            return true;
        }
        bool equals(const char *other) {
            for (usize i = 0; i < this->length; i++) {
                if (this->body[i] != other[i]) return false;
            }
            return true;
        }
        template<typename T>
        bool operator==(const T &other) {
            return this->equals(other);
        }
        friend std::ostream &operator<<(std::ostream &out, const str &s) {
            return out.write(s.body, s.length);
        }
        friend std::string &operator+=(std::string &string, const str &self) {
            return string.append(self.body, self.length);
        }
    private:
        const char *body;
        usize length;
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
                return this->tag == Tag::Ok;
            }
            bool is_err() {
                return this->tag == Tag::Err;
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
                return this->tag == Tag::Some;
            }
            bool is_none() {
                return this->tag == Tag::None;
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

    namespace error {
        template<typename T>
        concept Error = requires(T err) {
            { err.description() } -> std::same_as<std::string>;
        };
    }
};

// I feel a little guilty using GNU extrensions, but whatever, this is C++ baybe
#define TRY(EXPR)                                                            \
    ({                                                                       \
        auto ___temp = EXPR;                                                 \
        if (___temp.is_err()) return fst::result::Err(___temp.unwrap_err()); \
        ___temp.unwrap();                                                    \
    })

#define _DEBUG_FIELD(x) << ", " << #x << ": " << self.x

#define DEBUG_OSTREAM(T, FIRST, ...) \
    friend std::ostream &operator<<(std::ostream &out, T &self) { \
        return out << #T << " { " << #FIRST << ": " << self.FIRST \
        FOREACH(_DEBUG_FIELD, __VA_ARGS__) << " }";               \
    }

#endif // !FST_H_
