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

        using Ok = void;
        using Error = error::Error;

        using SerializeStruct = Serializer;

        Result<Ok> serialize_bool(const bool &value) {
            this->output += value ? "true" : "false";
            return fst::result::Ok();
        }

        Result<Ok> serialize_char(const char &value) {
            this->output += "\"";
            this->output += value;
            this->output += "\"";
            return fst::result::Ok();
        }

        Result<Ok> serialize_short(const short &value) { return serialize_long_long(value); }
        Result<Ok> serialize_int(const int &value) { return serialize_long_long(value); }
        Result<Ok> serialize_long(const long &value) { return serialize_long_long(value); }
        Result<Ok> serialize_long_long(const long long &value) {
            this->output += std::to_string(value);
            return fst::result::Ok();
        }

        Result<Ok> serialize_float(const float &value) { return serialize_double(value); }
        Result<Ok> serialize_double(const double &value) {
            this->output += std::to_string(value);
            return fst::result::Ok();
        }

        Result<Ok> serialize_str(const fst::str &value) {
            this->output += "\"";
            this->output += value;
            this->output += "\"";
            return fst::result::Ok();
        }
        Result<SerializeStruct *>
        serialize_struct(const fst::str &_name, const fst::usize _len) {
            this->output += "{";
            return fst::result::Ok(this);
        }
        template<serde::ser::Serializable T>
        Result<void>
        serialize_field(const fst::str &key, const T &value) {
            if (output.back() != '{') {
                this->output += ",";
            }
            serde::ser::Serialize<fst::str>::serialize(key, *this);
            this->output += ":";
            serde::ser::Serialize<T>::serialize(value, *this);
            return fst::result::Ok();
        }
        Result<SerializeStruct::Ok> end() {
            this->output += "}";
            return fst::result::Ok();
        }
    };
    static_assert(serde::ser::Serializer<Serializer>);
    static_assert(serde::ser::SerializeStruct<Serializer>);
}

#endif
