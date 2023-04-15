#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "error.hpp"
#include "json.hpp"
#include "option.hpp"
#include "result.hpp"
#include "serde.hpp"
#include "str.hpp"

struct RGB {
    int r;
    int g;
    int b;
    SERIALIZE(RGB, r, g, b)

    template<typename D> // Deserializer
    result::Result<RGB, typename D::Error> deserialize(D deserializer) {
        using namespace result;

        struct FieldImpl {
            enum class Field { r, g, b, };

            Result<Field, typename D::Error> deserialize(D deserializer) {
                struct FieldVisitor : serde::de::Visitor<Field> {
                    using Value = Field;

                    result::Result<Value, error::Error> visit_str(str value) override {
                        if (value == "r") {
                            return Ok(Field::r);
                        }
                        if (value == "g") {
                            return Ok(Field::g);
                        }
                        if (value == "b") {
                            return Ok(Field::b);
                        }
                        return Err("Unknown field");
                    }
                };
                deserializer.deserialize_identifier(FieldVisitor());
            }
        };
    }
    private:
    struct RGBVisitor : serde::de::Visitor<RGB> {
        using Value = RGB;

        template<typename V>
        result::Result<RGB, typename V::Error> visit_map(V map) {
            using namespace option;
            Option<decltype(RGB::r)> r = None();
            Option<decltype(RGB::g)> g = None();
            Option<decltype(RGB::b)> b = None();
        }
    };
};

struct ColoredText {
    RGB color;
    const char *text;
    SERIALIZE(ColoredText, color, text)
};

// Example of function taking array
template<typename T, int N>
void parray(T const (&arr)[N]) {
    for (int i = 0; i < N; i++) {
        std::cout << arr[i] << ' ';
    }
    std::cout << std::endl;
}

result::Result<double, error::Error> checked_div(double a, double b) {
    if (b == 0) return result::Err(error::Error("div by zero"));
    else return result::Ok(a / b);
}

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    assert(json::from(color).unwrap() == R"({"r":255,"g":0,"b":172})");
    assert(json::from(foo).unwrap() == R"({"color":{"r":255,"g":0,"b":172},"text":"bar"})");

    using json::de::Deserializer;
    assert(Deserializer("true").parse_bool().unwrap() == true);
    assert(Deserializer("false").parse_bool().unwrap() == false);
    assert(Deserializer("123").parse_unsigned<unsigned int>().unwrap() == 123);
    assert(Deserializer("-123").parse_signed<int>().unwrap() == -123);
    assert(Deserializer("\"qwerty\"").parse_string().unwrap().equals(str("qwerty", 6)));

    parray({1, 2, 3, 4});
    std::cout << checked_div(4, 0).err().unwrap().to_str() << std::endl;

    json::de::StructVisitor<RGB> visitor;

    return 0;
}
