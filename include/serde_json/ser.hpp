#ifndef JSON_SER_H_
#define JSON_SER_H_

#include <string>

#include "fst/fst.hpp"

namespace json::ser {
    using namespace fst;
    using result::Ok;
    using result::Err;
    using error::Error;

    struct Serializer {
        std::string output;

        struct Self {
            using Ok = void;
            using SerializeStruct = Serializer;
        };
        using SerializeStruct = Self::SerializeStruct::Self;

        template<typename T>
        using Result = result::Result<T, Error>;

        Result<Self::Ok> serialize(bool &v) {
            this->output += v ? "true" : "false";
            return Ok();
        }
        Result<Self::Ok> serialize(int &v) {
            this->output += std::to_string(v);
            return Ok();
        }
        Result<Self::Ok> serialize(const char *&v) {
            this->output += "\"";
            this->output += v;
            this->output += "\"";
            return Ok();
        }
        template<typename T>
        Result<Self::Ok> serialize(T &v) {
            v.serialize(*this);
            return Ok();
        }
        Result<Self::SerializeStruct *> serialize_struct(const char *name) {
            this->output += "{";
            return Ok(this);
        }
        template<typename T>
        Result<SerializeStruct::Ok> serialize_field(const char *key, T &value) {
            if (output.back() != '{') {
                this->output += ",";
            }
            serialize(key);
            this->output += ":";
            serialize(value);
            return Ok();
        }
        Result<SerializeStruct::Ok> end() {
            this->output += "}";
            return Ok();
        }
    };
}

#endif
