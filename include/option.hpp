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
        union Body {
            T Some;
            Body(){}
        } body;
        enum Tag tag;
        Option(T some) {
            this->tag = Tag::Some;
            this->body.Some = some;
        }
        Option(enum Tag tag) {
            this->tag = Tag::None;
        }
        ~Option() {
            if (this->is_some()) {
                this->body.Some.~T();
            }
        }
        T unwrap() {
            if (this->tag == Tag::Some) return this->body.Some;
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
using enum option::Tag;

#endif // !OPTION_H_
