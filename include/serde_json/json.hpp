#ifndef JSON_H_
#define JSON_H_

#include "fst/fst.hpp"
#include "serde/serde.hpp"
#include "serde_json/ser.hpp"
#include "serde_json/de.hpp"
#include <cstring>

namespace serde_json {
    using error::Result;
    template<typename T>
    Result<std::string> to_string(T &value) {
        ser::Serializer serializer;
        ser::Serializer::Serialize<T>{}(value, serializer);
        return fst::result::Ok(serializer.output);
    }

    template<typename T>
    Result<T> from_str(const char *json) {
        de::Deserializer deserializer(json);
        T t = TRY(de::Deserializer::Deserialize<T>{}(deserializer));
        if (strlen(deserializer.input) == 0) {
            return fst::result::Ok(t);
        } else {
            return fst::result::Err(error::Error::TrailingCharacters());
        }
    }
}

#endif // JSON_H_
