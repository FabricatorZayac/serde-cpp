#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <cstring>

#include "cursed_macros.h"
#include "error.hpp"
#include "result.hpp"
#include "datatype.hpp"
#include "str.hpp"

namespace json {
    namespace ser {
        struct Serializer {
            std::string output;

            using SerializeStruct = Serializer;
            using Unit = struct {};

            template<typename T>
            using Result = result::Result<T, error::Error>;

            template<typename T>
            using Ok = result::Ok<T>;

            Result<Unit> serialize(bool &v) {
                this->output += v ? "true" : "false";
                return Ok<Unit>(Unit());
            }
            Result<Unit> serialize(int &v) {
                this->output += std::to_string(v);
                return Ok<Unit>(Unit());
            }
            Result<Unit> serialize(const char *v) {
                this->output += "\"";
                this->output += v;
                this->output += "\"";
                return Ok<Unit>(Unit());
            }
            template<typename T>
            Result<Unit> serialize(T &v) {
                v.serialize(*this);
                return Ok<Unit>(Unit());
            }
            Result<SerializeStruct *> serialize_struct() {
                this->output += "{";
                return Ok<SerializeStruct *>(this);
            }
            template<typename T>
            Result<SerializeStruct *> serialize_field(const char *key, T &value) {
                if (output.back() != '{') {
                    this->output += ",";
                }
                serialize(key);
                this->output += ":";
                serialize(value);
                return Ok<SerializeStruct *>(this);
            }
            Result<Unit> end() {
                this->output += "}";
                return Ok<Unit>(Unit());
            }
        };
    }
    template<typename T>
    result::Result<std::string, error::Error>
    // std::string
    from(T &value) {
        ser::Serializer serializer;
        serializer.serialize(value);
        return {Ok, serializer.output};
    }
}

namespace json {
    namespace de {
        struct Deserializer {
            const char *input;
            Deserializer(const char *in) : input(in) {};
            using Error = error::Error;
            // Result alias
            template<typename T>
            struct Result : result::Result<T, Error> {
                Result(enum result::Tag tag, error::Error e) : result::Result<T, Error>(tag, e) {}
                Result(enum result::Tag tag, T ok) : result::Result<T, Error>(tag, ok) {}
            };

            // Parsing
            Result<char> peek_char() {
                if (*this->input == '\0') {
                    return {Err, Error("EOF")};
                } else {
                    return {Ok, *this->input};
                }
            }
            Result<char> next_char() {
                char ch = TRY(this->peek_char());
                this->input++;
                return {Ok, ch};
            }
            Result<bool> parse_bool() {
                if (strncmp(this->input, "true", 4) == 0) {
                    this->input += strlen("true");
                    return {Ok, true};
                } else if (strncmp(this->input, "false", 5) == 0) {
                    return {Ok, false};
                }
                return {Err, Error("Expected bool")};
            }
            template<typename T>
            Result<T> parse_unsigned() {
                char ch = TRY(this->next_char());
                if (ch <= '0' || ch >= '9')
                    return {Err, Error("Expected unsigned integer")};
                T res = ch - '0';
                while (true) {
                    match(this->peek_char()) {{
                        of(Ok, (ch, (ch >= '0' && ch <= '9'))) {
                            this->input++;
                            res *= 10;
                            res += ch - '0';
                        }
                        of_default { return {Ok, res}; }
                    }}
                }
            }
            template<typename T>
            Result<T> parse_signed() {
                bool neg = TRY(this->peek_char()) == '-';
                if (neg) this->input++;
                T res = TRY(this->parse_unsigned<T>());
                return {Ok, neg ? -res : res};
            }
            Result<str> parse_string() {
                if (TRY(this->next_char()) != '"') return {Err, Error("Expected string")};
                const char *end = strchr(this->input, '"');
                if (end == NULL) return {Err, Error("EOF")};
                str res(this->input, end - this->input);
                this->input = end + 1;
                return {Ok, res};
            }
        };
    }
    template<typename T>
    T into(const char *json) {
        de::Deserializer deserializer(json);
        // TODO
    }
}

namespace json {
    namespace de {
        template<typename T>
        struct Visitor {

        };
    }
}

#define _SER_FIELD(field) TRY(ser->serialize_field(#field, field));

/**
 * Specify `Serializer` with `using`
 * @params comma separated field names to serialize
 */
#define SERIALIZE(...)                                 \
    struct Unit {};                                    \
    result::Result<Unit, error::Error>                 \
    serialize(Serializer &serializer) {                \
        auto ser = TRY(serializer.serialize_struct()); \
        FOREACH(_SER_FIELD, __VA_ARGS__);              \
        TRY(ser->end());                               \
        return {Ok, Unit()};                           \
    }

#endif // JSON_H_
