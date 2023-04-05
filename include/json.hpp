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
            using SerializeStruct = Serializer;
            using Ok = struct {};
            using Err = error::Error;

            // Result alias
            template<typename T>
            struct Result : result::Result<T, Err> {
                Result(error::Error e) : result::Result<T, Err>(e) {}
                Result(T ok) : result::Result<T, Err>(ok) {}
            };

            std::string output;

            Result<Ok> serialize(bool &v) {
                this->output += v ? "true" : "false";
                return Ok();
            }
            Result<Ok> serialize(int &v) {
                this->output += std::to_string(v);
                return Ok();
            }
            Result<Ok> serialize(const char *v) {
                this->output += "\"";
                this->output += v;
                this->output += "\"";
                return Ok();
            }
            template<typename T>
            Result<Ok> serialize(T &v) {
                v.serialize(*this);
                return Ok();
            }
            Result<SerializeStruct *> serialize_struct() {
                this->output += "{";
                return this;
            }
            template<typename T>
            Result<SerializeStruct *> serialize_field(const char *key, T &value) {
                if (output.back() != '{') {
                    this->output += ",";
                }
                serialize(key);
                this->output += ":";
                serialize(value);
                return this;
            }
            Result<Ok> end() {
                this->output += "}";
                return Ok();
            }
        };
    }
    template<typename T>
    result::Result<std::string, error::Error>
    // std::string
    from(T &value) {
        ser::Serializer serializer;
        serializer.serialize(value);
        return serializer.output;
    }
    namespace de {
        struct Deserializer {
            const char *input;
            Deserializer(const char *in) : input(in) {};
            using Error = error::Error;
            // Result alias
            template<typename T>
            struct Result : result::Result<T, Error> {
                Result(error::Error e) : result::Result<T, Error>(e) {}
                Result(T ok) : result::Result<T, Error>(ok) {}
            };

            // Parsing
            Result<char> peek_char() {
                if (*this->input == '\0') {
                    return Error("EOF");
                } else {
                    return *this->input;
                }
            }
            Result<char> next_char() {
                char ch = TRY(this->peek_char());
                this->input++;
                return ch;
            }
            Result<bool> parse_bool() {
                if (strncmp(this->input, "true", 4) == 0) {
                    this->input += strlen("true");
                    return true;
                } else if (strncmp(this->input, "false", 5) == 0) {
                    return false;
                }
                return Error("Expected bool");
            }
            template<typename T>
            Result<T> parse_unsigned() {
                char ch = TRY(this->next_char());
                if (ch <= '0' || ch >= '9') return Error("Expected unsigned integer");
                T res = ch - '0';
                while (true) {
                    match(this->peek_char()) {{
                        of(Ok, (ch, (ch >= '0' && ch <= '9'))) {
                            this->input++;
                            res *= 10;
                            res += ch - '0';
                        }
                        of_default { return res; }
                    }}
                }
            }
            template<typename T>
            Result<T> parse_signed() {
                bool neg = TRY(this->peek_char()) == '-';
                if (neg) this->input++;
                T res = TRY(this->parse_unsigned<T>());
                return neg ? -res : res;
            }
            Result<str> parse_string() {
                if (TRY(this->next_char()) != '"') return Error("Expected string");
                const char *end = strchr(this->input, '"');
                if (end == NULL) return Error("EOF");
                str res(this->input, end - this->input);
                this->input = end + 1;
                return res;
            }
        };
        template<typename T>
        class Visitor {
            // TODO
        };
    }
    template<typename T>
    T into(const char *json) {
        de::Deserializer deserializer(json);
        // TODO
    }
}

#define _SER_FIELD(field, ...) TRY(ser->serialize_field(#field, field));

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
        return Unit();                                 \
    }

#endif // JSON_H_
