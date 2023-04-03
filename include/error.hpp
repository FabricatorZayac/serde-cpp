#ifndef ERROR_H_
#define ERROR_H_

#include <string>

namespace error {
    class Error {
        const char *message;
    public:
        Error(){}
        Error(const char *message) {
            this->message = message;
        }
        const char *to_str() {
            return this->message;
        }
    };
}

#endif // !ERROR_H_
