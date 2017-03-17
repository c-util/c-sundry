#pragma once

/*
 * Selection of Convenience Macros
 *
 * This header contains macros useful across our codebase. This includes
 * pre-processor macros and a *very* limited set of inlined functions.
 *
 * As this header is included all over the place, make sure to only add stuff
 * that really belongs all-over-the-place.
 *
 * This header also provides a basic ISO-C11/POSIX environment to the callers.
 * This means, we include the very basic set of headers, to avoid copying them
 * to all common callers. This set is quite limited, though, and is considered
 * API of this header.
 *
 * Conventions:
 *  - Any macro written in UPPER-CASE letters might have side-effects and
 *    special behavior. See its comments for details. Usually, such macros
 *    cannot be implemented as normal C-functions, so they behave differently.
 *  - Macros that behave like C-functions (no multiple evaluation, type-safe,
 *    etc.) use lower-case names, like c_min(). If those functions can be
 *    evaluated at compile-time, they *must* support constant folding (i.e.,
 *    you can use them in constant expressions), and they also provide an
 *    equivalent call without any guards, which is written as upper-case name,
 *    like C_MIN(). Those calls do *not* provide any guards, but can rather be
 *    used in file-context, compared to function-context (file-contexts don't
 *    allow statement-expressions).
 *  - If macros support different numbers of arguments, we use the number as
 *    suffix, like C_CC_MACRO2() and C_CC_MACRO3(). Usually, their concept can
 *    be extended to infinity, but the C-preprocessor does not allow it. Hence,
 *    we hard-code the number of arguments.
 *  - Any internal function is prefixed with C_INTERNAL_*() or c_internal_*().
 *    Never call those directly.
 *  - Inline helpers solely designed for _c_cleanup_(foobarp) always carry the
 *    'p' suffix (e.g., c_freep(), c_closep(), ...).
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Basic ISO-C11/POSIX headers are considered API of this header. Don't remove
 * them as we rely on them in our sources.
 */
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/* must not depend on any other c-util header */

/*
 * We require:
 *   sizeof(void*) == sizeof(long)
 *   sizeof(long) == 4 || sizeof(long) == 8
 *   sizeof(int) == 4
 * The linux kernel requires the same from the toolchain, so this should work
 * just fine.
 */
#if __SIZEOF_POINTER__ != __SIZEOF_LONG__
#  error "sizeof(void*) != sizeof(long)"
#elif __SIZEOF_LONG__ != 4 && __SIZEOF_LONG__ != 8
#  error "sizeof(long) != 4 && sizeof(long) != 8"
#elif __SIZEOF_INT__ != 4
#  error "sizeof(int) != 4"
#endif

/*
 * Shortcuts for gcc attributes. See GCC manual for details. They're 1-to-1
 * mappings to the GCC equivalents. No additional magic here.
 */
#define _c_align_(_x) __attribute__((__aligned__(_x)))
#define _c_alignas_(_x) __attribute__((__aligned__(__alignof(_x))))
#define _c_alloc_(...) __attribute__((__alloc_size__(__VA_ARGS__)))
#define _c_cleanup_(_x) __attribute__((__cleanup__(_x)))
#define _c_const_ __attribute__((__const__))
#define _c_deprecated_ __attribute__((__deprecated__))
#define _c_hidden_ __attribute__((__visibility__("hidden")))
#define _c_likely_(_x) (__builtin_expect(!!(_x), 1))
#define _c_malloc_ __attribute__((__malloc__))
#define _c_packed_ __attribute__((__packed__))
#define _c_printf_(_a, _b) __attribute__((__format__(printf, _a, _b)))
#define _c_public_ __attribute__((__visibility__("default")))
#define _c_pure_ __attribute__((__pure__))
#define _c_sentinel_ __attribute__((__sentinel__))
#define _c_unlikely_(_x) (__builtin_expect(!!(_x), 0))
#define _c_unused_ __attribute__((__unused__))
#define _c_weak_ __attribute__((__weak__))
#define _c_weakref_(_x) __attribute__((__weakref__(#_x)))

/**
 * C_DECL() - create declaration-expression
 * @_expr:              expression to evaluate to
 * @_assertion:         arbitrary declaration
 *
 * This macro simply evaluates to @_expr. That is, it can be used in any
 * context that expects an expression like @_expr. Additionally, it takes a
 * declaration as @_decl and evaluates it.
 *
 * Most common usage is in combination with _Static_assert(), which happens
 * to be defined as a declaration by ISO C11.
 *
 * Return: Evaluates to @_expr.
 */
#define C_DECL(_expr, _assertion)                                       \
        /* line split to get better diagnostics on @_assertion usage */ \
        (__builtin_choose_expr(!!(1 + 0 * sizeof(struct {               \
        _assertion;                                                     \
        })), (_expr), ((void)0)))

/**
 * C_CC_IF() - conditional expression at compile time
 * @_cond:      condition
 * @_if:        if-clause
 * @_else:      else-clause
 *
 * This is a compile-time if-else-statement. Depending on whether the constant
 * expression @_cond is true or false, this evaluates to the passed clause. The
 * other clause is *not* evaluated, however, it may be checked for syntax
 * errors and *constant* expressions are evaluated.
 *
 * Return: Evaluates to either if-clause or else-clause, depending on whether
 *         the condition is true. The other clause is *not* evaluated.
 */
#define C_CC_IF(_cond, _if, _else) __builtin_choose_expr(!!(_cond), _if, _else)

/**
 * C_CC_IS_CONST() - check whether a value is known at compile time
 * @_expr:      expression
 *
 * This checks whether the value of @_expr is known at compile time. Note that
 * a negative result does not mean that it is *NOT* known. However, it means
 * that it cannot be guaranteed to be constant at compile time. Hence, false
 * negatives are possible.
 *
 * This macro *always* evaluates to a constant expression, regardless whether
 * the passed expression is constant.
 *
 * The passed in expression is *never* evaluated. Hence, it can safely be used
 * in combination with C_CC_IF() to avoid multiple evaluations of macro
 * parameters.
 *
 * Return: 1 if constant, 0 if not.
 */
#define C_CC_IS_CONST(_expr) __builtin_constant_p(_expr)

/**
 * C_VAR() - generate unique variable name
 * @_x:         name of variable
 * @_uniq:      unique prefix, usually provided by __COUNTER__, optional
 *
 * This macro shall be used to generate unique variable names, that will not be
 * shadowed by recursive macro invocations. It is effectively a
 * C_CONCATENATE of both arguments, but also provides a globally separated
 * prefix and makes the code better readable.
 *
 * The second argument is optional. If not given, __LINE__ is implied, and as
 * such the macro will generate the same identifier if used multiple times on
 * the same code-line (or within a macro). This should be used if recursive
 * calls into the macro are not expected.
 *
 * This helper may be used by macro implementations that might reasonable well
 * be called in a stacked fasion, like:
 *     c_max(foo, c_max(bar, baz))
 * Such a stacked call of c_max() might cause compiler warnings of shadowed
 * variables in the definition of c_max(). By using C_VAR(), such warnings
 * can be silenced as each evaluation of c_max() uses unique variable names.
 *
 * Return: This evaluates to a constant identifier
 */
#define C_VAR(...) C_INTERNAL_VAR(__VA_ARGS__, 2, 1)
#define C_INTERNAL_VAR(_x, _uniq, _num, ...) C_VAR ## _num (_x, _uniq)
#define C_VAR1(_x, _unused) C_VAR2(_x, C_CONCATENATE(line, __LINE__))
#define C_VAR2(_x, _uniq) C_CONCATENATE(c_internal_var_unique_, C_CONCATENATE(_uniq, _x))

/**
 * C_CC_MACRO1() - provide save environment to a macro
 * @_call:      macro to call
 * @_x:         first argument
 *
 * This is the 1-argument equivalent of C_CC_MACRO2().
 *
 * Return: Result of @_call is returned.
 */
#define C_CC_MACRO1(_call, _x, ...) C_INTERNAL_CC_MACRO1(_call, __COUNTER__, (_x), ## __VA_ARGS__)
#define C_INTERNAL_CC_MACRO1(_call, _xq, _x, ...)                       \
        C_CC_IF(                                                        \
                C_CC_IS_CONST(_x),                                      \
                _call(_x, ## __VA_ARGS__),                              \
                __extension__ ({                                        \
                        const __auto_type C_VAR(X, _xq) = (_x);         \
                        _call(C_VAR(X, _xq), ## __VA_ARGS__);           \
                }))

/**
 * C_CC_MACRO2() - provide save environment to a macro
 * @_call:      macro to call
 * @_x:         first argument
 * @_y:         second argument
 *
 * This function simplifies the implementation of macros. Whenever you
 * implement a macro, provide the internal macro name as @_call and its
 * arguments as @_x and @_y. Inside of your internal macro, you:
 *  - are safe against multiple evaluation errors
 *  - support constant folding
 *  - have unique variable names for recursive callers
 *  - have properly typed arguments
 *
 * Return: Result of @_call is returned.
 */
#define C_CC_MACRO2(_call, _x, _y, ...) C_INTERNAL_CC_MACRO2(_call, __COUNTER__, (_x), __COUNTER__, (_y), ## __VA_ARGS__)
#define C_INTERNAL_CC_MACRO2(_call, _xq, _x, _yq, _y, ...)                      \
        C_CC_IF(                                                                \
                (C_CC_IS_CONST(_x) && C_CC_IS_CONST(_y)),                       \
                _call((_x), (_y), ## __VA_ARGS__),                              \
                __extension__ ({                                                \
                        const __auto_type C_VAR(X, _xq) = (_x);                 \
                        const __auto_type C_VAR(Y, _yq) = (_y);                 \
                        _call(C_VAR(X, _xq), C_VAR(Y, _yq), ## __VA_ARGS__);    \
                }))

/**
 * C_CC_MACRO3() - provide save environment to a macro
 * @_call:      macro to call
 * @_x:         first argument
 * @_y:         second argument
 * @_z:         third argument
 *
 * This is the 3-argument equivalent of C_CC_MACRO2().
 *
 * Return: Result of @_call is returned.
 */
#define C_CC_MACRO3(_call, _x, _y, _z, ...) C_INTERNAL_CC_MACRO3(_call, __COUNTER__, (_x), __COUNTER__, (_y), __COUNTER__, (_z), ## __VA_ARGS__)
#define C_INTERNAL_CC_MACRO3(_call, _xq, _x, _yq, _y, _zq, _z, ...)                             \
        C_CC_IF(                                                                                \
                (C_CC_IS_CONST(_x) && C_CC_IS_CONST(_y) && C_CC_IS_CONST(_z)),                  \
                _call((_x), (_y), (_z), ## __VA_ARGS__),                                        \
                __extension__ ({                                                                \
                        const __auto_type C_VAR(X, _xq) = (_x);                                 \
                        const __auto_type C_VAR(Y, _yq) = (_y);                                 \
                        const __auto_type C_VAR(Z, _zq) = (_z);                                 \
                        _call(C_VAR(X, _xq), C_VAR(Y, _yq), C_VAR(Z, _zq), ## __VA_ARGS__);     \
                }))

/**
 * C_STRINGIFY() - stringify a token, but evaluate it first
 * @_x:         token to evaluate and stringify
 *
 * Return: Evaluates to a constant string literal
 */
#define C_STRINGIFY(_x) C_INTERNAL_STRINGIFY(_x)
#define C_INTERNAL_STRINGIFY(_x) #_x

/**
 * C_CONCATENATE() - concatenate two tokens, but evaluate them first
 * @_x:         first token
 * @_y:         second token
 *
 * Return: Evaluates to a constant identifier
 */
#define C_CONCATENATE(_x, _y) C_INTERNAL_CONCATENATE(_x, _y)
#define C_INTERNAL_CONCATENATE(_x, _y) _x ## _y

/**
 * C_ARRAY_SIZE() - calculate number of array elements at compile time
 * @_x:         array to calculate size of
 *
 * Return: Evaluates to a constant integer expression
 */
#define C_ARRAY_SIZE(_x)                                                \
        C_DECL(sizeof(_x) / sizeof((_x)[0]),                            \
               /*                                                       \
                * Verify that `_x' is an array, not a pointer. Rely on  \
                * `&_x[0]' degrading arrays to pointers.                \
                */                                                      \
               _Static_assert(                                          \
                        !__builtin_types_compatible_p(                  \
                                __typeof__(_x),                         \
                                __typeof__(&(*(__typeof__(_x)*)0)[0])   \
                        ),                                              \
                        "C_ARRAY_SIZE() called with non-array argument" \
               ))

/**
 * C_DECIMAL_MAX() - calculate maximum length of the decimal
 *                   representation of an integer
 * @_type: integer variable/type
 *
 * This calculates the bytes required for the decimal representation of an
 * integer of the given type. It accounts for a possible +/- prefix, but it
 * does *NOT* include the trailing terminating zero byte.
 *
 * Return: Evaluates to a constant integer expression
 */
#define C_DECIMAL_MAX(_arg)                                                             \
        (_Generic((__typeof__(_arg)){ 0 },                                              \
                        char: C_INTERNAL_DECIMAL_MAX(sizeof(char)),                     \
                 signed char: C_INTERNAL_DECIMAL_MAX(sizeof(signed char)),              \
               unsigned char: C_INTERNAL_DECIMAL_MAX(sizeof(unsigned char)),            \
                signed short: C_INTERNAL_DECIMAL_MAX(sizeof(signed short)),             \
              unsigned short: C_INTERNAL_DECIMAL_MAX(sizeof(unsigned short)),           \
                  signed int: C_INTERNAL_DECIMAL_MAX(sizeof(signed int)),               \
                unsigned int: C_INTERNAL_DECIMAL_MAX(sizeof(unsigned int)),             \
                 signed long: C_INTERNAL_DECIMAL_MAX(sizeof(signed long)),              \
               unsigned long: C_INTERNAL_DECIMAL_MAX(sizeof(unsigned long)),            \
            signed long long: C_INTERNAL_DECIMAL_MAX(sizeof(signed long long)),         \
          unsigned long long: C_INTERNAL_DECIMAL_MAX(sizeof(unsigned long long))))
#define C_INTERNAL_DECIMAL_MAX(_bytes)          \
        C_DECL(1 + ((_bytes) <= 1 ?  3 :        \
                    (_bytes) <= 2 ?  5 :        \
                    (_bytes) <= 4 ? 10 :        \
                                    20),        \
               _Static_assert((_bytes) <= 8,    \
                              "Invalid use of C_INTERNAL_DECIMAL_MAX()"))

/**
 * c_container_of() - cast a member of a structure out to the containing structure
 * @_ptr:       pointer to the member or NULL
 * @_type:      type of the container struct this is embedded in
 * @_member:    name of the member within the struct
 */
#define c_container_of(_ptr, _type, _member) c_internal_container_of(__COUNTER__, (_ptr), _type, _member)
#define c_internal_container_of(_uniq, _ptr, _type, _member)                                    \
        __extension__ ({                                                                        \
                /* avoid `__typeof__' since it strips qualifiers */                             \
                __auto_type C_VAR(A, _uniq) = 0 ? &((const _type *)0)->_member : (_ptr);        \
                (_ptr) ? (_type*)( (char*)C_VAR(A, _uniq) - offsetof(_type, _member) ) : NULL;  \
        })

/**
 * c_max() - compute maximum of two values
 * @_a:         value A
 * @_b:         value B
 *
 * Calculate the maximum of both passed values. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: Maximum of both values is returned.
 */
#define c_max(_a, _b) C_CC_MACRO2(C_MAX, (_a), (_b))
#define C_MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))

/**
 * c_min() - compute minimum of two values
 * @_a:         value A
 * @_b:         value B
 *
 * Calculate the minimum of both passed values. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: Minimum of both values is returned.
 */
#define c_min(_a, _b) C_CC_MACRO2(C_MIN, (_a), (_b))
#define C_MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))

/**
 * c_less_by() - calculate clamped difference of two values
 * @_a:         minuend
 * @_b:         subtrahend
 *
 * Calculate [_a - _b], but clamp the result to 0. Both arguments are evaluated
 * exactly once, under all circumstances. Furthermore, if both values are
 * constant expressions, the result will be constant as well.
 *
 * Return: This computes [_a - _b], if [_a > _b]. Otherwise, 0 is returned.
 */
#define c_less_by(_a, _b) C_CC_MACRO2(C_LESS_BY, (_a), (_b))
#define C_LESS_BY(_a, _b) ((_a) > (_b) ? (_a) - (_b) : 0)

/**
 * c_clamp() - clamp value to lower and upper boundary
 * @_x:         value to clamp
 * @_low:       lower boundary
 * @_high:      higher boundary
 *
 * This clamps @_x to the lower and higher bounds given as @_low and @_high.
 * All arguments are evaluated exactly once, and yield a constant expression if
 * all arguments are constant as well.
 *
 * Return: Clamped integer value.
 */
#define c_clamp(_x, _low, _high) C_CC_MACRO3(C_CLAMP, (_x), (_low), (_high))
#define C_CLAMP(_x, _low, _high) ((_x) > (_high) ? (_high) : (_x) < (_low) ? (_low) : (_x))

/**
 * c_clz() - count leading zeroes
 * @_val:       value to count leading zeroes of
 *
 * This counts the leading zeroes of the binary representation of @_val. Note
 * that @_val must be of an integer type greater than, or equal to, 'unsigned
 * int'.
 *
 * This macro evaluates the argument exactly once, and if the input is
 * constant, it also evaluates to a constant expression.
 *
 * Note that this macro calculates the number of leading zeroes within the
 * scope of the integer type of @_val. That is, if the input is a 32bit type
 * with value 1, it yields 31. But if it is a 64bit type with the same value 1,
 * it yields 63.
 *
 * Return: Evaluates to an 'int', the number of leading zeroes.
 */
#define c_clz(_val) C_CC_MACRO1(C_CLZ, (_val))
#define C_CLZ(_val)                                                     \
        (_Generic((_val),                                               \
                unsigned int: __builtin_clz,                            \
               unsigned long: __builtin_clzl,                           \
          unsigned long long: __builtin_clzll)                          \
                /* clz(0) is undefined behavior, so work around it */   \
                ((_val) | !(_val)) + !_c_likely_(_val))

/**
 * c_log2() - binary logarithm
 * @_val:       input value
 *
 * This calculates the integer binary logarithm of @_val. If the logarithm
 * cannot be represented as an integer, the result is rounded down.
 *
 * Special case: log2(0) is arbitrarily defined as 0.
 *
 * Return: Evaluates to an 'int', the binary logarithm of the input.
 */
#define c_log2(_val) C_CC_MACRO1(C_LOG2, (_val))
#define C_LOG2(_val) ((_val) ? (sizeof(_val) * 8 - C_CLZ(_val) - 1) : 0)

/**
 * c_align_to() - align value to
 * @_val:       value to align
 * @_to:        align to multiple of this
 *
 * This aligns @_val to a multiple of @_to. If @_val is already a multiple of
 * @_to, @_val is returned unchanged. This function operates within the
 * boundaries of the type of @_val and @_to. Make sure to cast them if needed.
 *
 * The arguments of this macro are evaluated exactly once. If both arguments
 * are a constant expression, this also yields a constant return value.
 *
 * Note that @_to must be a power of 2. In case @_to is a constant expression,
 * this macro places a compile-time assertion on the popcount of @_to, to
 * verify it is a power of 2.
 *
 * Return: @_val aligned to a multiple of @_to
 */
#define c_align_to(_val, _to) C_CC_MACRO2(C_ALIGN_TO, (_val), (_to))
#define C_ALIGN_TO(_val, _to) (((_val) + (_to) - 1) & ~((_to) - 1))

/**
 * c_align8() - align value to multiple of 8
 * @_val:       value to align
 *
 * This is the same as c_align_to((_val), 8).
 *
 * Return: @_val aligned to a multiple of 8.
 */
#define c_align8(_val) c_align_to((_val), 8)

/**
 * c_align_power2() - align value to next power of 2
 * @_val:       value to align
 *
 * This aligns @_val to the next higher power of 2. If it already is a power of
 * 2, the value is returned unchanged. 0 is treated as power of 2 (so 0 yields
 * 0). Furthermore, on overflow, this yields 0 as well.
 *
 * Note that this always operates within the bounds of the type of @_val.
 *
 * Return: @_val aligned to the next higher power of 2
 */
#define c_align_power2(_val) C_CC_MACRO1(C_ALIGN_POWER2, (_val))
#define C_ALIGN_POWER2(_val)                                                            \
        (((_val) == 1) ? 1 : /* clz(0) is undefined */                                  \
                (C_CLZ((_val) - 1) < 1) ? 0 : /* shift-overflow is undefined */         \
                        (((__typeof__(_val))1) << (C_LOG2((_val) - 1) + 1)))

/**
 * c_div_round_up() - calculate integer quotient but round up
 * @_x:         dividend
 * @_y:         divisor
 *
 * Calculates [x / y] but rounds up the result to the next integer. All
 * arguments are evaluated exactly once, and yield a constant expression if all
 * arguments are constant.
 *
 * Note:
 * [(x + y - 1) / y] suffers from an integer overflow, even though the
 * computation should be possible in the given type. Therefore, we use
 * [x / y + !!(x % y)]. Note that on most CPUs a division returns both the
 * quotient and the remainder, so both should be equally fast. Furthermore, if
 * the divisor is a power of two, the compiler will optimize it, anyway.
 *
 * Return: The quotient is returned.
 */
#define c_div_round_up(_x, _y) C_CC_MACRO2(C_DIV_ROUND_UP, (_x), (_y))
#define C_DIV_ROUND_UP(_x, _y) ((_x) / (_y) + !!((_x) % (_y)))

/**
 * c_alloca8() - aligned alloca()
 * @_size:       hunk size to allocate
 *
 * This is almost the same as alloca(), but makes sure the result is 8-byte
 * aligned. This function never fails.
 *
 * Note: glibc guarantees that alloca() returns 16-byte aligned memory. This
 *       macro is just for documentational purposes as it is very hard to find
 *       any documentation on this. Anyway, it is ABI and we can safely rely on
 *       it.
 *
 * Return: Pointer to allocated stack memory.
 */
#define c_alloca8(_size) alloca(_size)

/**
 * c_errno() - return valid errno
 *
 * This helper should be used to shut up gcc if you know 'errno' is valid (ie.,
 * errno is > 0). Instead of "return -errno;", use
 * "return -c_errno();" It will suppress bogus gcc warnings in case it assumes
 * 'errno' might be 0 (or <0) and thus the caller's error-handling might not be
 * triggered.
 *
 * This helper should be avoided whenever possible. However, occasionally we
 * really want to shut up gcc (especially with static/inline functions). In
 * those cases, gcc usually cannot deduce that some error paths are guaranteed
 * to be taken. Hence, making the return value explicit allows gcc to better
 * optimize the code.
 *
 * Note that you really should never use this helper to work around broken libc
 * calls or syscalls, not setting 'errno' correctly.
 *
 * Return: Positive error code is returned.
 */
static inline int c_errno(void) {
        return _c_likely_(errno > 0) ? errno : EINVAL;
}

/*
 * Common Destructors
 * Followingly, there're a bunch of common 'static inline' constructors, which
 * simply call the function that they're named after, but return "INVALID"
 * instead of "void". This allows direct assignment to any member-field and/or
 * variable they're defined in, like:
 *
 *   foo = c_free(foo);
 * or
 *   foo->bar = c_close(foo->bar);
 *
 * Furthermore, all those destructors can be safely called with the "INVALID"
 * value as argument, and they will be a no-op.
 */

static inline void *c_free(void *p) {
        free(p);
        return NULL;
}

static inline int c_close(int fd) {
        if (fd >= 0)
                close(fd);
        return -1;
}

static inline FILE *c_fclose(FILE *f) {
        if (f)
                fclose(f);
        return NULL;
}

static inline DIR *c_closedir(DIR *d) {
        if (d)
                closedir(d);
        return NULL;
}

/*
 * Common Cleanup Helpers
 * A bunch of _c_cleanup_(foobarp) helpers that are used all over the place.
 * Note that all of those have the "if (IS_INVALID(foobar))" check inline, so
 * compilers can optimize most of the cleanup-paths in a function. However, if
 * the function they call already does this _inline_, then it might be skipped.
 */

#define C_DEFINE_CLEANUP(_type, _func)                  \
        static inline void _func ## p(_type *p) {       \
                if (*p)                                 \
                        _func(*p);                      \
        } struct c_internal_trailing_semicolon

#define C_DEFINE_DIRECT_CLEANUP(_type, _func)           \
        static inline void _func ## p(_type *p) {       \
                _func(*p);                              \
        } struct c_internal_trailing_semicolon

static inline void c_freep(void *p) {
        c_free(*(void **)p); /* free() is not type-safe, must hard-code it */
}

C_DEFINE_DIRECT_CLEANUP(int, c_close);
C_DEFINE_CLEANUP(FILE *, c_fclose);
C_DEFINE_CLEANUP(DIR *, c_closedir);

#ifdef __cplusplus
}
#endif
