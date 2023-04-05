#ifndef OPTION_H_
#define OPTION_H_

#include <cstdlib>

#pragma clang diagnostic ignored "-Wc++20-extensions"

namespace option {
    enum class Tag { Some, None };
    template<typename T>
    class Option {
        Option();
    public:
        union {
            T Some;
        };
        enum Tag tag;
        Option(T some) : tag(Tag::Some), Some(some) {}
        // temporary kinda shit solution
        // TODO: think of a better way to construct empty variants
        Option(enum Tag tag = Tag::None) : tag(Tag::None) {}
        ~Option() {
            if (this->is_some()) this->Some.~T();
        }
        T unwrap() {
            if (this->tag == Tag::Some) return this->Some;
            exit(1);
        }
        bool is_some() {
            return this->tag == Tag::Some ? true : false;
        }
        bool is_none() {
            return this->tag == Tag::None ? true : false;
        }
    };
}
using enum option::Tag; // Kinda evil, but I don't know how to make match work otherwise

#endif // !OPTION_H_
