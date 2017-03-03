#pragma once

/*
 * String Helpers
 *
 * This is a collection of helper functions operating on standard C strings
 * (i.e., zero-terminated character arrays). They extend the ISO str*() class
 * of function and behave in a similar manner.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <c-macro.h>
#include <stdlib.h>

/**
 * c_string_equal() - compare strings for equality
 * @a:          first string to compare, or NULL
 * @b:          second string to compare, or NULL
 *
 * Compare to strings for equality, the same way strcmp() does it.
 * Additionally, NULL is allowed as input and compares equal to itself only.
 * Unlike strcmp(), this returns a boolean.
 *
 * Return: True if both are equal, false if not.
 */
_c_pure_ static inline bool c_string_equal(const char *a, const char *b) {
        return (!a || !b) ? (a == b) : !strcmp(a, b);
}

/**
 * c_string_prefix() - check prefix of a string
 * @str:        string to check
 * @prefix:     prefix to look for
 *
 * This checks whether @str starts with @prefix. If it does, a pointer to the
 * first character in @str after the prefix is returned, if not, NULL is
 * returned.
 *
 * Return: Pointer directly behind the prefix in @str, or NULL if not found.
 */
_c_pure_ static inline char *c_string_prefix(const char *str, const char *prefix) {
        size_t l = strlen(prefix);
        return !strncmp(str, prefix, l) ? (char *)str + l : NULL;
}

#ifdef __cplusplus
}
#endif
