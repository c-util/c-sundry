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
 * c_string_compare() - compare two strings
 * @a:          first string to compare, or NULL
 * @b:          second string to compare, or NULL
 *
 * Compare two strings, the same way strcmp() does it.
 * Additionally, NULL is allowed as input, which compares equal to itself
 * and smaller than any other string.
 *
 * Return: Less than, greater than or equal to zero, as strcmp().
 */
_c_pure_ static inline int c_string_compare(const char *a, const char *b) {
        if (a == b)
                return 0;

        return (!a || !b) ? (a ? 1 : -1) : strcmp(a, b);
}

/**
 * c_string_equal() - compare strings for equality
 * @a:          first string to compare, or NULL
 * @b:          second string to compare, or NULL
 *
 * Compare two strings for equality, the same way strcmp() does it.
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

/**
 * c_string_to_hex() - encode string as ascii-hex
 * @str:        string to encode from
 * @n:          length of @str in bytes
 * @hex:        destination buffer
 *
 * This hex-encodes the source string into the destination buffer. The
 * destination buffer must be at least twice as big as the source.
 */
static inline void c_string_to_hex(const char *str, size_t n, char *hex) {
        static const char table[16] = "0123456789abcdef";

        while (n--) {
                *hex++ = table[(*str >> 4) & 0x0f];
                *hex++ = table[(*str++) & 0x0f];
        }
}

/**
 * c_string_from_hex() - decode ascii-hex string
 * @str:        string buffer to write into
 * @n:          length of @str in bytes
 * @hex:        hex encoded buffer to decode
 *
 * This hex-decodes @hex into the string buffer @str. Be aware that @hex must
 * be twice the size as @str / @n.
 *
 * Return: True if successful, false if invalid.
 */
static inline bool c_string_from_hex(char *str, size_t n, const char *hex) {
        static const uint8_t table[128] = {
                 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9,  -1,  -1, -1,  -1,  -1,  -1,
                 -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                 -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
                 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        };
        uint8_t v1, v2;

        for ( ; n; --n, hex += 2) {
                v1 = table[hex[0] & 0x7f];
                v2 = table[hex[1] & 0x7f];
                if (_c_unlikely_((hex[0] | hex[1] | v1 | v2) & 0x80))
                        return false;

                *str++ = (v1 << 4) | v2;
        }

        return true;
}

/**
 * c_string_verify_ascii() - verify that a scring is ASCII encoded
 * @strp:               pointer to string to verify
 * @lenp:               pointer to length of string
 *
 * The first @lenp bytes of the string pointed to by @strp is
 * verified to be ASCII encoded, and @strp and @lenp are updated to
 * point to the first non-ASCII character or the first NULL of the
 * sting, and the remaining number of bytes of the string, respectively.
 */
static inline void c_string_verify_ascii(char **strp, size_t *lenp) {
        unsigned char *str = (unsigned char *) *strp;
        size_t len = *lenp;

        while (len > 0) {
                if (_c_unlikely_(*str == 0x00 || *str > 0x7F))
                        break;

                ++str;
                --len;
        }

        *strp = (char *) str;
        *lenp = len;
}

/**
 * c_string_verify_utf8() - verify that a scring is UTF-8 encoded
 * @strp:               pointer to string to verify
 * @lenp:               pointer to length of string
 *
 * Up to the first @lenp bytes of the string pointed to by @strp is
 * verified to be UTF-8 encoded, and @strp and @lenp are updated to
 * point to the first non-UTF-8 character or the first NULL of the
 * string, and the remaining number of bytes of the string,
 * respectively.
 */
static inline void c_string_verify_utf8(char **strp, size_t *lenp) {
        unsigned char *str = (unsigned char *)*strp;
        size_t len = *lenp;

        /* See Unicode 9.0.0, Chapter 3, Section D92 */

        while (len > 0) {
                if (_c_unlikely_(*str == 0x00)) {
                        break;
                } else if (*str < 0x80) {
                        ++str;
                        --len;
                } else if (_c_unlikely_(*str < 0xC2)) {
                        break;
                } else if (*str < 0xE0) {
                        if (_c_unlikely_(len < 2))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;

                        str += 2;
                        len -= 2;
                } else if (*str < 0xE1) {
                        if (_c_unlikely_(len < 3))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0xA0 || *(str + 1) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xED) {
                        if (_c_unlikely_(len < 3))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xEE) {
                        if (_c_unlikely_(len < 3))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x9F))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xF0) {
                        if (_c_unlikely_(len < 3))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xF1) {
                        if (_c_unlikely_(len < 4))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x90 || *(str + 1) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else if (*str < 0xF4) {
                        if (_c_unlikely_(len < 4))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else if (*str < 0xF5) {
                        if (_c_unlikely_(len < 4))
                                break;
                        if (_c_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x8F))
                                break;
                        if (_c_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_c_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else
                        break;
        }

        *strp = (char *)str;
        *lenp = len;
}

#ifdef __cplusplus
}
#endif
