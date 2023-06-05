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

template <typename D> struct serde ::de ::Deserialize<ColoredText, D> {
    using OUTPUT = ColoredText;
    fst ::result ::Result<ColoredText, typename D ::Error>
    operator()(D &deserializer) {
      return deserializer.deserialize_struct("ColoredText", FIELDS,
                                             ColoredTextVisitor{});
    };
    struct ColoredTextVisitor {
      using Value = ColoredText;
      template <typename V>
      fst ::result ::Result<ColoredText, typename V ::Error> visit_map(V map) {
        fst ::option ::Option<decltype(OUTPUT ::color)> color =
            fst ::option ::None();
        fst ::option ::Option<decltype(OUTPUT ::text)> text =
            fst ::option ::None();
        for (auto key = ({
               auto ___temp = map.template next_key<Field>();
               if (___temp.is_err())
                 return fst ::result ::Err(___temp.unwrap_err());
               ___temp.unwrap();
             });
             key.is_some(); key = ({
                              auto ___temp = map.template next_key<Field>();
                              if (___temp.is_err())
                                return fst ::result ::Err(___temp.unwrap_err());
                              ___temp.unwrap();
                            })) {
          switch (key.unwrap()) {
          case Field ::color:
            if (color.is_some()) {
              return fst ::result ::Err(V ::Error ::duplicate_field("color"));
            }
            color = fst ::option ::Some(({
              auto ___temp =
                  map.template next_value<decltype(OUTPUT ::color)>();
              if (___temp.is_err())
                return fst ::result ::Err(___temp.unwrap_err());
              ___temp.unwrap();
            }));
            break;
          case Field ::text:
            if (text.is_some()) {
              return fst ::result ::Err(V ::Error ::duplicate_field("text"));
            }
            text = fst ::option ::Some(({
              auto ___temp = map.template next_value<decltype(OUTPUT ::text)>();
              if (___temp.is_err())
                return fst ::result ::Err(___temp.unwrap_err());
              ___temp.unwrap();
            }));
            break;
          }
        }
        return fst ::result ::Ok(ColoredText{
            .color = ({
              auto ___temp = color.ok_or_else<typename V ::Error>(
                  []() { return V ::Error ::missing_field("color"); });
              if (___temp.is_err())
                return fst ::result ::Err(___temp.unwrap_err());
              ___temp.unwrap();
            }),
            .text = ({
              auto ___temp = text.ok_or_else<typename V ::Error>(
                  []() { return V ::Error ::missing_field("text"); });
              if (___temp.is_err())
                return fst ::result ::Err(___temp.unwrap_err());
              ___temp.unwrap();
            }),
        });
      }
    };
    constexpr static fst ::str FIELDS[] = {
        "color", "text"
    };
    enum class Field { color, text };
};
template <typename D>
struct serde ::de ::Deserialize<
    typename serde ::de ::Deserialize<ColoredText, D>::Field, D> {
    using Struct = serde ::de ::Deserialize<ColoredText, D>;
    using Field = typename Struct ::Field;
    fst ::result ::Result<Field, typename D ::Error>
    operator()(D &deserializer) {
      struct FieldVisitor {
        using Value = Field;
        fst ::result ::Result<Value, typename D ::Error>
        visit_str(fst ::str value) {
          if (value == "color") {
            return fst ::result ::Ok(Field ::color);
          }
          if (value == "text") {
            return fst ::result ::Ok(Field ::text);
          }
          return fst ::result ::Err(
              D ::Error ::unknown_field(value, Struct ::FIELDS));
        }
      };
      return deserializer.deserialize_identifier(FieldVisitor());
    }
};

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

    std::cout << serde_json::from_str<fst::str>("\"asd\"").unwrap() << std::endl;

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
