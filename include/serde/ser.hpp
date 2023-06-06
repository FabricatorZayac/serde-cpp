#ifndef SERDE_SER_H_
#define SERDE_SER_H_

#include <concepts>
#include <string>

#include "fst/fst.hpp"

namespace serde {
namespace ser {
    template<typename Self>
    concept Error = fst::error::Error<Self>
                 && requires(Self err, const char *msg) {
        { Self::custom(msg) } -> std::same_as<Self>;
    };
}

namespace detail::archetypes {
    namespace ser {
        struct Error {
            std::string description();
            static Error custom(const char *);
        };
        struct SerializeStruct {
            using Ok = void;
            using Error = Error;

            template<typename T>
            fst::result::Result<Ok, Error>
            serialize_field(const fst::str, const T &value);
            
            fst::result::Result<Ok, Error> end();
        };
        struct Serializer {
            using Ok = void;
            using Error = Error;
            using SerializeStruct = SerializeStruct;

            fst::result::Result<Ok, Error> serialize_bool(const bool &);

            fst::result::Result<Ok, Error> serialize_short(const short &);
            fst::result::Result<Ok, Error> serialize_int(const int &);
            fst::result::Result<Ok, Error> serialize_long(const long &);
            fst::result::Result<Ok, Error> serialize_long_long(const long long &);

            fst::result::Result<Ok, Error> serialize_float(const float&);
            fst::result::Result<Ok, Error> serialize_double(const double&);

            fst::result::Result<Ok, Error> serialize_str(const fst::str &);
            fst::result::Result<SerializeStruct *, Error>
            serialize_struct(const fst::str &, const fst::usize);
        };
        struct Serializable;
    }
}

namespace ser {
    template<typename S>
    concept SerializeStruct =
    requires(S serializer,
             const fst::str &key,
             const detail::archetypes::ser::Serializable &value) {
        typename S::Ok;
        requires ser::Error<typename S::Error>;
        { serializer.end() } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_field(key, value) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
    };
    template<typename S>
    concept Serializer =
    requires(S serializer,
             const bool &Bool,
             const short &Short,
             const int &Int,
             const long &Long,
             const long long &LongLong,
             const float &Float,
             const double &Double,
             const fst::str &Str,
             const fst::str &name,
             fst::usize len) {
        typename S::Ok;
        typename S::SerializeStruct;
        requires Error<typename S::Error>;
        { serializer.serialize_bool(Bool) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_short(Short) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_int(Int) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_long(Long) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_long_long(LongLong) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_float(Float) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_double(Double) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_str(Str) } -> std::same_as<
        fst::result::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_struct(name, len) } -> std::same_as<
        fst::result::Result<typename S::SerializeStruct *, typename S::Error>>;
    };
}

namespace ser {
    template<typename T, Serializer S>
    struct Serialize;

    template<Serializer S>
    struct Serialize<detail::archetypes::ser::Serializable, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const detail::archetypes::ser::Serializable &self, S &serializer);
    };

    template<Serializer S>
    struct Serialize<bool, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const bool &self, S &serializer) {
            return serializer.serialize_bool(self);
        }
    };
    
    template<Serializer S>
    struct Serialize<short, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const short &self, S &serializer) {
            return serializer.serialize_int(self);
        }
    };
    template<Serializer S>
    struct Serialize<int, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const int &self, S &serializer) {
            return serializer.serialize_int(self);
        }
    };
    template<Serializer S>
    struct Serialize<long, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const long &self, S &serializer) {
            return serializer.serialize_long(self);
        }
    };
    template<Serializer S>
    struct Serialize<long long, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const long long &self, S &serializer) {
            return serializer.serialize_long_long(self);
        }
    };

    template<Serializer S>
    struct Serialize<float, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const float &self, S &serializer) {
            return serializer.serialize_float(self);
        }
    };
    template<Serializer S>
    struct Serialize<double, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const double &self, S &serializer) {
            return serializer.serialize_double(self);
        }
    };

    template<Serializer S>
    struct Serialize<fst::str, S> {
        static fst::result::Result<typename S::Ok, typename S::Error>
        serialize(const fst::str &self, S &serializer) {
            return serializer.serialize_str(self);
        }
    };
}

namespace ser {
    template<typename T>
    concept Serializable =
    requires(T &self, detail::archetypes::ser::Serializer &serializer) {
        Serialize<T, detail::archetypes::ser::Serializer>::serialize(self, serializer);
    };
}
}

#endif // !SERDE_SER_H_
