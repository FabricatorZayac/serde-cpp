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

    namespace error {
        template<typename T>
        concept Error = requires(T err) {
            { err.description() } -> std::same_as<std::string>;
        };
    }
};

#define _DEBUG_FIELD(x) << ", " << #x << ": " << self.x

#define DEBUG_OSTREAM(T, FIRST, ...) \
    friend std::ostream &operator<<(std::ostream &out, T &self) { \
        return out << #T << " { " << #FIRST << ": " << self.FIRST \
        FOREACH(_DEBUG_FIELD, __VA_ARGS__) << " }";               \
    }

#endif // !FST_H_
