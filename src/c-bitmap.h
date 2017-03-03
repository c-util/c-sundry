#pragma once

/*
 * Dynamic Bitmaps
 *
 * This implements helpers to deal with dynamically sized bitmaps. Allocation
 * and management is left to the caller. This just provides convenience helpers
 * to set, clear, and test a single bit (or all bits).
 *
 * Bitmaps must be of type `uint8_t[]' (i.e., byte arrays). The LSB of the
 * first element of the array is addressed as bit `0'. The MSB of the last
 * element of the array is addressed as bit `n', where `n' is the number of
 * bits in the bitmap.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <c-macro.h>
#include <stdlib.h>

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

/**
 * c_bitmap_set() - set bit in bitmap
 * @bitmap:     bitmap
 * @bit:        bit to set
 *
 * This sets bit @bit in the bitmap @bitmap. The bitmap is treated as an array
 * of bytes, and @bit is the index of the bit to set (thus starting at 0).
 *
 * The caller is responsible for range checks. This function assumes the bitmap
 * is big enough to hold bit @bit.
 */
static inline void c_bitmap_set(void *bitmap, unsigned int bit) {
        *((uint8_t *)bitmap + bit / 8) |= (1 << (bit % 8));
}

/**
 * c_bitmap_clear() - clear bit in bitmap
 * @bitmap:     bitmap
 * @bit:        bit to clear
 *
 * This clears bit @bit in the bitmap @bitmap. The bitmap is treated as an array
 * of bytes, and @bit is the index of the bit to clear (thus starting at 0).
 *
 * The caller is responsible for range checks. This function assumes the bitmap
 * is big enough to hold bit @bit.
 */
static inline void c_bitmap_clear(void *bitmap, unsigned int bit) {
        *((uint8_t *)bitmap + bit / 8) &= ~(1 << (bit % 8));
}

/**
 * c_bitmap_set_all() - set all bits in bitmap
 * @bitmap:     bitmap
 * @n_bits:     number of bits
 *
 * This sets all bits in the bitmap @bitmap. The bitmap is treated as an array
 * of bytes, and @n_bits is the number of bits to set. @n_bits is rounded up to
 * the next multiple of 8.
 *
 * The caller is responsible for range checks. This function assumes the bitmap
 * is big enough to hold @n_bits bits.
 */
static inline void c_bitmap_set_all(void *bitmap, unsigned int n_bits) {
        memset(bitmap, 0xff, c_div_round_up(n_bits, 8));
}

/**
 * c_bitmap_clear_all() - clear all bits in bitmap
 * @bitmap:     bitmap
 * @n_bits:     number of bits
 *
 * This clears all bits in the bitmap @bitmap. The bitmap is treated as an array
 * of bytes, and @n_bits is the number of bits to clear. @n_bits is rounded up
 * to the next multiple of 8.
 *
 * The caller is responsible for range checks. This function assumes the bitmap
 * is big enough to hold @n_bits bits.
 */
static inline void c_bitmap_clear_all(void *bitmap, unsigned int n_bits) {
        memset(bitmap, 0, c_div_round_up(n_bits, 8));
}

#ifdef __cplusplus
}
#endif
