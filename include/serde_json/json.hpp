#ifndef JSON_H_
#define JSON_H_

#include "fst/fst.hpp"
#include "serde/serde.hpp"
#include "serde_json/ser.hpp"
#include "serde_json/de.hpp"
#include <cstring>

namespace serde_json {
    template<serde::ser::Serializable T>
    error::Result<std::string> to_string(T &value) {
        ser::Serializer serializer;
        serde::ser::Serialize<T>::serialize(value, serializer).unwrap();
        return ftl::Ok(serializer.output);
    }

    /* template<serde::de::Deserializable T> */
    template<typename T>
    error::Result<T> from_str(const char *json) {
        de::Deserializer deserializer(json);
        T t = TRY(serde::de::Deserialize<T>::deserialize(deserializer));
        if (strlen(deserializer.input) == 0) {
            return ftl::Ok(t);
        } else {
            return ftl::Err(error::Error::TrailingCharacters());
        }
    }
}

#endif // JSON_H_
