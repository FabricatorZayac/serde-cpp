#include "fst.hpp"

namespace fst::error {
    Error::Error() : message(0) {}
    Error::Error(const char *message) : message(message) {}
    const char *Error::to_str() {
        return this->message;
    }
}
