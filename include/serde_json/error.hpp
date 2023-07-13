#ifndef JSON_ERROR_H_
#define JSON_ERROR_H_

#include "fst/cursed_macros.h"
#include "fst/fst.hpp"
#include "serde/ser.hpp"
#include "serde/de.hpp"
#include <concepts>
#include <ostream>
#include <sstream>
#include <ftl.hpp>

#define TAG_CONSTRUCTOR(TAG) \
    static Error TAG() { return Tag::TAG; }

#define __ENUM_INIT(...) \
    enum class Tag {     \
        __VA_ARGS__      \
    } tag;               \
    FOREACH(TAG_CONSTRUCTOR, __VA_ARGS__)

#define __ENUM_STRING_CASE(TAG) \
    case Tag::TAG:              \
        return #TAG;

namespace serde_json::error {
    struct Error {
        std::string description() const {
            switch (tag) {
            case Tag::Message:
                return msg;
            FOREACH(__ENUM_STRING_CASE, 
                    Eof,
                    Syntax,
                    ExpectedBoolean,
                    ExpectedInteger,
                    ExpectedString,
                    ExpectedNull,
                    ExpectedArray,
                    ExpectedArrayComma,
                    ExpectedArrayEnd,
                    ExpectedMap,
                    ExpectedMapColon,
                    ExpectedMapComma,
                    ExpectedMapEnd,
                    ExpectedEnum,
                    TrailingCharacters)
            }
        }

        static Error Message(const char *msg) { return msg; }
        __ENUM_INIT(
            Message,
            Eof,
            Syntax,
            ExpectedBoolean,
            ExpectedInteger,
            ExpectedString,
            ExpectedNull,
            ExpectedArray,
            ExpectedArrayComma,
            ExpectedArrayEnd,
            ExpectedMap,
            ExpectedMapColon,
            ExpectedMapComma,
            ExpectedMapEnd,
            ExpectedEnum,
            TrailingCharacters)
        static Error custom(const char *msg) { return Error::Message(msg); }

        static Error invalid_type(serde::de::Unexpected unexp) {
            std::stringstream msg;
            msg << "invalid type: " << unexp;
            return msg.str().c_str();
        }
        static Error invalid_value(serde::de::Unexpected unexp) {
            std::stringstream msg;
            msg << "invalid value: " << unexp;
            return msg.str().c_str();
        }
        static Error invalid_length(const size_t len) {
            std::stringstream msg;
            msg << "invalid length: " << len;
            return msg.str().c_str();
        }
        // NOTE: idfk how to check this with a concept
        static Error unknown_field(const ftl::str field,
                const ftl::Slice<const ftl::str> &expected) {
            std::stringstream msg;
            msg << "unknown field `" << field << "`, ";
            if (expected.len() == 0) {
                msg << "there are no fields";
            } else {
                msg << "expected one of ";
                msg << ftl::debug << expected;
            }
            return msg.str().c_str();
        }
        static Error missing_field(const ftl::str field) {
            std::stringstream msg;
            msg << "missing field: `" << field << '`';
            return msg.str().c_str();
        }
        static Error duplicate_field(const ftl::str field) {
            std::stringstream msg;
            msg << "duplicate field: `" << field << '`';
            return msg.str().c_str();
        }

        friend std::ostream &operator<<(ftl::Debug &&debug, const Error &self) {
            return debug.out << self.description();
        }
    private:
        Error(const char *msg) : tag(Tag::Message), msg(msg) {};
        Error(Tag tag) : tag(tag) {}
        std::string msg;
    };
    static_assert(serde::de::concepts::Error<Error>);

    template<typename T>
    using Result = ftl::Result<T, Error>;
}

#endif // !JSON_ERROR_H_
