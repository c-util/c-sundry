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

/*
 * Tests for Public API
 * This simply tests for API visibility and availability. It does not contain
 * functionality tests.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-bitmap.h"
#include "c-macro.h"
#include "c-string.h"
#include "c-syscall.h"
#include "c-usec.h"

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
        test_string();
        test_syscall();
        test_usec();
        return 0;
}
