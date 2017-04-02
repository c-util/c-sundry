/*
 * Tests for string manipulation helpers
 */

#include <stdlib.h>
#include "c-macro.h"
#include "c-string.h"

static void test_compare(void) {
        int r;

        r = c_string_compare(NULL, NULL);
        assert(r == 0);

        r = c_string_compare("", NULL);
        assert(r > 0);

        r = c_string_compare(NULL, "");
        assert(r < 0);

        r = c_string_compare("a", "a");
        assert(r == 0);

        r = c_string_compare("a", "b");
        assert(r < 0);
}

static void test_equal(void) {
        bool b;

        b = c_string_equal(NULL, NULL);
        assert(b);

        b = c_string_equal("", NULL);
        assert(!b);

        b = c_string_equal(NULL, "");
        assert(!b);

        b = c_string_equal("a", "a");
        assert(b);

        b = c_string_equal("a", "b");
        assert(!b);
}

static void test_verify_from_hex(const char *hex) {
        _c_cleanup_(c_freep) char *raw = NULL, *copy = NULL;
        bool valid_hex1, valid_hex2;
        size_t n_hex;

        /* a hex-string is valid _iff_ it consists of 0-9a-fA-F */

        n_hex = strlen(hex);
        raw = calloc(1, n_hex);
        copy = calloc(1, n_hex);
        assert(raw && copy);

        valid_hex1 = !(n_hex % 2) && (n_hex == strspn(hex, "0123456789abcdefABCDEF"));
        valid_hex2 = !(n_hex % 2) && c_string_from_hex(raw, n_hex / 2, hex);

        assert(valid_hex1 == valid_hex2);

        /* verify one round through c_string_from/to_hex() keeps the form */

        if (valid_hex2) {
                c_string_to_hex(raw, n_hex / 2, copy);
                assert(!strncasecmp(hex, copy, n_hex));
        }
}

/* test hex en/de-coders */
static void test_hex(void) {
        test_verify_from_hex("0");
        test_verify_from_hex("00");
        test_verify_from_hex("0a");
        test_verify_from_hex("a" "0");
        test_verify_from_hex("0123456789abcdefABCDEF");
        test_verify_from_hex("a" "\x01");
        test_verify_from_hex("\x01" "a");
}

int main(int argc, char **argv) {
        test_compare();
        test_equal();
        test_hex();
        return 0;
}
