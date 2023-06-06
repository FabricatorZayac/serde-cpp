#ifndef SERDE_DE_H_
#define SERDE_DE_H_

#include "fst/fst.hpp"

namespace serde {
namespace de {
    template<typename D>
    concept Deserializer =
    requires(D deserializer) {
        fst::error::Error<typename D::Error>;
    };
}

namespace de {
    template<typename T, Deserializer D>
    struct Deserialize;
    
    template<Deserializer D>
    struct Deserialize<int, D> {
        static fst::result::Result<int, typename D::Error>
        deserialize(D &deserializer) {
            struct IntVisitor {
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
            struct StrVisitor {
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

#endif // !SERDE_DE_H_
