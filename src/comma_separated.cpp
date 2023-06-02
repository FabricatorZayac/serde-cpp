#include "json.hpp"
#include "comma_separated.hpp"

using namespace fst;
using namespace option;
using namespace result;

namespace json::de {
    CommaSeparated::CommaSeparated(Deserializer &de) : de(de), first(true) {}
    template<typename K>
    Result<Option<K>> CommaSeparated::next_key_seed(K seed) {
        if (TRY(this->de.peek_char()) == '}') {
            return Ok(None());
        }
        if (!this->first && TRY(this->de.next_char()) != ',') {
            return Err(error::Error("Expected map comma"));
        }
        this->first = false;
        return seed.deserialize(this->de);
    }

    template<typename V>
    Result<typename V::value> CommaSeparated::next_value_seed(V seed) {
        if (TRY(this->de.next_char()) != ':') {
            return Err("Expected map column");
        }
        return seed.deserialize(this->de);
    }
}
