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
#include <stdatomic.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRef CRef;
typedef void (*CRefFn) (CRef *ref, void *userdata);

/**
 * struct CRef - atomic reference counter
 * @n_refs:             current number of references owned
 *
 * The CRef object implements an atomic reference counter to be embedded in
 * other objects. It supports lockless acquire/release operations between
 * multiple threads.
 *
 * The reference counter is initialized to 1. Once it drops to 0, no more
 * references can be acquired and the object can be released asynchronously.
 * The current number of references can be fetched by reading @n_refs directly,
 * or optionally via atomic_load_explicit(&ref->n_refs, memory_order_relaxed).
 */
struct CRef {
        _Atomic unsigned long n_refs;
};

/**
 * C_REF_INIT - initialize static reference counter
 *
 * This provides a static initializer for a CRef object. It is meant to be used
 * as assignment for static variables. To initialize non-static variables, cast
 * the lvalue to CRef before assigning it.
 */
#define C_REF_INIT { .n_refs = ATOMIC_VAR_INIT(1UL), }

/**
 * c_ref_add() - acquire references
 * @ref:                reference counter to operate on, or NULL
 * @n:                  number of references to acquire
 *
 * Acquire @n references to the reference counter @ref. The caller must
 * guarantee that they already own a reference to @n. Furthermore, the caller
 * must ensure that this cannot overflow 'unsigned long' (usually it is enough
 * to ensure that a reference is associated with some allocated object).
 *
 * If @ref is NULL, this is a no-op. @n must not be 0.
 *
 * Return: @ref is returned.
 */
static inline CRef *c_ref_add(CRef *ref, unsigned long n) {
        unsigned long n_refs;

        assert(n > 0);

        if (ref) {
                /*
                 * Acquire references but do not place any barriers. Nobody
                 * should place decisions based on this operations, ever! So no
                 * need to order it.
                 */
                n_refs = atomic_fetch_add_explicit(&ref->n_refs, n,
                                                   memory_order_relaxed);
                assert(n_refs > 0);
        }

        return ref;
}

/**
 * c_ref_add_unless_zero() - acquire references if possible
 * @ref:                reference counter to operate on, or NULL
 * @n:                  number of references to acquire
 *
 * Acquire @n references to the reference counter @ref, if, and only if, it has
 * not already dropped to 0. In case of success, this has the same effect as
 * c_ref_add(). In case of failure, this will return NULL without acquiring any
 * reference. The caller must check the return value of this function.
 *
 * This function does not give any memory ordering guarantees. Just like
 * c_ref_add(), no decision should be placed based on the fact that a reference
 * has been acquired. That is, even if this returns NULL, the caller must not
 * try to deduce the state of the surrounding object. Furthermore, the caller
 * must provide sufficient synchronization on the pointer to the object. This
 * function just makes sure to never acquire references to possibly unlocked
 * objects that have already been released. Any further synchronization is up
 * to the caller.
 *
 * If @ref is NULL, this is a no-op. @n must not be 0.
 *
 * Return: @ref is returned on success, otherwise NULL is returned.
 */
static inline CRef *c_ref_add_unless_zero(CRef *ref, unsigned long n) {
        unsigned long n_refs;

        assert(n > 0);

        if (ref) {
                /*
                 * Try replacing ref->n_refs with (n->n_refs + n). This
                 * requires us to fetch the value and loop on cmpxchg. On
                 * failure, bail out with NULL. Otherwise, retry until we
                 * completed the operation.
                 * Note that we do not provide barriers. We expect the caller
                 * to synchronize via the actual pointer of the object. If this
                 * function fails, the caller should not use this information
                 * to deduce the state of the object. It must rely on external
                 * synchronization, if that is required.
                 */
                n_refs = atomic_load_explicit(&ref->n_refs,
                                              memory_order_relaxed);
                do {
                        if (n_refs == 0)
                                return NULL;
                } while (!atomic_compare_exchange_weak_explicit(&ref->n_refs,
                                                &n_refs, n_refs + n,
                                                memory_order_relaxed,
                                                memory_order_relaxed));
        }

        return ref;
}

/**
 * c_ref_inc() - acquire reference
 * @ref:                reference counter to operate on, or NULL
 *
 * This acquires a single reference to @ref. See c_ref_add() for details. The
 * caller must guarantee that it already owns a reference to @ref.
 *
 * If @ref is NULL, this is a no-op.
 *
 * Return: @ref is returned.
 */
static inline CRef *c_ref_inc(CRef *ref) {
        return c_ref_add(ref, 1UL);
}

/**
 * c_ref_inc_unless_zero() - acquire reference if possible
 * @ref:                reference counter to operate on, or NULL
 *
 * Acquire a single reference to @ref, if the reference counter has not already
 * dropped to zero. See c_ref_add_unless_zero() for details.
 *
 * If @ref is NULL, this is a no-op.
 *
 * Return: @ref is returned on success, otherwise NULL is returned.
 */
static inline CRef *c_ref_inc_unless_zero(CRef *ref) {
        return c_ref_add_unless_zero(ref, 1UL);
}

/**
 * c_ref_unreachable() - convenience callback
 * @ref:                reference counter to release
 * @userdata:           userdata provided by caller
 *
 * This is a convenience callback to pass to c_ref_sub() and friends, if, and
 * only if, you are sure that your call will not cause the reference counter to
 * drop to 0. This release callback will abort the application if it is
 * actually called.
 */
static inline void c_ref_unreachable(CRef *ref, void *userdata) {
        assert(0);
}

/**
 * c_ref_sub() - release references
 * @ref:                reference counter to operate on, or NULL
 * @n:                  number of references to release
 * @func:               release function, or NULL
 * @userdata:           userdata to pass to release function
 *
 * Release @n references to the reference counter @ref. The caller must ensure
 * that it actually owns @n references when calling this. If this causes the
 * counter to drop to 0, then @func will be invoked (if non-NULL), with @ref
 * and @userdata passed to it. Otherwise, @func is not invoked and the call
 * simply returns to the caller after dropping @n references.
 *
 * This function provides sufficient read/write barriers regarding any
 * modifications of the object associated with @ref. That is, when an object is
 * actually released (i.e., @func is called), any prior writes of any thread
 * that were done while holding a reference, are guaranteed to be visible to
 * the release thread.
 *
 * If @ref is NULL, this is a no-op. @n must not be 0.
 *
 * Return: NULL is returned.
 */
static inline CRef *c_ref_sub(CRef *ref, unsigned long n, CRefFn func, void *userdata) {
        unsigned long n_refs;

        if (ref) {
                /*
                 * Make sure to order all our stores to the object before
                 * releasing the reference. We must guarantee that a racing
                 * unref operation will see our stores before they release the
                 * object. We could use memory_order_acq_rel, but we rather
                 * perform the acquire-barrier only in the release-path, since
                 * it is only needed there.
                 */
                n_refs = atomic_fetch_sub_explicit(&ref->n_refs, n,
                                                   memory_order_release);
                assert(n_refs >= n);
                if (n_refs == n) {
                        atomic_thread_fence(memory_order_acquire);
                        if (func)
                                func(ref, userdata);
                }
        }

        return NULL;
}

/**
 * c_ref_dec() - release a single reference
 * @ref:                reference counter to operate on, or NULL
 * @func:               release function, or NULL
 * @userdata:           userdata to pass to release function
 *
 * This releases a single reference to @ref. See c_ref_sub() for details.
 *
 * If @ref is NULL, this is a no-op.
 *
 * Return: NULL is returned.
 */
static inline CRef *c_ref_dec(CRef *ref, CRefFn func, void *userdata) {
        return c_ref_sub(ref, 1UL, func, userdata);
}

#ifdef __cplusplus
}
#endif
