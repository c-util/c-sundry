/*
 * Tests for Dynamic Bitmaps
 * Bunch of tests for the dynamic bitmap module.
 */

#include <stdlib.h>
#include "c-bitmap.h"
#include "c-macro.h"

/* test c_bitmap_* helpers */
static void test_bitmap(void) {
        uint8_t bitmap[] = {
                0xff, 0x00,
                0x80, 0xf0,
                0x04, 0xff,
                0x00, 0x00,

                0xff, 0xff,
                0x00, 0x00,
                0x00, 0x00,
                0xff, 0xff,
        };
        unsigned int i, j;

        /*
         * Verify the bitmap test/set functions work correctly, given our
         * pre-initialized bitmap.
         */

        /* 0-15 */
        for (i = 0; i < 8; ++i)
                assert(c_bitmap_test(bitmap, i));
        for ( ; i < 16; ++i)
                assert(!c_bitmap_test(bitmap, i));

        /* 16-31 */
        for ( ; i < 23; ++i)
                assert(!c_bitmap_test(bitmap, i));
        assert(c_bitmap_test(bitmap, i++));
        for ( ; i < 28; ++i)
                assert(!c_bitmap_test(bitmap, i));
        for ( ; i < 32; ++i)
                assert(c_bitmap_test(bitmap, i));

        /* 32-47 */
        for ( ; i < 34; ++i)
                assert(!c_bitmap_test(bitmap, i));
        assert(c_bitmap_test(bitmap, i++));
        for ( ; i < 40; ++i)
                assert(!c_bitmap_test(bitmap, i));
        for ( ; i < 48; ++i)
                assert(c_bitmap_test(bitmap, i));

        /* 48-63 */
        for ( ; i < 64; ++i)
                assert(!c_bitmap_test(bitmap, i));

        /* 64-79 */
        for ( ; i < 80; ++i)
                assert(c_bitmap_test(bitmap, i));

        /* 80-95 */
        for ( ; i < 96; ++i)
                assert(!c_bitmap_test(bitmap, i));

        /* 96-111 */
        for ( ; i < 112; ++i)
                assert(!c_bitmap_test(bitmap, i));

        /* 112-127 */
        for ( ; i < 128; ++i)
                assert(c_bitmap_test(bitmap, i));

        /*
         * Verify that set_all/clear_all works correctly on our bitmap. Make
         * sure to run both twice, to verify it works even on uninitialized
         * maps.
         */

        c_bitmap_set_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i)
                assert(c_bitmap_test(bitmap, i));

        c_bitmap_clear_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i)
                assert(!c_bitmap_test(bitmap, i));

        c_bitmap_set_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i)
                assert(c_bitmap_test(bitmap, i));

        c_bitmap_clear_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i)
                assert(!c_bitmap_test(bitmap, i));

        /*
         * Verify that set/clear affect only a single bit. We do this by
         * setting/clearing just a single bit, while keeping all other set to
         * the inverse. Then we check that just a single bit was flipped.
         */

        c_bitmap_clear_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i) {
                c_bitmap_set(bitmap, i);
                for (j = 0; j < sizeof(bitmap) * 8; ++j)
                        assert(c_bitmap_test(bitmap, j) == (i == j));
                c_bitmap_clear(bitmap, i);
                for (j = 0; j < sizeof(bitmap) * 8; ++j)
                        assert(!c_bitmap_test(bitmap, j));
        }

        c_bitmap_set_all(bitmap, sizeof(bitmap) * 8);
        for (i = 0; i < sizeof(bitmap) * 8; ++i) {
                c_bitmap_clear(bitmap, i);
                for (j = 0; j < sizeof(bitmap) * 8; ++j)
                        assert(c_bitmap_test(bitmap, j) == (i != j));
                c_bitmap_set(bitmap, i);
                for (j = 0; j < sizeof(bitmap) * 8; ++j)
                        assert(c_bitmap_test(bitmap, j));
        }
}

int main(int argc, char **argv) {
        test_bitmap();
        return 0;
}
