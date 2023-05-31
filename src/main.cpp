#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>
#include <assert.h>

#include "datatype_macros.hpp"
#include "json.hpp"
#include "serde.hpp"
#include "fst.hpp"

struct RGB {
    int r;
    int g;
    int b;
    SERIALIZE(RGB, r, g, b);
    DESERIALIZE(RGB, r, g, b);

private:
    struct RGBVisitor : serde::de::Visitor<RGB> {
        using Value = RGB;

        template<typename V>
        fst::result::Result<RGB, typename V::Error> visit_map(V map) {
            fst::option::Option<decltype(RGB::r)> r = fst::option::None();
            fst::option::Option<decltype(RGB::g)> g = fst::option::None();
            fst::option::Option<decltype(RGB::b)> b = fst::option::None();
            while (auto key = TRY(map.next_key())) {
                switch (key) {
                case Field::r:
                    if (r.is_some()) {
                        return fst::result::Err("Duplicate field `r`");
                    }
                    r = Some(TRY(map.next_value()));
                    break;
                case Field::g:
                    if (g.is_some()) {
                        return fst::result::Err("Duplicate field `g`");
                    }
                    g = Some(TRY(map.next_value()));
                    break;
                case Field::b:
                    if (b.is_some()) {
                        return fst::result::Err("Duplicate field `b`");
                    }
                    b = Some(TRY(map.next_value()));
                    break;
                }
            }
            auto _r = TRY(r.ok_or_else<fst::error::Error>(
                        [](){
                            return fst::error::Error("Missing field `r`");
                        }));
            auto _g = TRY(g.ok_or_else<fst::error::Error>(
                        [](){
                            return fst::error::Error("Missing field `g`");
                        }));
            auto _b = TRY(b.ok_or_else<fst::error::Error>(
                        [](){
                            return fst::error::Error("Missing field `b`");
                        }));
            return fst::result::Ok(
                    RGB {
                        .r = _r,
                        .g = _g,
                        .b = _b
                    });
        }
    };
};

struct ColoredText {
    RGB color;
    const char *text;
    SERIALIZE(ColoredText, color, text)
};

fst::result::Result<double, fst::error::Error> checked_div(double a, double b) {
    if (b == 0) return fst::result::Err(fst::error::Error("div by zero"));
    else return fst::result::Ok(a / b);
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
    assert(Deserializer("\"qwerty\"").parse_string().unwrap() == "qwerty");

    auto des = Deserializer(R"({"r":0,"g":255,"b":123})");
    RGB col;
    col.deserialize(des);

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

    fst::Fn<fst::result::Result<double, fst::error::Error>, double, double>
        auto div = checked_div;
    match(div(a, b)) {{
        of(Ok, (x)) {
            std::cout << "Division result is: " << x << std::endl;
        }
        of(Err, (e)) {
            std::cout << e.to_str() << std::endl;
        }
    }}

    fst::Fn<int, int, int> auto add = [](int a, int b){ return a + b; };
    assert(add(4, 7) == 11);

    fst::Index auto arr0 = fst::arr<int, 5>{5, 4, 3, 2, 1};
    int ar[] = {5, 4, 3, 2, 1};
    fst::Index auto arr1 = ar;
    assert(arr0[3] == 2);
    assert(arr1[2] == 3);

    fst::option::Option opt(fst::option::Some(5));
    std::cout
        << opt.ok_or_else<fst::error::Error>(
                [](){
                    return fst::error::Error("kek");
                }).unwrap()
        << std::endl;

    opt = fst::option::None();
    std::cout
        << opt.ok_or_else<fst::error::Error>(
                [](){
                    return fst::error::Error("kek");
                }).unwrap_err().to_str()
        << std::endl;

    return 0;
}
