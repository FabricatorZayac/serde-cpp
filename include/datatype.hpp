#ifndef DATATYPE_H_
#define DATATYPE_H_

#define match(EXPR)        \
    auto ___self = EXPR;   \
    switch (___self.tag)   \

#define of(TAG, ...)                                     \
    break;}                                              \
    case TAG:{                                           \
        __VA_OPT__(auto __VA_ARGS__ = ___self.body.TAG); \

#endif // DATATYPE_H_
