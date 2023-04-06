#ifndef OPTION_H_
#define OPTION_H_

#include "datatype.hpp"
#include <cstdlib>

namespace option {
    DATA_INITIALIZER_TYPES((Some, T), (None));

    template<typename T>
    struct Option {
        data(Option, (Some, T), (None));
        T unwrap() {
            if (this->tag == Tag::Some) return this->Some_val;
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

#endif // !OPTION_H_
