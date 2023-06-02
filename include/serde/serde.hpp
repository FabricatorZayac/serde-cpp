#ifndef SERDE_H_
#define SERDE_H_

#include "fst/fst.hpp"

namespace serde {
namespace de {
    using namespace fst;

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
    fst::result::Result<void, fst::error::Error>            \
    serialize(S &serializer) {                              \
        using fst::result::Ok;                              \
        using fst::result::Err;                             \
        auto ser = TRY(serializer.serialize_struct(#NAME)); \
        FOREACH(_SER_FIELD, __VA_ARGS__);                   \
        TRY(ser->end());                                    \
        return Ok();                                        \
    }

#define FIELD_VISITOR(FIELD)     \
    if (value == #FIELD) {       \
        return Ok(Field::FIELD); \
    }

#define FIELD_NAME(X) #X,

#define DESERIALIZE(NAME, ...)                                                  \
    template<typename D>                                                        \
    fst::result::Result<RGB, typename D::Error> deserialize(D &deserializer) {  \
        using namespace fst::result;                                            \
        struct FieldImpl {                                                      \
            Result<Field, typename D::Error> deserialize(D &deserializer) {     \
                struct FieldVisitor : serde::de::Visitor<Field> {               \
                    using Value = Field;                                        \
                    fst::result::Result<Value, fst::error::Error>               \
                    visit_str(fst::str value) override {                        \
                        FOREACH(FIELD_VISITOR, __VA_ARGS__)                     \
                        return Err(fst::error::Error("Unknown field"));         \
                    }                                                           \
                };                                                              \
                return deserializer.deserialize_identifier(FieldVisitor());     \
            }                                                                   \
        };                                                                      \
        const char *FIELDS[] = {                                                \
            FOREACH(FIELD_NAME, __VA_ARGS__)                                    \
        };                                                                      \
        return deserializer.deserialize_struct(#NAME, FIELDS, NAME##Visitor()); \
    }                                                                           \
    enum class Field { __VA_ARGS__ }

#endif // SERDE_H_
