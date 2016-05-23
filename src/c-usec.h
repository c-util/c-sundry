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

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* uint64_t stores up to 584,942.417355 years in microseconds */
#define c_usec_from_nsec(_nsec) ((_nsec) / UINT64_C(1000))
#define c_usec_from_msec(_msec) ((_msec) * UINT64_C(1000))
#define c_usec_from_sec(_sec) c_usec_from_msec((_sec) * UINT64_C(1000))
#define c_usec_from_timespec(_ts) (c_usec_from_sec((_ts)->tv_sec) + c_usec_from_nsec((_ts)->tv_nsec))
#define c_usec_from_timeval(_tv) (c_usec_from_sec((_tv)->tv_sec) + (_tv)->tv_usec)

static inline uint64_t c_usec_from_clock(clockid_t clock) {
        struct timespec ts;
        int r;

        r = clock_gettime(clock, &ts);
        assert(r >= 0);
        return c_usec_from_timespec(&ts);
}

#ifdef __cplusplus
}
#endif
