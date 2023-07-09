#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include <ftl.hpp>
#include <vector>

#include "serde/macros.hpp"
#include "serde_json/error.hpp"
#include "serde_json/json.hpp"

#define _DEBUG_FIELD(x) << ", " << #x << ": " << self.x
#define DEBUG_STRUCT(T, FIRST, ...)                                              \
    inline std::ostream &operator<<(ftl::Debug &&debug, const T &self) {         \
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
DEBUG_STRUCT(RGB, r, g, b);

struct ColoredText {
    RGB color;
    ftl::str text;
};
SERIALIZE(ColoredText, color, text);
DESERIALIZE(ColoredText, color, text);
DEBUG_STRUCT(ColoredText, color, text);

/* static_assert(serde::de::Visitor< */
/*         serde::de::Deserialize<RGB>::Visitor, */
/*         serde_json::error::Error>); */

using namespace std;

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    cout << serde_json::to_string(color).unwrap() << endl
         << serde_json::to_string(foo).unwrap() << endl
         << serde_json::to_string(5).unwrap() << endl
         << serde_json::to_string(ftl::Slice{69, 420}).unwrap() << endl
         << serde_json::to_string((int[]){420, 69}).unwrap() << endl
         << serde_json::to_string(ftl::Slice<const RGB>{
                 {255, 255, 255},
                 {  0,   0,   0},
             }).unwrap() << endl
         << serde_json::to_string(ftl::Some(69)).unwrap() << endl
         << serde_json::to_string(ftl::None()).unwrap() << endl;

    switch (auto ___self =
                serde_json::from_str<RGB>(R"({"r":0,"g":255,"b":123})");
            ___self.tag) {
    case decltype(___self)::Tag::Ok: {
        auto res = ___self.unwrap();
        cout << ftl::debug << res << endl;
    } break;
    case decltype(___self)::Tag::Err: {
        auto err = ___self.unwrap_err();
        cout << "Error: " << err.description() << endl;
    }
    }

    switch (auto ___self = serde_json ::from_str<ColoredText>(
                R"({"color":{"r":5,"g":25,"b":30},"text":"baz"})");
            ___self.tag) {
    case decltype(___self)::Tag::Ok: {
        auto res = ___self.unwrap();
        cout << ftl::debug << res << endl;
    } break;
    case decltype(___self)::Tag::Err: {
        auto err = ___self.unwrap_err();
        cout << "Error: " << err.description() << endl;
    }
    }

    return 0;
}
