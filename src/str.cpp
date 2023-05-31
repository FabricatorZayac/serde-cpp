#include "fst.hpp"

namespace fst {
    str::str(const char *body, int length) : body(body), length(length) {}
    bool str::equals(const str &other) {
        if (other.length != this->length) return false;
        for (int i; i < this->length; i++) {
            if (this->body[i] != other.body[i]) return false;
        }
        return true;
    }
    bool str::equals(const char *other) {
        for (int i; i < this->length; i++) {
            if (this->body[i] != other[i]) return false;
        }
        return true;
    }
}
