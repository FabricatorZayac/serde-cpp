#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "datatype_macros.hpp"
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

result::Result<double, error::Error> checked_div(double a, double b) {
    if (b == 0) return result::Err(error::Error("div by zero"));
    else return result::Ok(a / b);
}

int main() {
    RGB color{0xFF, 0x00, 0xAC};
    ColoredText foo{color, "bar"};

    std::cout << json::from(color).unwrap() << std::endl;
    std::cout << json::from(foo).unwrap() << std::endl;
    assert(json::from(color).unwrap() == R"({"r":255,"g":0,"b":172})");
    assert(json::from(foo).unwrap() == R"({"color":{"r":255,"g":0,"b":172},"text":"bar"})");

    using json::de::Deserializer;
    assert(Deserializer("true").parse_bool().unwrap() == true);
    assert(Deserializer("false").parse_bool().unwrap() == false);
    assert(Deserializer("123").parse_unsigned<unsigned int>().unwrap() == 123);
    assert(Deserializer("-123").parse_signed<int>().unwrap() == -123);
    assert(Deserializer("\"qwerty\"").parse_string().unwrap().equals(str("qwerty", 6)));

    double a = 5, b = 2;
    /* std::cin >> a >> b; */
    match(checked_div(a, b)) {{
        of(Ok, (x)) {
            std::cout << "Division result is: " << x << std::endl;
        }
        of(Err, (e)) {
            std::cout << e.to_str() << std::endl;
        }
    }}
    match(checked_div(b, a)) {{
        of(Ok, (x)) {
            std::cout << "Division result is: " << x << std::endl;
        }
        of(Err, (e)) {
            std::cout << e.to_str() << std::endl;
        }
    }}

    return 0;
}
