#include <array>
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

#define DEBUG(SIG) DEBUG_STRUCT SIG

#define DERIVE(SIG, ...) APPLYEACH(SIG, __VA_ARGS__)

struct RGB {
    int r;
    int g;
    int b;
};
DERIVE((RGB, r, g, b), DEBUG, SERIALIZE, DESERIALIZE)

struct ColoredText {
    RGB color;
    ftl::str text;
};
DERIVE((ColoredText, color, text), DEBUG, SERIALIZE, DESERIALIZE)

/* static_assert(serde::de::Visitor< */
/*         serde::de::Deserialize<RGB>::Visitor, */
/*         serde_json::error::Error>); */

using namespace std;
using namespace ftl;

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    cout << debug << serde_json::to_string(color) << endl
         << debug << serde_json::to_string(foo) << endl
         << debug << serde_json::to_string(5) << endl
         << debug << serde_json::to_string(Slice{69, 420}) << endl
         << debug << serde_json::to_string((int[]){420, 69}) << endl
         << debug << serde_json::to_string(Slice<const RGB>{
                 {255, 255, 255},
                 {  0,   0,   0},
             }) << endl
         << debug << serde_json::to_string(array{42, 96}) << endl
         << debug << serde_json::to_string(vector{96, 42}) << endl
         << debug << serde_json::to_string(Some(69)) << endl
         << debug << serde_json::to_string(None()) << endl;

    cout << debug << serde_json::from_str<RGB>(R"({"r":0,"g":255,"b":123})") << endl;
    cout << debug << serde_json::from_str<ColoredText>(R"({"color":{"r":5,"g":25,"b":30},"text":"baz"})") << endl;
    cout << debug << serde_json::from_str<array<int, 2>>("[69,420]") << endl;

    return 0;
}
