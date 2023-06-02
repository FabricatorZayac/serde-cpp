#ifndef COMMA_SEPARATED_H_
#define COMMA_SEPARATED_H_

#include "fst/fst.hpp"

// TODO: fix this shit bruh
namespace json::de {
    using namespace fst;

    struct Deserializer;
    struct CommaSeparated {
        using Error = error::Error;

        Deserializer &de;
        bool first;
        CommaSeparated(Deserializer &de);

        template<typename T>
        using Result = result::Result<T, error::Error>;

        // MapAccess trait
        template<typename K>
        Result<option::Option<K>> next_key_seed(K seed);
        template<typename V>
        Result<typename V::value> next_value_seed(V seed);

        template<typename K>
        Result<option::Option<K>> next_key() {
            return next_key_seed(K());
        }
        template<typename V>
        Result<V> next_key() {
            return next_value_seed(V());
        }
    };
}

#endif // COMMA_SEPARATED_H_
