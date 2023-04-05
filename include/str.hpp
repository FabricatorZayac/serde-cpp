#ifndef STR_H_
#define STR_H_

#include <cstring>
#include <ostream>
class str {
    const char *body;
    int length;
public:
    str(const char *body, int length) : body(body), length(length) {}
    str(const char *body) : body(body), length(strlen(body)) {}
    friend std::ostream& operator<<(std::ostream &stream, str self) {
        for (int i = 0; i < self.length; i++) {
            stream << self.body[i];
        }
        return stream;
    }
    bool equals(const str &other) {
        if (other.length != this->length) return false;
        for (int i; i < this->length; i++) {
            if (this->body[i] != other.body[i]) return false;
        }
        return true;
    }
};

#endif // STR_H_
