#pragma once

/***
  This file is part of bus1. See COPYING for details.

  bus1 is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  bus1 is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with bus1; If not, see <http://www.gnu.org/licenses/>.
***/

#include <linux/memfd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * c_syscall_clone() - wrapper for raw clone(2) syscall
 * @flags:              clone flags
 * @child_stack:        location of the stack used by the child process
 *
 * This is a wrapper for the raw kernel clone() syscall. The raw clone()
 * corresponds more closely to fork(2) in that execution in the child
 * continues from the point of the call.
 *
 * Return: The thread ID of the child process on success, -1 on failure.
 */
static inline int c_syscall_clone(unsigned long flags, void *child_stack) {
#if defined(__s390__) || defined(__CRIS__)
        return (int)syscall(__NR_clone, child_stack, flags);
#else
        return (int)syscall(__NR_clone, flags, child_stack);
#endif
}

/**
 * c_syscall_memfd_create() - wrapper for memfd_create(2) syscall
 * @name:       name for memfd inode
 * @flags:      memfd flags
 *
 * This is a wrapper for the memfd_create(2) syscall. Currently, no user-space
 * wrapper is exported by any libc.
 *
 * Return: New memfd file-descriptor on success, -1 on failure.
 */
static inline int c_syscall_memfd_create(const char *name, unsigned int flags) {
        return syscall(__NR_memfd_create, name, flags);
}

/**
 * c_syscall_gettid() - wrapper for gettid(2) syscall
 *
 * This is a wrapper for the gettid(2) syscall. Currently, no user-space
 * wrapper is exported by any libc.
 *
 * Return: Thread ID of calling process.
 */
static inline int c_syscall_gettid(void) {
        return syscall(SYS_gettid);
}

#ifdef __cplusplus
}
#endif
