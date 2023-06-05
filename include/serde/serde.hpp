#ifndef SERDE_H_
#define SERDE_H_

#include "fst/datatype_macros.hpp"
#include "fst/fst.hpp"
#include <array>
#include <concepts>
#include <ostream>

namespace serde {
namespace ser {
    template<typename Self>
    concept Error = fst::error::Error<Self>
                 && requires(Self err, const char *msg) {
        { Self::custom(msg) } -> std::same_as<Self>;
    };
}

namespace ser {
    template<typename S>
    concept SerializeStruct =
    requires(S serializer) {
        typename S::Ok;
        ser::Error<typename S::Error>;
    };
    template<typename S>
    concept Serializer =
    requires(S serializer,
             bool &Bool,
             int &Int,
             fst::str &Str) {
        typename S::Ok;
        ser::Error<typename S::Error>;
        { serializer.serialize_bool(Bool) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_int(Int) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_str(Str) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
    };
    template<typename V>
    concept Visitor = requires {
        typename V::Value;
    };
}

namespace de {
    template<typename D>
    concept Deserializer =
    requires(D deserialize) {
        fst::error::Error<typename D::Error>;
    };
}

namespace ser {
    template<typename T, Serializer S>
    struct Serialize {};

    template<Serializer S>
    struct Serialize<int, S> {
        fst::result::Result<typename S::Ok, typename S::Error>
        operator()(int &self, S &serializer) {
            return serializer.serialize_int(self);
        }
    };
    template<Serializer S>
    struct Serialize<fst::str, S> {
        fst::result::Result<typename S::Ok, typename S::Error>
        operator()(fst::str &self, S &serializer) {
            return serializer.serialize_str(self);
        }
    };
}

namespace de {
    template<typename T, Deserializer D>
    struct Deserialize {};
    
    template<Deserializer D>
    struct Deserialize<int, D> {
        fst::result::Result<int, typename D::Error> operator()(D &deserializer) {
            struct IntVisitor {
                using Value = int;
                fst::result::Result<Value, typename D::Error> visit_int(int value) {
                    return fst::result::Ok(value);
                }
            };
            return deserializer.deserialize_int(IntVisitor{});
        }
    };

    template<Deserializer D>
    struct Deserialize<fst::str, D> {
        fst::result::Result<fst::str, typename D::Error> operator()(D &deserializer) {
            struct StrVisitor {
                using Value = fst::str;
                fst::result::Result<Value, typename D::Error> visit_str(fst::str value) {
                    return fst::result::Ok(value);
                }
            };
            return deserializer.deserialize_str(StrVisitor{});
        }
    };
}

namespace de {
    struct Unexpected {
        DATA_INITIALIZER_TYPES((Bool, T),
                               (Unsigned, U),
                               (Signed, V),
                               (Float, W),
                               (Char, X),
                               (Str, Y),
                               (Unit),
                               (Map),
                               (Other, Z));
        DATA(Unexpected,
                (Bool, bool),
                (Unsigned, unsigned long long),
                (Signed, long long),
                (Float, double),
                (Char, char),
                (Str, fst::str),
                (Unit),
                (Map),
                (Other, fst::str));
        friend std::ostream &operator<<(std::ostream &out, const Unexpected &unexp) {
            match(unexp) {{
                of(Bool, (b)) {
                    return out << "boolean `" << b << '`';
                }
                of(Unsigned, (i)) {
                    return out << "integer `" << i << '`';
                }
                of(Signed, (i)) {
                    return out << "integer `" << i << '`';
                }
                of(Float, (f)) {
                    return out << "floating point `" << f << '`';
                }
                of(Char, (c)) {
                    return out << "character `" << c << '`';
                }
                of(Str, (s)) {
                    return out << "string " << s;
                }
                of(Unit) {
                    return out << "unit value";
                }
                of(Map) {
                    return out << "map";
                }
                of(Other, (other)) {
                    return out << other;
                }
            }}
        }
    };

    template<typename Self>
    concept Error = fst::error::Error<Self>
                 && requires(Self err,
                             const char *msg,
                             Unexpected unexp,
                             const fst::usize len,
                             const fst::str field,
                             const fst::str expected[]) {
        { Self::custom(msg) } -> std::convertible_to<Self>;
        { Self::invalid_type(unexp) } -> std::convertible_to<Self>;
        { Self::invalid_value(unexp) } -> std::convertible_to<Self>;
        { Self::invalid_length(len) } -> std::convertible_to<Self>;
        // NOTE: idk how to check the template
        /* { Self::unknown_field(field, expected) } -> std::convertible_to<Self>; */
        { Self::missing_field(field) } -> std::convertible_to<Self>;
        { Self::duplicate_field(field) } -> std::convertible_to<Self>;
    };
}
}

#define _SER_FIELD(field) TRY(state->serialize_field(#field, self.field));

#define SERIALIZE(NAME, ...)                                  \
template<typename S>                                          \
struct serde::ser::Serialize<NAME, S> {                       \
    fst::result::Result<typename S::Ok, typename S::Error>    \
    operator()(NAME &self, S &serializer) {                   \
        using fst::result::Ok;                                \
        using fst::result::Err;                               \
        auto state = TRY(serializer.serialize_struct(#NAME)); \
        FOREACH(_SER_FIELD, __VA_ARGS__);                     \
        TRY(state->end());                                    \
        return Ok();                                          \
    }                                                         \
}

#define FIELD_VISITOR(FIELD)                  \
    if (value == #FIELD) {                    \
        return fst::result::Ok(Field::FIELD); \
    }

#define FIELD_NAME(FIELD) #FIELD,

#define MAP_VISITOR_TEMPORARY_INIT(FIELD) \
    fst::option::Option<decltype(OUTPUT::FIELD)> FIELD = fst::option::None();

#define MAP_VISITOR_CASE(FIELD)                                         \
    case Field::FIELD:                                                  \
        if (FIELD.is_some()) {                                          \
            return fst::result::Err(V::Error::duplicate_field(#FIELD)); \
        }                                                               \
        FIELD = fst::option::Some(                                      \
                TRY(map.template next_value<decltype(OUTPUT::FIELD)>()));\
        break;

#define MAP_VISITOR_RETURN(FIELD) \
    .FIELD = TRY(FIELD.ok_or_else<typename V::Error>([](){  \
                    return V::Error::missing_field(#FIELD); \
                })),

#define DESERIALIZE(TYPE, ...)                                                          \
template<typename D>                                                                    \
struct serde::de::Deserialize<TYPE, D> {                                                \
    using OUTPUT = TYPE;                                                                \
    fst::result::Result<TYPE, typename D::Error>                                        \
    operator()(D &deserializer) {                                                       \
        return deserializer.deserialize_struct(#TYPE, FIELDS, TYPE##Visitor{});         \
    };                                                                                  \
    struct TYPE##Visitor {                                                              \
        using Value = TYPE;                                                             \
        template<typename V>                                                            \
        fst::result::Result<TYPE, typename V::Error> visit_map(V map) {                 \
            FOREACH(MAP_VISITOR_TEMPORARY_INIT, __VA_ARGS__)                            \
            for (auto key = TRY(map.template next_key<Field>());                        \
                    key.is_some();                                                      \
                    key = TRY(map.template next_key<Field>())) {                        \
                switch (key.unwrap()) {                                                 \
                    FOREACH(MAP_VISITOR_CASE, __VA_ARGS__)                              \
                }                                                                       \
            }                                                                           \
            return fst::result::Ok(                                                     \
                    TYPE {                                                              \
                        FOREACH(MAP_VISITOR_RETURN, __VA_ARGS__)                        \
                    });                                                                 \
        }                                                                               \
    };                                                                                  \
    constexpr static fst::str FIELDS[] = {                                              \
        FOREACH(FIELD_NAME, __VA_RGS__)                                                 \
    };                                                                                  \
    enum class Field { __VA_ARGS__ };                                                   \
};                                                                                      \
template<typename D>                                                                    \
struct serde::de::Deserialize<typename serde::de::Deserialize<TYPE, D>::Field, D> {     \
    using Struct = serde::de::Deserialize<TYPE, D>;                                     \
    using Field = typename Struct::Field;                                               \
    fst::result::Result<Field, typename D::Error> operator()(D &deserializer) {         \
        struct FieldVisitor {                                                           \
            using Value = Field;                                                        \
            fst::result::Result<Value, typename D::Error> visit_str(fst::str value) {   \
                FOREACH(FIELD_VISITOR, __VA_ARGS__)                                     \
                return fst::result::Err(D::Error::unknown_field(value, Struct::FIELDS));\
            }                                                                           \
        };                                                                              \
        return deserializer.deserialize_identifier(FieldVisitor());                     \
    }                                                                                   \
}

#endif // SERDE_H_
