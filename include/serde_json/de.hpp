#include <concepts>
#include <cstring>

#include "serde/de.hpp"
#include "serde/serde.hpp"
#include "fst/fst.hpp"
#include "serde_json/error.hpp"

#include <ftl.hpp>

namespace serde_json::de {
    using error::Result;

    struct Deserializer {
        using Error = error::Error;
        const char *input;
        Deserializer(const char *in) : input(in) {};

        // Parsing
        Result<char> peek_char() {
            if (*this->input == '\0') {
                return ftl::Err(Error::Eof());
            } else {
                return ftl::Ok(*this->input);
            }
        }
        Result<char> next_char() {
            char ch = TRY(this->peek_char());
            this->input++;
            return ftl::Ok(ch);
        }
        Result<bool> parse_bool() {
            if (strncmp(this->input, "true", 4) == 0) {
                this->input += strlen("true");
                return ftl::Ok(true);
            } else if (strncmp(this->input, "false", 5) == 0) {
                return ftl::Ok(false);
            }
            return ftl::Err(Error(Error::ExpectedBoolean()));
        }
        template<typename T>
        Result<T> parse_unsigned() {
            char ch = TRY(this->next_char());
            if (ch < '0' || ch > '9')
                return ftl::Err(Error::ExpectedInteger());
            T res = ch - '0';
            while (true) {
                // TODO: refactor this and remove the stupid macros
                switch (auto ___self = this->peek_char(); ___self.tag) {
                case decltype(___self)::Tag ::Ok: {
                    auto ch = ___self.unwrap();
                    if (!(ch >= '0' && ch <= '9')) goto ___default;
                    {
                        this->input++;
                        res *= 10;
                        res += ch - '0';
                    }
                    break;
                }
                default: {
                    ___default : { return ftl::Ok(res); }
                }
                }
            }
        }
        template<typename T>
        Result<T> parse_signed() {
            bool neg = TRY(this->peek_char()) == '-';
            if (neg) this->input++;
            T res = TRY(this->parse_unsigned<T>());
            return ftl::Ok(neg ? -res : res);
        }
        Result<fst::str> parse_string() {
            if (TRY(this->next_char()) != '"') return ftl::Err(Error::ExpectedString());
            const char *end = strchr(this->input, '"');
            if (end == NULL) return ftl::Err(Error::Eof());
            fst::str res(this->input, end - this->input);
            this->input = end + 1;
            return ftl::Ok(res);
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
                    return ftl::Ok(value);
                } else {
                    return ftl::Err(Error::ExpectedMapEnd());
                }
            } else {
                return ftl::Err(Error::ExpectedMap());
            }
        }
        template<typename V>
        Result<typename V::Value>
        deserialize_struct(
            const char *name,
            const fst::str fields[],
            V visitor
        ) {
            (void)name;
            (void)fields;
            return deserialize_map(visitor);
        }
        struct CommaSeparated {
            using Error = error::Error;

            Deserializer &de;
            bool first;

            CommaSeparated(Deserializer &de) : de(de), first(true) {}

            // MapAccess trait
            template<typename K>
            Result<ftl::Option<K>> next_key_seed(K seed) {
                (void)seed;
                if (TRY(this->de.peek_char()) == '}') {
                    return ftl::Ok(ftl::Option<K>(ftl::None()));
                }
                if (!this->first && TRY(this->de.next_char()) != ',') {
                    return ftl::Err(Error::ExpectedMapComma());
                }
                this->first = false;
                return serde::de::Deserialize<K>::deserialize(this->de).map(ftl::Some<K>);
                // return ftl::Ok(ftl::Option<K>(ftl::Some(TRY(
                //                     serde::de::Deserialize<K>::deserialize(this->de)))));
            }
            template<typename V>
            Result<V> next_value_seed(V seed) {
                (void)seed;
                if (TRY(this->de.next_char()) != ':') {
                    return ftl::Err(Error::ExpectedMapColon());
                }
                return serde::de::Deserialize<V>::deserialize(this->de);
            }

            template<typename K>
            Result<ftl::Option<K>> next_key() {
                return next_key_seed(K());
            }
            template<typename V>
            Result<V> next_value() {
                return next_value_seed(V());
            }
        };
    };
    static_assert(serde::de::Deserializer<Deserializer>);
}
