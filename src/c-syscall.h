#pragma once

/*
 * Syscall Wrappers
 *
 * The linux syscalls are usually not directly accessible from applications,
 * since most standard libraries do not provide wrapper functions. This module
 * provides direct syscall wrappers via `syscall(3)' for a set of otherwise
 * unavailable syscalls.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/resource.h>
#include <sys/syscall.h>

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
        /* Make Travis happy. */
#if defined __NR_memfd_create
        long nr = __NR_memfd_create;
#elif defined __x86_64__
        long nr = 319;
#elif defined __i386__
        long nr = 356;
#else
#  error "__NR_memfd_create is undefined"
#endif
        return (int)syscall(nr, name, flags);
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
        return (int)syscall(SYS_gettid);
}

#ifdef __cplusplus
}
#endif
