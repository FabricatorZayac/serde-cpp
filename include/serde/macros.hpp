#ifndef SERDE_MACROS_H_
#define SERDE_MACROS_H_

#include "fst/cursed_macros.h"
#include "fst/fst.hpp"

#define _SER_FIELD(field) TRY(state->serialize_field(#field, self.field));

#define SERIALIZE(NAME, ...)                                                \
namespace serde::ser {                                                      \
    template<Serializer S>                                                  \
    struct Serialize<NAME, S> {                                             \
        fst::result::Result<typename S::Ok, typename S::Error>              \
        static serialize(const NAME &self, S &serializer) {                 \
            using fst::result::Ok;                                          \
            using fst::result::Err;                                         \
            auto state =                                                    \
            TRY(serializer.serialize_struct(#NAME, NUM_ARGS(__VA_ARGS__))); \
            FOREACH(_SER_FIELD, __VA_ARGS__);                               \
            return state->end();                                            \
        }                                                                   \
    };                                                                      \
}

#define FIELD_VISITOR(FIELD)                  \
    if (value == #FIELD) {                    \
        return fst::result::Ok(Field::FIELD); \
    }

#define FIELD_NAME(FIELD) #FIELD,

#define MAP_VISITOR_TEMPORARY_INIT(FIELD) \
    fst::option::Option<decltype(Value::FIELD)> FIELD = fst::option::None();

#define MAP_VISITOR_CASE(FIELD)                                          \
    case Field::FIELD:                                                   \
        if (FIELD.is_some()) {                                           \
            return fst::result::Err(V::Error::duplicate_field(#FIELD));  \
        }                                                                \
        FIELD = fst::option::Some(                                       \
                TRY(map.template next_value<decltype(Value::FIELD)>())); \
        break;

#define MAP_VISITOR_RETURN(FIELD)                                   \
    .FIELD = TRY(FIELD.template ok_or_else<typename V::Error>([](){ \
                    return V::Error::missing_field(#FIELD);         \
                })),

#define DESERIALIZE(TYPE, ...)                                                \
namespace serde::de {                                                         \
    template<serde::de::Deserializer D>                                       \
    struct Deserialize<TYPE, D> {                                             \
        static fst::result::Result<TYPE, typename D::Error>                   \
        deserialize(D &deserializer) {                                        \
            return deserializer.deserialize_struct(#TYPE, FIELDS, Visitor{}); \
        };                                                                    \
        struct Visitor {                                                      \
            using Value = TYPE;                                               \
            template<typename V>                                              \
            fst::result::Result<Value, typename V::Error> visit_map(V map) {  \
                FOREACH(MAP_VISITOR_TEMPORARY_INIT, __VA_ARGS__)              \
                for (auto key = TRY(map.template next_key<Field>());          \
                        key.is_some();                                        \
                        key = TRY(map.template next_key<Field>())) {          \
                    switch (key.unwrap()) {                                   \
                        FOREACH(MAP_VISITOR_CASE, __VA_ARGS__)                \
                    }                                                         \
                }                                                             \
                return fst::result::Ok(                                       \
                        TYPE {                                                \
                            FOREACH(MAP_VISITOR_RETURN, __VA_ARGS__)          \
                        });                                                   \
            }                                                                 \
        };                                                                    \
        constexpr static fst::str FIELDS[] = {                                \
            FOREACH(FIELD_NAME, __VA_RGS__)                                   \
        };                                                                    \
        enum class Field { __VA_ARGS__ };                                     \
    };                                                                        \
    template<serde::de::Deserializer D>                                       \
    struct Deserialize<typename Deserialize<TYPE, D>::Field, D> {             \
        using Struct = Deserialize<TYPE, D>;                                  \
        using Field = typename Struct::Field;                                 \
        static fst::result::Result<Field, typename D::Error>                  \
        deserialize(D &deserializer) {                                        \
            struct FieldVisitor :                                             \
                serde::detail::archetypes::de::Visitor<Field, typename D::Error> { \
                using Value = Field;                                          \
                fst::result::Result<Value, typename D::Error>                 \
                visit_str(fst::str value) {                                   \
                    FOREACH(FIELD_VISITOR, __VA_ARGS__)                       \
                    return fst::result::Err(                                  \
                            D::Error::unknown_field(value, Struct::FIELDS));  \
                }                                                             \
            };                                                                \
            return deserializer.deserialize_identifier(FieldVisitor());       \
        }                                                                     \
    };                                                                        \
}

#endif // !SERDE_MACROS_H_
