#ifndef JSON_SER_H_
#define JSON_SER_H_

#include <string>

#include "fst/fst.hpp"
#include "error.hpp"
#include "serde/serde.hpp"

namespace serde_json::ser {
    using error::Result;

    struct Serializer {
        std::string output;

        using Error = error::Error;

        using Ok = void;
        using SerializeStruct = Serializer;

        Result<Ok> serialize_bool(bool &value) {
            this->output += value ? "true" : "false";
            return fst::result::Ok();
        }
        Result<Ok> serialize_int(int &value) {
            this->output += std::to_string(value);
            return fst::result::Ok();
        }
        Result<Ok> serialize_str(fst::str &value) {
            this->output += "\"";
            this->output += value;
            this->output += "\"";
            return fst::result::Ok();
        }
        Result<SerializeStruct *> serialize_struct(const char *name) {
            this->output += "{";
            return fst::result::Ok(this);
        }
        template<typename T>
        Result<SerializeStruct::Ok> serialize_field(fst::str key, T &value) {
            if (output.back() != '{') {
                this->output += ",";
            }
            Serialize<fst::str>{}(key, *this);
            this->output += ":";
            Serialize<T>{}(value, *this);
            return fst::result::Ok();
        }
        Result<SerializeStruct::Ok> end() {
            this->output += "}";
            return fst::result::Ok();
        }
        template<typename T>
        using Serialize = serde::ser::Serialize<T, Serializer>;
    };
}

#endif
