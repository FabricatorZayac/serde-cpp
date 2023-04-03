#ifndef JSON_H_
#define JSON_H_

#include <string>
#include <cstring>

#include "cursed_macros.h"
#include "error.hpp"
#include "result.hpp"
#include "datatype.hpp"

namespace json {
    namespace ser {
        struct Serializer {
            std::string output;
            void serialize(bool &v) {
                this->output += v ? "true" : "false";
            }
            void serialize(int &v) {
                this->output += std::to_string(v);
            }
            void serialize(const char *v) {
                this->output += "\"";
                this->output += v;
                this->output += "\"";
            }
            template<typename T>
            void serialize(T &v) {
                v.serialize(*this);
            }
            Serializer *serialize_struct(const char *name) {
                this->output += "{";
                return this;
            }
            template<typename T>
            void serialize_field(const char *key, T &value) {
                if (output.back() != '{') {
                    this->output += ",";
                }
                serialize(key);
                this->output += ":";
                serialize(value);
            }
            void end() {
                this->output += "}";
            }
        };
    }
    template<typename T>
    std::string from(T &value) {
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
                char ch = TRY(this->peek_char());
                if (ch <= '0' || ch >= '9') return Error("Expected unsigned integer");
                T res = ch - '0';
                while (true) {
                    match(this->peek_char().ok()) {{
                        of(Some, ch)
                            if (ch <= '0' || ch >= '9') {
                                return res;
                            } else {
                                this->input++;
                                res *= 10;
                                res += ch - '0';
                            }
                        of(None) return res;
                    }}
                }
            }
        };
    }
    template<typename T>
    T into(const char *json) {
        de::Deserializer deserializer(json);
        // TODO
    }
}

#define _SER_FIELD(field, ...) serialize_struct->serialize_field(#field, field);

#define SERIALIZE(T, ...)                                        \
    void serialize(json::ser::Serializer &serializer) {          \
        auto serialize_struct = serializer.serialize_struct(#T); \
        FOREACH(_SER_FIELD, __VA_ARGS__)                         \
        serialize_struct->end();                                 \
    }

#endif // JSON_H_
