#ifndef COMMA_SEPARATED_H_
#define COMMA_SEPARATED_H_

#include "json.hpp"

namespace json::de {
        struct CommaSeparated {
            Deserializer *de;
            bool first;
            CommaSeparated(Deserializer *de) : de(de), first(true) {}

            template<typename T>
            using Result = result::Result<T, error::Error>;

            // MapAccess trait
            template<typename K>
            Result<option::Option<K>> next_key_seed(K seed) {
                if (TRY(this->de->peek_char())) {

                }
            }
        };
}

#endif // COMMA_SEPARATED_H_
