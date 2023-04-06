#ifndef DATATYPE_H_
#define DATATYPE_H_

#include "cursed_macros.h"

#define __FIELD(x, ...) __VA_OPT__(CAR(__VA_ARGS__) x;)
#define _FIELD(x) __FIELD x

// VALUE is (TAG, TYPE)

/******************************************************************************/
#define _DATA_CONSTRUCTOR_BODY(TAG, ...) \
    (__VA_OPT__(CAR(__VA_ARGS__) x)) : tag(Tag::TAG) __VA_OPT__(, TAG(x)) {}

#define DATA_CONSTRUCTOR_BODY(VALUE) _DATA_CONSTRUCTOR_BODY VALUE
#define DATA_CONSTRUCTOR(TYPENAME) TYPENAME DATA_CONSTRUCTOR_BODY
/******************************************************************************/

/******************************************************************************/
#define _DATA_COPY_CONSTRUCTOR(TAG, ...)                       \
    case Tag::TAG:                                             \
    __VA_OPT__(new (&this->TAG) CAR(__VA_ARGS__)(source.TAG);) \
    break;
#define DATA_COPY_CONSTRUCTOR(VALUE) _DATA_COPY_CONSTRUCTOR VALUE
/******************************************************************************/

/******************************************************************************/
#define _DATA_DESTRUCTOR(TAG, ...)             \
    case Tag::TAG:                             \
    __VA_OPT__(this->TAG.~CAR(__VA_ARGS__)();) \
               break;
#define DATA_DESTRUCTOR(VALUE) _DATA_DESTRUCTOR VALUE
/******************************************************************************/

/******************************************************************************/
#define _DATA_PUBLIC_CONSTRUCTOR(TAG, ...) \
    case Tag::TAG:                         \
    __VA_OPT__(new(this) CDR(__VA_ARGS__)(args...));   \
    break;

#define DATA_PUBLIC_CONSTRUCTOR(VALUE) _DATA_PUBLIC_CONSTRUCTOR VALUE
/******************************************************************************/

/**
 * @brief   Generic definition of a Rust-like Enum (sum type)
 * @details Usage:
 *              data(NAME,
 *                   (TAG, TYPE),
 *                   ...)
 * @param   NAME name of the surrounding struct
 * @param   ... Type definition
 */
#define data(NAME, ...)                                   \
    public:                                               \
    union {                                               \
        FOREACH(_FIELD, __VA_ARGS__)                      \
    };                                                    \
    enum Tag tag;                                         \
    NAME(const NAME &source) : tag(source.tag) {          \
        switch (source.tag) {                             \
            FOREACH(DATA_COPY_CONSTRUCTOR, __VA_ARGS__)   \
        }                                                 \
    }                                                     \
    ~NAME() {                                             \
        switch (this->tag) {                              \
            FOREACH(DATA_DESTRUCTOR, __VA_ARGS__)         \
        }                                                 \
    }

/**
 * @brief   Rust-like match statement (not an expression!)
 * @details Match statement only works with enums directly accessible in the namespace.
 *          Cannot specify multiple patterns of same tag with different predicates;
 *          that behavior is to be handled in #of_default.
 *          Usage:
 *              match(EXPR) {{
 *                  of(TAG, ...)
 *                  ...
 *                  [of_default]
 *              }}
 * @param   EXPR Expression returning a tagged union
 */
#define match(EXPR)      \
    auto ___self = EXPR; \
    switch (___self.tag)

/**
 * @brief Specifies branch of match statement
 * @details Usage:
 *          of(TAG, ...) statement
 * @param TAG enum value to match to;
 * @param ... [(TEMP[, (PREDICATE)])]
 *            If predicate doesn't hold, forwards to of_default
 */
#define of(TAG, ...)                                        \
    break;}                                                 \
    case TAG:{                                              \
        __VA_OPT__(auto CAR __VA_ARGS__ = ___self.TAG; \
                   _PATTERN_PREDICATE(CDR __VA_ARGS__))

#define _PATTERN_PREDICATE(...) \
    __VA_OPT__(if (!(CAR __VA_ARGS__)) goto ___default;)

/**
 * @brief default brainch of match statement (kinda like _ in Rust)
 * @details Usage:
 *          of_default statement
 */
#define of_default \
    break;}        \
    default:{ ___default:

#endif // DATATYPE_H_
