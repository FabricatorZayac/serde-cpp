#ifndef STR_H_
#define STR_H_

struct str {
    const char *body;
    int length;
    str(const char *body, int length) : body(body), length(length) {}
    bool equals(const str &other) {
        if (other.length != this->length) return false;
        for (int i; i < this->length; i++) {
            if (this->body[i] != other.body[i]) return false;
        }
        return true;
    }
    // Kinda shit but I don't care
    bool equals(const char *&other) {
        for (int i; i < this->length; i++) {
            if (this->body[i] != other[i]) return false;
        }
        return true;
    }
    template<typename T>
    bool operator==(const T &other) {
        return this->equals(other);
    }
};

#endif // STR_H_
