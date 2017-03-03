/*
 * Tests for Public API
 * This simply tests for API visibility and availability. It does not contain
 * functionality tests.
 */

#include <stdlib.h>
#include "c-bitmap.h"
#include "c-macro.h"
#include "c-ref.h"
#include "c-string.h"
#include "c-syscall.h"
#include "c-usec.h"

static void test_ref_release(_Atomic unsigned long *ref, void *userdata) {
        assert(userdata == (void *)0xdeadbeefUL);

        assert(!c_ref_inc_unless_zero(ref));
        assert(!c_ref_add_unless_zero(ref, 16));

        *ref = (_Atomic unsigned long)C_REF_INIT;
        c_ref_add(ref, 15);
}

static void test_ref(void) {
        _Atomic unsigned long ref = C_REF_INIT;

        assert(ref == 1);
        c_ref_inc(&ref);
        assert(ref == 2);
        c_ref_add(&ref, 14);
        assert(ref == 16);
        c_ref_dec(&ref, NULL, NULL);
        assert(ref == 15);
        c_ref_sub(&ref, 13, c_ref_unreachable, NULL);
        assert(ref == 2);

        ref = (_Atomic unsigned long)C_REF_INIT;
        assert(ref == 1);

        c_ref_inc_unless_zero(&ref);
        assert(ref == 2);
        c_ref_add_unless_zero(&ref, 2);
        assert(ref == 4);
        c_ref_sub(&ref, 4, test_ref_release, (void *)0xdeadbeefUL);
        assert(ref == 16);
}

static void test_string(void) {
        assert(!c_string_equal("foo", "bar"));
        assert(!c_string_prefix("foo", "bar"));
}

static void test_syscall(void) {
        int r;

        r = c_syscall_clone(~0UL, NULL);
        assert(r < 0);

        r = c_syscall_memfd_create(NULL, ~0U);
        assert(r < 0);

        r = c_syscall_gettid();
        assert(r >= 0);
}

static void test_usec(void) {
        uint64_t u_time;

        u_time = c_usec_from_clock(CLOCK_MONOTONIC);
        u_time = c_usec_from_nsec(u_time);
        u_time = c_usec_from_msec(u_time);
        u_time = c_usec_from_sec(u_time);
        u_time = c_usec_from_timespec(&(struct timespec){});
        u_time = c_usec_from_timeval(&(struct timeval){});
}

int main(int argc, char **argv) {
        test_ref();
        test_string();
        test_syscall();
        test_usec();
        return 0;
}
