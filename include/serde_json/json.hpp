#ifndef JSON_H_
#define JSON_H_

#include "serde_json/ser.hpp"
#include "serde_json/de.hpp"

namespace json {
    using namespace fst;
    template<typename T>
    result::Result<std::string, fst::error::Error> from(T &value) {
        ser::Serializer serializer;
        serializer.serialize(value);
        return result::Ok(serializer.output);
    }

    template<typename T>
    T into(const char *json) {
        de::Deserializer deserializer(json);
        // TODO
    }
}

#endif // JSON_H_
