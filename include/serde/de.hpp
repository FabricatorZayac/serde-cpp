#ifndef SERDE_DE_H_
#define SERDE_DE_H_

#include "fst/fst.hpp"
#include <concepts>

namespace serde {
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
        { Self::custom(msg) } -> std::same_as<Self>;
        { Self::invalid_type(unexp) } -> std::same_as<Self>;
        { Self::invalid_value(unexp) } -> std::same_as<Self>;
        { Self::invalid_length(len) } -> std::same_as<Self>;
        // NOTE: idk how to check the template
        /* { Self::unknown_field(field, expected) } -> std::convertible_to<Self>; */
        { Self::missing_field(field) } -> std::same_as<Self>;
        { Self::duplicate_field(field) } -> std::same_as<Self>;
    };
    template<typename A>
    concept MapAccess =
    requires(A map) {
        requires Error<typename A::Error>;
    };
}

namespace detail::archetypes::de {
    struct Error {
        std::string description();
        static Error custom(const char *);

        static Error invalid_type(serde::de::Unexpected);
        static Error invalid_value(serde::de::Unexpected);
        static Error invalid_length(fst::usize);
        template<fst::usize N>
        static Error unknown_field(const fst::str, const fst::str[N]);
        static Error missing_field(const fst::str);
        static Error duplicate_field(const fst::str);
    };
    struct MapAccess {
        using Error = Error;

        template<typename K>
        fst::result::Result<fst::option::Option<typename K::Value>, Error>
        next_key_seed(K);
    };
    template<typename T>
    struct Visitor {
        using Value = T;

        fst::result::Result<Value, Error> visit_bool(bool);

        fst::result::Result<Value, Error> visit_char(char);

        fst::result::Result<Value, Error> visit_short(short);
        fst::result::Result<Value, Error> visit_int(int);
        fst::result::Result<Value, Error> visit_long(long);
        fst::result::Result<Value, Error> visit_long_long(long long);

        fst::result::Result<Value, Error> visit_float(float);
        fst::result::Result<Value, Error> visit_double(double);

        fst::result::Result<Value, Error> visit_str(fst::str);

        fst::result::Result<Value, Error> visit_map(MapAccess);
    };
    struct Deserializer {

    };
    struct Deserializable {

    };
}

namespace de {
    template<typename V, typename E>
    concept Visitor =
    requires(V visitor,
             bool Bool,
             char Char,
             short Short,
             int Int,
             long Long,
             long long LongLong,
             float Float,
             double Double,
             fst::str Str,
             detail::archetypes::de::MapAccess map) {
        typename V::Value;
        requires Error<E>;
        { visitor.visit_bool(Bool) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_char(Char) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_short(Short) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_int(Int) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_long(Long) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_long_long(LongLong) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_float(Float) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_double(Double) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_str(Str) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
        { visitor.visit_map(map) } ->
        std::same_as<fst::result::Result<typename V::Value, E>>;
    };

    template<typename D>
    concept Deserializer =
    requires(D deserializer,
             detail::archetypes::de::Visitor<void> visitor,
             const char *name,
             const fst::str fields[]) {
        requires fst::error::Error<typename D::Error>;
        { deserializer.deserialize_any(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_bool(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_short(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_int(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_long(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_long_long(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_ushort(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_uint(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_ulong(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_ulong_long(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_str(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_identifier(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_map(visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_struct(name, fields, visitor) } -> std::same_as<
        fst::result::Result<typename decltype(visitor)::Value, typename D::Error>>;
    };
}

namespace de {
    template<typename T, Deserializer D>
    struct Deserialize;

    template<Deserializer D>
    struct Deserialize<int, D> {
        static fst::result::Result<int, typename D::Error>
        deserialize(D &deserializer) {
            struct IntVisitor :
                detail::archetypes::de::Visitor<int> {
                using Value = int;
                fst::result::Result<Value, typename D::Error>
                visit_int(int value) {
                    return fst::result::Ok(value);
                }
            };
            return deserializer.deserialize_int(IntVisitor{});
        }
    };

    template<Deserializer D>
    struct Deserialize<fst::str, D> {
        static fst::result::Result<fst::str, typename D::Error>
        deserialize(D &deserializer) {
            struct StrVisitor :
                detail::archetypes::de::Visitor<fst::str> {
                using Value = fst::str;
                fst::result::Result<Value, typename D::Error>
                visit_str(fst::str value) {
                    return fst::result::Ok(value);
                }
            };
            return deserializer.deserialize_str(StrVisitor{});
        }
    };
}
}

#endif // !SERDE_DE_H_
