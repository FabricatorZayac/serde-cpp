#include <concepts>
#include <cstring>

#include "serde/serde.hpp"
#include "fst/fst.hpp"
#include "serde_json/error.hpp"

namespace serde_json::de {
    using fst::result::Ok;
    using fst::result::Err;
    using error::Result;

    struct Deserializer {
        using Error = error::Error;
        const char *input;
        Deserializer(const char *in) : input(in) {};

        // Parsing
        Result<char> peek_char() {
            if (*this->input == '\0') {
                return Err(Error::Eof());
            } else {
                return Ok(*this->input);
            }
        }
        Result<char> next_char() {
            char ch = TRY(this->peek_char());
            this->input++;
            return Ok(ch);
        }
        Result<bool> parse_bool() {
            if (strncmp(this->input, "true", 4) == 0) {
                this->input += strlen("true");
                return Ok(true);
            } else if (strncmp(this->input, "false", 5) == 0) {
                return Ok(false);
            }
            return Err(Error(Error::ExpectedBoolean()));
        }
        template<typename T>
        Result<T> parse_unsigned() {
            char ch = TRY(this->next_char());
            if (ch < '0' || ch > '9')
                return Err(Error::ExpectedInteger());
            T res = ch - '0';
            while (true) {
                match(this->peek_char()) {{
                    of(Ok, (ch, (ch >= '0' && ch <= '9'))) {
                        this->input++;
                        res *= 10;
                        res += ch - '0';
                    }
                    of_default { return Ok(res); }
                }}
            }
        }
        template<typename T>
        Result<T> parse_signed() {
            bool neg = TRY(this->peek_char()) == '-';
            if (neg) this->input++;
            T res = TRY(this->parse_unsigned<T>());
            return Ok(neg ? -res : res);
        }
        Result<fst::str> parse_string() {
            if (TRY(this->next_char()) != '"') return Err(Error::ExpectedString());
            const char *end = strchr(this->input, '"');
            if (end == NULL) return Err(Error::Eof());
            fst::str res(this->input, end - this->input);
            this->input = end + 1;
            return Ok(res);
        }
        // Deserializer trait
        template<typename V>
        Result<typename V::Value> deserialize_any(V visitor) {
            switch (TRY(this->peek_char())) {
            case 't':
            case 'f':
                this->deserialize_bool(visitor);
                break;
            case '"':
                this->deserialize_str(visitor);
                break;
            case '0' ... '9':
                this->deserialize_ulong_long(visitor);
                break;
            case '-':
                this->deserialize_long_long(visitor);
                break;
            }
        }
        template<typename V>
        Result<typename V::Value> deserialize_bool(V visitor) {
            return visitor.visit_bool(TRY(this->parse_bool()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_short(V visitor) {
            return visitor.visit_short(TRY(this->parse_signed<short>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_int(V visitor) {
            return visitor.visit_int(TRY(this->parse_signed<int>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_long(V visitor) {
            return visitor.visit_long(TRY(this->parse_signed<long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_long_long(V visitor) {
            return visitor.visit_long_long(TRY(this->parse_signed<long long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ushort(V visitor) {
            return visitor.visit_short(TRY(this->parse_unsigned<unsigned short>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_uint(V visitor) {
            return visitor.visit_int(TRY(this->parse_unsigned<unsigned int>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ulong(V visitor) {
            return visitor.visit_long(TRY(this->parse_unsigned<unsigned long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_ulong_long(V visitor) {
            return visitor.visit_long_long(TRY(this->parse_unsigned<unsigned long long>()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_str(V visitor) {
            return visitor.visit_str(TRY(this->parse_string()));
        }
        template<typename V>
        Result<typename V::Value> deserialize_identifier(V visitor) {
            return this->deserialize_str(visitor);
        }
        template<typename V>
        Result<typename V::Value> deserialize_map(V visitor) {
            if (TRY(this->next_char()) == '{') {
                auto value = TRY(visitor.visit_map(CommaSeparated(*this)));
                if (TRY(this->next_char()) == '}') {
                    return Ok(value);
                } else {
                    return Err(Error::ExpectedMapEnd());
                }
            } else {
                return Err(Error::ExpectedMap());
            }
        }
        template<typename V>
        Result<typename V::Value>
        deserialize_struct(
            const char *_name,
            const fst::str _fields[],
            V visitor
        ) {
            return deserialize_map(visitor);
        }
        struct CommaSeparated {
            using Error = error::Error;

            Deserializer &de;
            bool first;

            CommaSeparated(Deserializer &de) : de(de), first(true) {}

            // MapAccess trait
            template<typename K>
            Result<fst::option::Option<K>> next_key_seed(K seed) {
                if (TRY(this->de.peek_char()) == '}') {
                    return Ok(fst::option::Option<K>(fst::option::None()));
                }
                if (!this->first && TRY(this->de.next_char()) != ',') {
                    return Err(Error::ExpectedMapComma());
                }
                this->first = false;
                return Ok(fst::option::Option<K>(fst::option::Some(TRY(
                                    Deserialize<K>{}(this->de)))));
            }
            template<typename V>
            Result<V> next_value_seed(V seed) {
                if (TRY(this->de.next_char()) != ':') {
                    return Err(Error::ExpectedMapColon());
                }
                return Deserialize<V>{}(this->de);
            }

            template<typename K>
            Result<fst::option::Option<K>> next_key() {
                return next_key_seed(K());
            }
            template<typename V>
            Result<V> next_value() {
                return next_value_seed(V());
            }
        };
        template<typename T>
        using Deserialize = serde::de::Deserialize<T, Deserializer>;
    };
}
