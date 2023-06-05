#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "fst/datatype_macros.hpp"
#include "fst/fst.hpp"
#include "serde/serde.hpp"
#include "serde_json/json.hpp"

struct RGB {
    int r;
    int g;
    int b;
};
SERIALIZE(RGB, r, g, b);
DESERIALIZE(RGB, r, g, b);

struct ColoredText {
    RGB color;
    fst::str text;
};
SERIALIZE(ColoredText, color, text);
DESERIALIZE(ColoredText, color, text);

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    std::cout << serde_json::to_string(color).unwrap() << std::endl;
    std::cout << serde_json::to_string(foo).unwrap() << std::endl;

    match(serde_json::from_str<RGB>(R"({"r":0,"g":255,"b":123})")) {{
        of(Ok, (res)) {
            std::cout << "RGB "
                << "{ r: " << res.r
                << ", g: " << res.g
                << ", b: " << res.b << " }" << std::endl;
        }
        of(Err, (err)) {
            std::cout << "Error: " << err.description() << std::endl;
        }
    }}

    match(serde_json::from_str<ColoredText>(R"({"color":{"r":5,"g":25,"b":30},"text":"baz"})")) {{
        of(Ok, (res)) {
            std::cout << "ColoredText "
                << "{ color: "
                << "RGB { r: " << res.color.r
                    << ", g: " << res.color.g
                    << ", b: " << res.color.b
                << " }, text: " << res.text << " }" << std::endl;
        }
        of(Err, (err)) {
            std::cout << "Error: " << err.description() << std::endl;
        }
    }}

    return 0;
}
