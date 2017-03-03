#pragma once

/*
 * Time Handling
 *
 * The `time_t' set of functions are usually of little use if you need precise
 * timing in the sub-second range. This module implements helpers to deal with
 * time-related operations with microsecond precision. A `uint64_t' is used as
 * datatype.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

/* uint64_t stores up to 584,942.417355 years in microseconds */
#define c_usec_from_nsec(_nsec) ((_nsec) / UINT64_C(1000))
#define c_usec_from_msec(_msec) ((_msec) * UINT64_C(1000))
#define c_usec_from_sec(_sec) c_usec_from_msec((_sec) * UINT64_C(1000))
#define c_usec_from_timespec(_ts) (c_usec_from_sec((_ts)->tv_sec) + c_usec_from_nsec((_ts)->tv_nsec))
#define c_usec_from_timeval(_tv) (c_usec_from_sec((_tv)->tv_sec) + (_tv)->tv_usec)

/**
 * c_usec_from_clock() - read current clock value
 * @clock:              ID of clock to read
 *
 * This reads the current value of the clock specified via @clock. The value is
 * returned in microsecond precision. The caller must guarantee that the clock
 * is valid and available on the machine.
 *
 * Return: Current clock value in microseconds.
 */
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
