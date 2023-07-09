#ifndef JSON_SER_H_
#define JSON_SER_H_

#include <functional>
#include <string>

#include "fst/fst.hpp"
#include "error.hpp"
#include "serde/ser.hpp"

namespace serde_json::ser {
    using error::Result;

    struct Serializer {
        std::string output;

        using Ok = void;
        using Error = error::Error;

        using SerializeStruct = Serializer;
        using SerializeSeq = Serializer;

        Result<Ok> serialize_bool(const bool &value) {
            this->output += value ? "true" : "false";
            return ftl::Ok();
        }

        Result<Ok> serialize_char(const char &value) {
            this->output += "\"";
            this->output += value;
            this->output += "\"";
            return ftl::Ok();
        }

        Result<Ok> serialize_short(const short &value) { return serialize_long_long(value); }
        Result<Ok> serialize_int(const int &value) { return serialize_long_long(value); }
        Result<Ok> serialize_long(const long &value) { return serialize_long_long(value); }
        Result<Ok> serialize_long_long(const long long &value) {
            this->output += std::to_string(value);
            return ftl::Ok();
        }

        Result<Ok> serialize_float(const float &value) { return serialize_double(value); }
        Result<Ok> serialize_double(const double &value) {
            this->output += std::to_string(value);
            return ftl::Ok();
        }

        Result<Ok> serialize_str(const ftl::str &value) {
            this->output += "\"";
            this->output += value;
            this->output += "\"";
            return ftl::Ok();
        }

        Result<SerializeStruct &>
        serialize_struct(const ftl::str &name, const size_t len) {
            (void)name;
            (void)len;
            this->output += "{";
            return ftl::Ok(std::ref(*this));
        }
        template<serde::ser::Serializable T>
        Result<void>
        serialize_field(const ftl::str &key, const T &value) {
            if (output.back() != '{') {
                this->output += ',';
            }
            serde::ser::Serialize<ftl::str>::serialize(key, *this).unwrap();
            this->output += ":";
            serde::ser::Serialize<T>::serialize(value, *this).unwrap();
            return ftl::Ok();
        }
        Result<SerializeStruct::Ok> end() {
            this->output += "}";
            return ftl::Ok();
        }

        Result<SerializeSeq &>
        serialize_seq(ftl::Option<size_t> len) {
            (void)len;
            output += '[';
            return ftl::Ok(std::ref(*this));
        }
        template<serde::ser::Serializable T>
        Result<void> serialize_element(const T &value) {
            if (output.back() != '[') output += ',';
            return serde::ser::Serialize<T>::serialize(value, *this);
        }
        Result<SerializeSeq::Ok> end_seq() {
            this->output += "]";
            return ftl::Ok();
        }
    };
    static_assert(serde::ser::Serializer<Serializer>);
    static_assert(serde::ser::SerializeStruct<Serializer>);
    static_assert(serde::ser::SerializeSeq<Serializer>);
}

#endif
