#ifndef FST_H_
#define FST_H_

#include <concepts>
#include <string>

#include "datatype_macros.hpp"

namespace fst {
    namespace error {
        template<typename T>
        concept Error = requires(T err) {
            { err.description() } -> std::same_as<std::string>;
        };
    }
};

#endif // !FST_H_
