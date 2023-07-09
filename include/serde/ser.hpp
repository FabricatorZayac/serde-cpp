#ifndef SERDE_SER_H_
#define SERDE_SER_H_

#include <concepts>
#include <string>

#include "fst/fst.hpp"
#include <ftl.hpp>
#include <type_traits>

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
    struct SerializeSeq {
        using Ok = void;
        using Error = Error;

        template<typename T>
        ftl::Result<void, Error>
        serialize_element(const T &value);
        
        ftl::Result<Ok, Error> end_seq();
    };
    struct Serializer {
        using Ok = void;
        using Error = Error;
        using SerializeStruct = SerializeStruct;
        using SerializeSeq = SerializeSeq;

        ftl::Result<Ok, Error> serialize_bool(const bool &);

        ftl::Result<Ok, Error> serialize_char(const char &);

        ftl::Result<Ok, Error> serialize_short(const short &);
        ftl::Result<Ok, Error> serialize_int(const int &);
        ftl::Result<Ok, Error> serialize_long(const long &);
        ftl::Result<Ok, Error> serialize_long_long(const long long &);

        ftl::Result<Ok, Error> serialize_float(const float&);
        ftl::Result<Ok, Error> serialize_double(const double&);

        ftl::Result<Ok, Error> serialize_str(const ftl::str &);

        ftl::Result<SerializeStruct &, Error>
        serialize_struct(const ftl::str &, const size_t);
        ftl::Result<SerializeSeq &, Error>
        serialize_seq(ftl::Option<size_t>);
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
    concept SerializeSeq =
    requires(S serializer,
             const detail::archetypes::ser::Serializable &value) {
        typename S::Ok;
        requires ser::Error<typename S::Error>;

        { serializer.serialize_element(value) } -> std::same_as<
        ftl::Result<void, typename S::Error>>;
        { serializer.end_seq() } -> std::same_as<
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
             size_t len,
             ftl::Option<size_t> opt_len) {
        typename S::Ok;
        requires Error<typename S::Error>;

        requires SerializeStruct<typename S::SerializeStruct>;
        requires SerializeSeq<typename S::SerializeSeq>;

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
        ftl::Result<typename S::SerializeStruct &, typename S::Error>>;
        { serializer.serialize_seq(opt_len) } -> std::same_as<
        ftl::Result<typename S::SerializeSeq &, typename S::Error>>;
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

    template<typename T, typename S = detail::archetypes::ser::Serializer>
    concept Serializable =
    requires(const T &self, S &serializer) {
        { Serialize<T>::serialize(self, serializer) }
        -> std::same_as<ftl::Result<typename S::Ok, typename S::Error>>;
    } or requires(const T &self, S &serializer) {
        { Serialize<std::decay_t<T>>::serialize(self, serializer) }
        -> std::same_as<ftl::Result<typename S::Ok, typename S::Error>>;
    };

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
            return serializer.serialize_short(self);
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

    template<Serializable T>
    struct Serialize<ftl::Slice<T>> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const ftl::Slice<T> &self, S &serializer) {
            typename S::SerializeSeq &state = 
                TRY(serializer.serialize_seq(ftl::Some(self.len())));
            for (auto &e : self) {
                TRY(state.serialize_element(e));
            }
            return state.end_seq();
        }
    };

    template<Serializable T, size_t N>
    struct Serialize<T[N]> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const T (&self)[N], S &serializer) {
            typename S::SerializeSeq &state = 
                TRY(serializer.serialize_seq(ftl::Some(N)));
            for (auto &e : self) {
                TRY(state.serialize_element(e));
            }
            return state.end_seq();
        }
    };
    
    template<Serializable T>
    struct Serialize<ftl::Option<T>> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const ftl::Option<T> &self, S &serializer) {
            if (self.is_some()) return serializer.serialize_some(self.unwrap());
            return serializer.serialize_none();
        }
    };
    template<>
    struct Serialize<ftl::Option<void>> {
        template<Serializer S>
        static ftl::Result<typename S::Ok, typename S::Error>
        serialize(const ftl::Option<void> &self, S &serializer) {
            (void)self;
            return serializer.serialize_none();
        }
    };
}
}

#endif // !SERDE_SER_H_
