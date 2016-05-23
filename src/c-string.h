#pragma once

/***
  This file is part of c-sundry. See COPYING for details.

  c-sundry is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  c-sundry is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with c-sundry; If not, see <http://www.gnu.org/licenses/>.
***/

#include <c-macro.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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
