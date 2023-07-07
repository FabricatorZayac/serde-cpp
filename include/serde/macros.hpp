#ifndef SERDE_MACROS_H_
#define SERDE_MACROS_H_

#include "fst/cursed_macros.h"
#include "fst/fst.hpp"

#define _SER_FIELD(field) TRY(state->serialize_field(#field, self.field));

#define SERIALIZE(NAME, ...)                                                \
namespace serde::ser {                                                      \
    template<>                                                              \
    struct Serialize<NAME> {                                                \
        template<serde::ser::Serializer S>                                  \
        ftl::Result<typename S::Ok, typename S::Error>              \
        static serialize(const NAME &self, S &serializer) {                 \
            using ftl::Ok;                                          \
            using ftl::Err;                                         \
            auto state =                                                    \
            TRY(serializer.serialize_struct(#NAME, NUM_ARGS(__VA_ARGS__))); \
            FOREACH(_SER_FIELD, __VA_ARGS__);                               \
            return state->end();                                            \
        }                                                                   \
    };                                                                      \
}

#define FIELD_VISITOR(FIELD)                  \
    if (value == #FIELD) {                    \
        return ftl::Ok(Field::FIELD); \
    }

#define FIELD_NAME(FIELD) #FIELD,

#define MAP_VISITOR_TEMPORARY_INIT(FIELD) \
    ftl::Option<decltype(Value::FIELD)> FIELD = ftl::None();

#define MAP_VISITOR_CASE(FIELD)                                          \
    case Field::FIELD:                                                   \
        if (FIELD.is_some()) {                                           \
            return ftl::Err(V::Error::duplicate_field(#FIELD));  \
        }                                                                \
        FIELD = ftl::Some(                                       \
                TRY(map.template next_value<decltype(Value::FIELD)>())); \
        break;

#define MAP_VISITOR_RETURN(FIELD)                                   \
    .FIELD = TRY(FIELD.ok_or_else([](){ \
                    return V::Error::missing_field(#FIELD);         \
                })),

#define DESERIALIZE(TYPE, ...)                                                \
namespace serde::de {                                                         \
    template<>                                                                \
    struct Deserialize<TYPE> {                                                \
        template<serde::de::Deserializer D>                                   \
        static ftl::Result<TYPE, typename D::Error>                   \
        deserialize(D &deserializer) {                                        \
            return deserializer.deserialize_struct(#TYPE, FIELDS, Visitor{}); \
        };                                                                    \
        struct Visitor {              \
            using Value = TYPE; \
            template<typename V>                                              \
            ftl::Result<Value, typename V::Error> visit_map(V map) {  \
                FOREACH(MAP_VISITOR_TEMPORARY_INIT, __VA_ARGS__)              \
                for (auto key = TRY(map.template next_key<Field>());          \
                        key.is_some();                                        \
                        key = TRY(map.template next_key<Field>())) {          \
                    switch (key.unwrap()) {                                   \
                        FOREACH(MAP_VISITOR_CASE, __VA_ARGS__)                \
                    }                                                         \
                }                                                             \
                return ftl::Ok(                                       \
                        TYPE {                                                \
                            FOREACH(MAP_VISITOR_RETURN, __VA_ARGS__)          \
                        });                                                   \
            }                                                                 \
        };                                                                    \
        constexpr static ftl::str FIELDS[] = {                                \
            FOREACH(FIELD_NAME, __VA_RGS__)                                   \
        };                                                                    \
        enum class Field { __VA_ARGS__ };                                     \
    };                                                                        \
    template<>                                                                \
    struct Deserialize<typename Deserialize<TYPE>::Field> {                   \
        using Field = typename Deserialize<TYPE>::Field;                      \
        template<serde::de::Deserializer D>                                   \
        static ftl::Result<Field, typename D::Error>                  \
        deserialize(D &deserializer) {                                        \
            struct FieldVisitor : detail::archetypes::de::Visitor<Field> {    \
                ftl::Result<Value, typename D::Error>                 \
                visit_str(ftl::str value) {                                   \
                    FOREACH(FIELD_VISITOR, __VA_ARGS__)                       \
                    return ftl::Err(D::Error::unknown_field(          \
                                value, Deserialize<TYPE>::FIELDS));           \
                }                                                             \
            };                                                                \
            return deserializer.deserialize_identifier(FieldVisitor());       \
        }                                                                     \
    };                                                                        \
}

#endif // !SERDE_MACROS_H_
