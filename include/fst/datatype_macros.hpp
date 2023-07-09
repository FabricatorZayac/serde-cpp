#include "cursed_macros.h"

#ifndef DATATYPE_H_
#define DATATYPE_H_

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
#define match(EXPR) \
    switch (auto ___self = EXPR; ___self.tag)

/******************************************************************************/
#define OF_PATTERN_PREDICATE(...) \
    __VA_OPT__(if (!(CAR __VA_ARGS__)) goto ___default;)
/******************************************************************************/

/**
 * @brief Specifies branch of match statement
 * @details Usage:
 *          of(TAG, ...) statement
 * @param TAG enum value to match to;
 * @param ... [(TEMP[, (PREDICATE)])]
 *            If predicate doesn't hold, forwards to of_default
 */
#define of(TAG, ...)                                         \
    break;}                                                  \
    case decltype(___self)::Tag::TAG:{                       \
        __VA_OPT__(auto CAR __VA_ARGS__ = ___self.TAG##_val; \
                   OF_PATTERN_PREDICATE(CDR __VA_ARGS__))

/**
 * @brief default brainch of match statement (kinda like _ in Rust)
 * @details Usage:
 *          of_default statement
 */
#define of_default \
    break;}        \
    default:{ ___default:

#endif // DATATYPE_H_
