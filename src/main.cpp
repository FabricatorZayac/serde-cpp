#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "error.hpp"
#include "json.hpp"
#include "result.hpp"
#include "str.hpp"

using json::ser::Serializer;

struct RGB {
    int r;
    int g;
    int b;
    SERIALIZE(r, g, b)
};

struct ColoredText {
    RGB color;
    const char *text;
    SERIALIZE(color, text)
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
    if (b == 0) return {Err, error::Error("div by zero")};
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
    assert(Deserializer("\"qwerty\"").parse_string().unwrap().equals(str("qwerty")));

    parray({1, 2, 3, 4});
    std::cout << checked_div(4, 0).Err.to_str() << std::endl;

    return 0;
}
