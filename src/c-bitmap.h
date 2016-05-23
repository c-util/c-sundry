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
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * c_bitmap_test() - test bit in bitmap
 * @bitmap:     bitmap
 * @bit:        bit to test
 *
 * This tests whether bit @bit is set in the bitmap @bitmap. The bitmap is
 * treated as an array of bytes, and @bit is the index of the bit to test (thus
 * starting at 0).
 *
 * The caller is responsible for range checks. This function assumes the bitmap
 * is big enough to hold bit @bit.
 *
 * Return: True if the bit is set, false if not.
 */
static inline bool c_bitmap_test(const void *bitmap, unsigned int bit) {
        return *((const uint8_t *)bitmap + bit / 8) & (1 << (bit % 8));
}

static inline void c_bitmap_set(void *bitmap, unsigned int bit) {
        *((uint8_t *)bitmap + bit / 8) |= (1 << (bit % 8));
}

static inline void c_bitmap_clear(void *bitmap, unsigned int bit) {
        *((uint8_t *)bitmap + bit / 8) &= ~(1 << (bit % 8));
}

static inline void c_bitmap_set_all(void *bitmap, unsigned int n_bits) {
        memset(bitmap, 0xff, c_div_round_up(n_bits, 8));
}

static inline void c_bitmap_clear_all(void *bitmap, unsigned int n_bits) {
        memset(bitmap, 0, c_div_round_up(n_bits, 8));
}

#ifdef __cplusplus
}
#endif
