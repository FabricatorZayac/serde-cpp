#ifndef SERDE_DE_H_
#define SERDE_DE_H_

#include <concepts>
#include <cstddef>

#include <ftl.hpp>

#include "fst/fst.hpp"
#include "fst/datatype_macros.hpp"

namespace serde {
namespace de {
    struct Unexpected {
        static Unexpected Bool(bool Bool) { return Unexpected { .tag = Tag::Bool, .Bool_val = Bool }; }
        static Unexpected Unsigned(unsigned long long Unsigned) { return Unexpected { .tag = Tag::Unsigned, .Unsigned_val = Unsigned }; }
        static Unexpected Signed(long long Signed) { return Unexpected { .tag = Tag::Signed, .Signed_val = Signed }; }
        static Unexpected Float(float Float) { return Unexpected { .tag = Tag::Float, .Float_val = Float }; }
        static Unexpected Char(char Char) { return Unexpected { .tag = Tag::Char, .Char_val = Char }; }
        static Unexpected Str(ftl::str Str) { return Unexpected { .tag = Tag::Str, .Str_val = Str }; }
        static Unexpected Unit() { return Unexpected { .tag = Tag::Unit }; }
        static Unexpected Map() { return Unexpected { .tag = Tag::Unit }; }
        static Unexpected Other(ftl::str Other) { return Unexpected { .tag = Tag::Other, .Other_val = Other }; }

        enum class Tag {
            Bool,
            Unsigned,
            Signed,
            Float,
            Char,
            Str,
            Unit,
            Map,
            Other,
        } tag;
        union {
            bool Bool_val;
            unsigned long long Unsigned_val;
            long long Signed_val;
            double Float_val;
            char Char_val;
            ftl::str Str_val;
            ftl::str Other_val;
        };
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

    namespace concepts {
        template<typename Self>
        concept Error = fst::error::Error<Self>
                     && requires(Self err,
                                 const char *msg,
                                 Unexpected unexp,
                                 const size_t len,
                                 const ftl::str field,
                                 const ftl::Slice<const ftl::str> &expected) {
            { Self::custom(msg) } -> std::same_as<Self>;
            { Self::invalid_type(unexp) } -> std::same_as<Self>;
            { Self::invalid_value(unexp) } -> std::same_as<Self>;
            { Self::invalid_length(len) } -> std::same_as<Self>;
            { Self::unknown_field(field, expected) } -> std::convertible_to<Self>;
            { Self::missing_field(field) } -> std::same_as<Self>;
            { Self::duplicate_field(field) } -> std::same_as<Self>;
        };
        template<typename A>
        concept MapAccess =
        requires(A map) {
            requires Error<typename A::Error>;
        };
    }
}

namespace detail::archetypes::de {
    struct Error {
        std::string description();
        static Error custom(const char *);

        static Error invalid_type(serde::de::Unexpected);
        static Error invalid_value(serde::de::Unexpected);
        static Error invalid_length(size_t);
        static Error unknown_field(const ftl::str, const ftl::Slice<ftl::str>);
        static Error missing_field(const ftl::str);
        static Error duplicate_field(const ftl::str);
    };
    struct MapAccess {
        using Error = Error;

        template<typename K>
        ftl::Result<ftl::Option<typename K::Value>, Error>
        next_key_seed(K);
    };
    template<typename T>
    struct Visitor {
        using Value = T;

        ftl::Result<Value, Error> visit_bool(bool);

        ftl::Result<Value, Error> visit_char(char);

        ftl::Result<Value, Error> visit_short(short);
        ftl::Result<Value, Error> visit_int(int);
        ftl::Result<Value, Error> visit_long(long);
        ftl::Result<Value, Error> visit_long_long(long long);

        ftl::Result<Value, Error> visit_float(float);
        ftl::Result<Value, Error> visit_double(double);

        ftl::Result<Value, Error> visit_str(ftl::str);

        ftl::Result<Value, Error> visit_map(MapAccess);
    };
    struct Deserializer {
        using Error = Error;
    };
    struct Deserialize;
    struct DeserializeSeed;
}

namespace de::concepts {
    // NOTE: doesn't work
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
             ftl::str Str,
             detail::archetypes::de::MapAccess map) {
        typename V::Value;
        requires concepts::Error<E>;
        { visitor.visit_bool(Bool) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_char(Char) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_short(Short) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_int(Int) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_long(Long) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_long_long(LongLong) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_float(Float) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_double(Double) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_str(Str) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
        { visitor.visit_map(map) } ->
        std::same_as<ftl::Result<typename V::Value, E>>;
    };

    template<typename D>
    concept Deserializer =
    requires(D deserializer,
             detail::archetypes::de::Visitor<void> visitor,
             const char *name,
             const ftl::str fields[]) {
        requires concepts::Error<typename D::Error>;
        { deserializer.deserialize_any(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_bool(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_short(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_int(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_long(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_long_long(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_ushort(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_uint(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_ulong(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_ulong_long(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_str(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_identifier(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;

        { deserializer.deserialize_map(visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
        { deserializer.deserialize_struct(name, fields, visitor) } -> std::same_as<
        ftl::Result<typename decltype(visitor)::Value, typename D::Error>>;
    };
}

namespace de {
    template<typename T>
    struct Deserialize;
    template<typename T>
    struct DeserializeSeed;

    template<typename T>
    struct DeserializeSeed<ftl::PhantomData<T>> {
        using Value = T;

        template<typename D>
        static ftl::Result<T, typename D::Error>
        deserialize(const ftl::PhantomData<T> &self, D &deserializer) {
            (void)self;
            return Deserialize<T>::deserialize(deserializer);
        }
    };

    template<>
    struct Deserialize<short> {
        template<concepts::Deserializer D>
        static ftl::Result<short, typename D::Error>
        deserialize(D &deserializer) {
            struct ShortVisitor {
                using Value = short;
                ftl::Result<Value, typename D::Error>
                visit_short(int value) {
                    return ftl::Ok(value);
                }
            };
            return deserializer.deserialize_short(ShortVisitor{});
        }
    };
    template<>
    struct Deserialize<int> {
        template<concepts::Deserializer D>
        static ftl::Result<int, typename D::Error>
        deserialize(D &deserializer) {
            struct IntVisitor {
                using Value = int;
                ftl::Result<Value, typename D::Error>
                visit_int(int value) {
                    return ftl::Ok(value);
                }
            };
            return deserializer.deserialize_int(IntVisitor{});
        }
    };
    template<>
    struct Deserialize<long> {
        template<concepts::Deserializer D>
        static ftl::Result<long, typename D::Error>
        deserialize(D &deserializer) {
            struct LongVisitor {
                using Value = long;
                ftl::Result<Value, typename D::Error>
                visit_long(long value) {
                    return ftl::Ok(value);
                }
            };
            return deserializer.deserialize_long(LongVisitor{});
        }
    };
    template<>
    struct Deserialize<long long> {
        template<concepts::Deserializer D>
        static ftl::Result<long long, typename D::Error>
        deserialize(D &deserializer) {
            struct LongLongVisitor {
                using Value = long long;
                ftl::Result<Value, typename D::Error>
                visit_long_long(long long value) {
                    return ftl::Ok(value);
                }
            };
            return deserializer.deserialize_long_long(LongLongVisitor{});
        }
    };

    template<>
    struct Deserialize<ftl::str> {
        template<concepts::Deserializer D>
        static ftl::Result<ftl::str, typename D::Error>
        deserialize(D &deserializer) {
            struct StrVisitor {
                using Value = ftl::str;
                ftl::Result<Value, typename D::Error>
                visit_str(ftl::str value) {
                    return ftl::Ok(value);
                }
            };
            return deserializer.deserialize_str(StrVisitor{});
        }
    };
}

namespace de::concepts {
    template<typename T, typename D = detail::archetypes::de::Deserializer>
    concept Deserialize = requires(D &deserializer) {
        { Deserialize<T>::deserialize(deserializer) }
        -> std::same_as<ftl::Result<T, typename D::Error>>;
    };

    // template<typename T, typename D = detail::archetypes::de::Deserializer>
    // concept DeserializeSeed = requires(const T &seed, D &deserializer) {
    //     { DeserializeSeed<T>::deserialize(seed, deserializer) }
    //     -> std::same_as<ftl::Result<T, typename D::Error>>;
    // };
}
}

#endif // !SERDE_DE_H_
