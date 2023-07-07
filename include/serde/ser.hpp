#ifndef SERDE_SER_H_
#define SERDE_SER_H_

#include <concepts>
#include <string>

#include "fst/fst.hpp"
#include <ftl.hpp>

namespace serde {
namespace ser {
    template<typename Self>
    concept Error = fst::error::Error<Self>
                 && requires(Self err, const char *msg) {
        { Self::custom(msg) } -> std::same_as<Self>;
    };
}

namespace detail::archetypes::ser {
    struct Error {
        std::string description();
        static Error custom(const char *);
    };
    struct SerializeStruct {
        using Ok = void;
        using Error = Error;

        template<typename T>
        ftl::Result<Ok, Error>
        serialize_field(const ftl::str, const T &value);
        
        ftl::Result<Ok, Error> end();
    };
    struct Serializer {
        using Ok = void;
        using Error = Error;
        using SerializeStruct = SerializeStruct;

        ftl::Result<Ok, Error> serialize_bool(const bool &);

        ftl::Result<Ok, Error> serialize_char(const char &);

        ftl::Result<Ok, Error> serialize_short(const short &);
        ftl::Result<Ok, Error> serialize_int(const int &);
        ftl::Result<Ok, Error> serialize_long(const long &);
        ftl::Result<Ok, Error> serialize_long_long(const long long &);

        ftl::Result<Ok, Error> serialize_float(const float&);
        ftl::Result<Ok, Error> serialize_double(const double&);

        ftl::Result<Ok, Error> serialize_str(const ftl::str &);
        ftl::Result<SerializeStruct *, Error>
        serialize_struct(const ftl::str &, const size_t);
    };
    struct Serializable;
}

namespace ser {
    template<typename S>
    concept SerializeStruct =
    requires(S serializer,
             const ftl::str &key,
             const detail::archetypes::ser::Serializable &value) {
        typename S::Ok;
        requires ser::Error<typename S::Error>;
        { serializer.serialize_field(key, value) } -> std::same_as<
        ftl::Result<void, typename S::Error>>;
        { serializer.end() } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
    };
    template<typename S>
    concept Serializer =
    requires(S serializer,
             const bool &Bool,
             const char &Char,
             const short &Short,
             const int &Int,
             const long &Long,
             const long long &LongLong,
             const float &Float,
             const double &Double,
             const ftl::str &Str,
             const ftl::str &name,
             size_t len) {
        typename S::Ok;
        requires SerializeStruct<typename S::SerializeStruct>;
        requires Error<typename S::Error>;
        { serializer.serialize_bool(Bool) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_char(Char) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_short(Short) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_int(Int) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_long(Long) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_long_long(LongLong) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_float(Float) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_double(Double) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;

        { serializer.serialize_str(Str) } -> std::same_as<
        ftl::Result<typename S::Ok, typename S::Error>>;
        { serializer.serialize_struct(name, len) } -> std::same_as<
        ftl::Result<typename S::SerializeStruct *, typename S::Error>>;
    };
}

namespace ser {
    template<typename T>
    struct Serialize;

    template<>
    struct Serialize<detail::archetypes::ser::Serializable> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const detail::archetypes::ser::Serializable &self, S &serializer);
    };

    // NOTE: impl Serialize for bool where S: Serializer
    template<>
    struct Serialize<bool> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const bool &self, S &serializer) {
            return serializer.serialize_bool(self);
        }
    };

    template<>
    struct Serialize<short> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const short &self, S &serializer) {
            return serializer.serialize_int(self);
        }
    };
    template<>
    struct Serialize<int> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const int &self, S &serializer) {
            return serializer.serialize_int(self);
        }
    };
    template<>
    struct Serialize<long> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const long &self, S &serializer) {
            return serializer.serialize_long(self);
        }
    };
    template<>
    struct Serialize<long long> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const long long &self, S &serializer) {
            return serializer.serialize_long_long(self);
        }
    };

    template<>
    struct Serialize<float> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const float &self, S &serializer) {
            return serializer.serialize_float(self);
        }
    };
    template<>
    struct Serialize<double> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const double &self, S &serializer) {
            return serializer.serialize_double(self);
        }
    };

    template<>
    struct Serialize<ftl::str> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const ftl::str &self, S &serializer) {
            return serializer.serialize_str(self);
        }
    };
}

namespace ser {
    template<typename T, typename S = detail::archetypes::ser::Serializer>
    concept Serializable =
    requires(const T &self, S &serializer) {
        { Serialize<T>::serialize(self, serializer) }
        -> std::same_as<ftl::Result<typename S::Ok, typename S::Error>>;
    };
}
}

#endif // !SERDE_SER_H_
