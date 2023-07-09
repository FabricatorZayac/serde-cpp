#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "serde/de.hpp"
#include "serde_json/error.hpp"
#include "serde_json/json.hpp"

#include <ftl.hpp>

#define _DEBUG_FIELD(x) << ", " << #x << ": " << self.x

#define DEBUG_OSTREAM(T, FIRST, ...)                                             \
    inline std::ostream &operator<<(ftl::Debug &&debug, T &self) {               \
        return debug.out << #T << " { " << #FIRST << ": " << debug << self.FIRST \
        FOREACH(_DEBUG_FIELD, __VA_ARGS__) << " }";                              \
    }

struct RGB {
    int r;
    int g;
    int b;
};
SERIALIZE(RGB, r, g, b);
DESERIALIZE(RGB, r, g, b);
DEBUG_OSTREAM(RGB, r, g, b)

struct ColoredText {
    RGB color;
    ftl::str text;
};
SERIALIZE(ColoredText, color, text);
DESERIALIZE(ColoredText, color, text);
DEBUG_OSTREAM(ColoredText, color, text)

/* static_assert(serde::de::Visitor< */
/*         serde::de::Deserialize<RGB>::Visitor, */
/*         serde_json::error::Error>); */

using namespace std;

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    cout << serde_json::to_string(color).unwrap() << endl;
    cout << serde_json::to_string(foo).unwrap() << endl;

    switch (auto ___self =
                serde_json::from_str<RGB>(R"({"r":0,"g":255,"b":123})");
            ___self.tag) {
    case decltype(___self)::Tag::Ok: {
        auto res = ___self.unwrap();
        std::cout << ftl::debug << res << std::endl;
    } break;
    case decltype(___self)::Tag::Err: {
        auto err = ___self.unwrap_err();
        std::cout << "Error: " << err.description() << std::endl;
    }
    }

    switch (auto ___self = serde_json ::from_str<ColoredText>(
                R"({"color":{"r":5,"g":25,"b":30},"text":"baz"})");
            ___self.tag) {
    case decltype(___self)::Tag::Ok: {
        auto res = ___self.unwrap();
        std::cout << ftl::debug << res << std::endl;
    } break;
    case decltype(___self)::Tag::Err: {
        auto err = ___self.unwrap_err();
        std::cout << "Error: " << err.description() << std::endl;
    }
    }

    return 0;
}
