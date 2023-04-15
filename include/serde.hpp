#ifndef SERDE_H_
#define SERDE_H_

#include "result.hpp"
#include "str.hpp"

// Ideally I want everything to be traits and not classes to inherit

template<typename T, typename S>
concept Serialize = requires(T &value, S &serializer)
{
    value.serialize(serializer);
};

// template<typename T>
// concept SerializeStruct = requires(T a, const char *key, )
// {
//     a.serialize_field(key, value);
// };

// template<typename D, typename V>
// concept Deserializer = requires(D &deserializer, V &visitor)
// {
//     deserializer.deserialize_bool(visitor);
//     deserializer.deserialize_short(visitor);
//     deserializer.deserialize_ushort(visitor);
//     deserializer.deserialize_int(visitor);
//     deserializer.deserialize_uint(visitor);
//     deserializer.deserialize_long(visitor);
//     deserializer.deserialize_ulong(visitor);
//     deserializer.deserialize_long_long(visitor);
//     deserializer.deserialize_ulong_long(visitor);
//     deserializer.deserialize_str(visitor);
//     deserializer.deserialize_identifier(visitor);
// };

namespace serde {
    namespace de {
        using result::Err;
        template<typename T>
        using Result = result::Result<T, error::Error>;

        template<typename T>
        struct Visitor {
            using Value = T;

            virtual Result<Value> visit_bool(bool value) {
                return Err(error::Error("Invalid type: Unexpected Bool"));
            }
            virtual Result<Value> visit_short(short value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_int(int value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_long(long value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_long_long(long long value) {
                return Err(error::Error("Invalid type: Unexpected Signed"));
            }
            virtual Result<Value> visit_ushort(unsigned short value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_uint(unsigned int value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_ulong(unsigned long value) {
                return this->visit_long_long(value);
            }
            virtual Result<Value> visit_ulong_long(unsigned long long value) {
                return Err(error::Error("Invalid type: Unexpected Unsigned"));
            }
            virtual Result<Value> visit_float(float value) {
                return this->visit_double(value);
            }
            virtual Result<Value> visit_double(double value) {
                return Err(error::Error("Invalid type: Unexpected Float"));
            }
            virtual Result<Value> visit_str(str value) {
                return Err(error::Error("Invalid type: Unexpected str"));
            }
            // idk, feels wrong and broken
            template<typename A>
            Result<Value> visit_map(A map) {
                return Err(error::Error("Invalid type: Unexpected map"));
            }
        };
    }
}

#define _SER_FIELD(field) TRY(ser->serialize_field(#field, field));

/**
 * Specify `Serializer` with `using`
 * @params comma separated field names to serialize
 */
#define SERIALIZE(NAME, ...)                                \
    template<typename S>                                    \
    result::Result<void, error::Error>                      \
    serialize(S &serializer) {                              \
        using result::Ok;                                   \
        using result::Err;                                  \
        auto ser = TRY(serializer.serialize_struct(#NAME)); \
        FOREACH(_SER_FIELD, __VA_ARGS__);                   \
        TRY(ser->end());                                    \
        return Ok();                                        \
    }

#define DESERIALIZE(...)


#endif // SERDE_H_
