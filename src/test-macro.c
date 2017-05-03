/*
 * Tests for utility macros.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include "c-macro.h"
#include "c-syscall.h"

/*
 * Trivial Malloc Counters
 *
 * This test contains a hook for malloc() and free(). They use fallbacks to
 * their libc equivalents, but update the atomic counter @test_malloc_n. This
 * counter can be used to measure the number of calls to malloc() and free() in
 * protected environments. It is not meant to be authoritative, but safe in
 * trivial tests like this one.
 */
static _Atomic size_t test_malloc_n;

void *malloc(size_t size) {
        static _Atomic(void *) next;
        void *p;

        if (!next) {
                p = dlsym(RTLD_NEXT, "malloc");
                assert(p);
                next = p;
        }

        p = ((void *(*)(size_t))next)(size);
        if (p)
                ++test_malloc_n;

        return p;
}

void free(void *ptr) {
        static _Atomic(void *) next;
        void *p;

        if (!next) {
                p = dlsym(RTLD_NEXT, "free");
                assert(p);
                next = p;
        }

        if (ptr)
                --test_malloc_n;

        ((void (*)(void *))next)(ptr);
}

/*
 * Tests for:
 *  - c_free*()
 *  - c_close*()
 *  - c_fclose*()
 *  - c_closedir*()
 */
static void test_destructors(void) {
        int i;

        /*
         * Verify that c_free*() works as expected. We have hooks into malloc()
         * and free(), so simply verify the counter after a bunch of c_free*()
         * calls.
         */
        {
                size_t n_alloc;

                n_alloc = test_malloc_n;
                for (i = 0; i < 16; ++i) {
                        _c_cleanup_(c_freep) void *foo;
                        _c_cleanup_(c_freep) int **bar; /* supports any type */
                        size_t sz = 128 * 1024;

                        foo = malloc(sz);
                        assert(foo);

                        assert(n_alloc < test_malloc_n);

                        bar = malloc(sz);
                        assert(bar);
                        bar = c_free(bar);
                        assert(!bar);

                        assert(n_alloc < test_malloc_n);
                }
                assert(n_alloc == test_malloc_n);

                assert(c_free(NULL) == NULL);
        }

        /*
         * Test c_close*(), rely on sparse FD allocation. Make sure all the
         * helpers actually close the fd, and cope fine with negative numbers.
         */
        {
                int fd;

                fd = c_syscall_memfd_create("foobar", 0);
                assert(fd >= 0);

                /* verify c_close() returns -1 */
                assert(c_close(fd) == -1);

                /* verify c_close() deals fine with negative fds */
                assert(c_close(-1) == -1);
                assert(c_close(-16) == -1);

                /* make sure c_closep() deals fine with negative FDs */
                {
                        _c_cleanup_(c_closep) int t = 0;
                        t = -1;
                }

                /*
                 * Make sure the c_close() earlier worked, by allocating the
                 * FD again and relying on the same FD number to be reused. Do
                 * this twice, to verify that the c_closep() in the cleanup
                 * path works as well.
                 */
                for (i = 0; i < 2; ++i) {
                        _c_cleanup_(c_closep) int t = -1;

                        t = c_syscall_memfd_create("foobar", 0);
                        assert(t >= 0);
                        assert(t == fd);
                }
        }

        /*
         * Test c_fclose() and c_fclosep(). This uses the same logic as the
         * tests for c_close() (i.e., sparse FD allocation).
         */
        {
                FILE *f;
                int fd;

                fd = c_syscall_memfd_create("foobar", 0);
                assert(fd >= 0);

                f = fdopen(fd, "r");
                assert(f);

                /* verify c_fclose() returns NULL */
                f = c_fclose(f);
                assert(!f);

                /* verify c_fclose() deals fine with NULL */
                assert(!c_fclose(NULL));

                /* make sure c_flosep() deals fine with NULL */
                {
                        _c_cleanup_(c_fclosep) FILE *t = (void *)0xdeadbeef;
                        t = NULL;
                }

                /*
                 * Make sure the c_fclose() earlier worked, by allocating the
                 * FD again and relying on the same FD number to be reused. Do
                 * this twice, to verify that the c_fclosep() in the cleanup
                 * path works as well.
                 */
                for (i = 0; i < 2; ++i) {
                        _c_cleanup_(c_fclosep) FILE *t = NULL;
                        int tfd;

                        tfd = c_syscall_memfd_create("foobar", 0);
                        assert(tfd >= 0);
                        assert(tfd == fd); /* the same as before */

                        t = fdopen(tfd, "r");
                        assert(t);
                }
        }
}

/*
 * Tests for all remaining macros:
 *  - C_CC_IS_CONST()
 *  - C_VAR()
 *  - C_STRINGIFY()
 *  - C_CONCATENATE()
 *  - C_EXPAND()
 *  - C_ARRAY_SIZE()
 *  - C_DECIMAL_MAX()
 *  - c_container_of()
 *  - c_max()
 *  - c_min()
 *  - c_less_by()
 *  - c_clamp()
 *  - c_clz()
 *  - c_log2()
 *  - c_align_to()
 *  - c_align8()
 *  - c_align_power2()
 *  - c_div_round_up()
 *  - c_alloca8()
 *  - c_negative_errno()
 */
static void test_misc(int non_constant_expr) {
        int foo;

        /*
         * Test constant-expr checks.
         * The C_CC_IS_CONST() macro allows verifying whether an expression is
         * constant. The return value of the macro itself is constant, and as
         * such can be used for constant expressions itself.
         */
        {
                foo = 11;
                static_assert(C_CC_IS_CONST(5), "");
                static_assert(!C_CC_IS_CONST(non_constant_expr), "");
                static_assert(C_CC_IS_CONST(C_CC_IS_CONST(non_constant_expr)), "");
                static_assert(!C_CC_IS_CONST(foo++), ""); /* *NOT* evaluated */
                assert(foo == 11);
        }

        /*
         * Test C_VAR() macro. It's sole purpose is to create a valid C
         * identifier given a single argument (which itself must be a valid
         * identifier).
         * Just test that we can declare variables with it and use it in
         * expressions.
         */
        {
                {
                        int C_VAR(sub, UNIQUE) = 5;
                        /* make sure the variable name does not clash */
                        int sub = 12, subUNIQUE = 12, UNIQUEsub = 12;

                        assert(7 + C_VAR(sub, UNIQUE) == sub);
                        assert(sub == subUNIQUE);
                        assert(sub == UNIQUEsub);
                }
                {
                        /*
                         * Make sure both produce different names, even though they're
                         * exactly the same expression.
                         */
                        _c_unused_ int C_VAR(sub, __COUNTER__), C_VAR(sub, __COUNTER__);
                }
                {
                        /* verify C_VAR() with single argument works line-based */
                        int C_VAR(sub); C_VAR(sub) = 5; assert(C_VAR(sub) == 5);
                }
                {
                        /* verify C_VAR() with no argument works line-based */
                        int C_VAR(); C_VAR() = 5; assert(C_VAR() == 5);
                }
        }

        /*
         * Test stringify/concatenation helpers. Also make sure to test that
         * the passed arguments are evaluated first, before they're stringified
         * and/or concatenated.
         */
        {
#define TEST_TOKEN foobar
                assert(!strcmp("foobar", C_STRINGIFY(foobar)));
                assert(!strcmp("foobar", C_STRINGIFY(TEST_TOKEN)));
                assert(!strcmp("foobar", C_STRINGIFY(C_CONCATENATE(foo, bar))));
                assert(!strcmp("foobarfoobar", C_STRINGIFY(C_CONCATENATE(TEST_TOKEN, foobar))));
                assert(!strcmp("foobarfoobar", C_STRINGIFY(C_CONCATENATE(foobar, TEST_TOKEN))));
#undef TEST_TOKEN
        }

        /*
         * Test tuple expansion. This is used to strip tuple-wrappers in the
         * pre-processor.
         * We make sure that it works with {0,1,2}-tuples, as well as only
         * strips a single layer.
         */
        {
                /*
                 * strcmp() might be a macro, so make sure we get a proper C
                 * expression below. Otherwise, C_EXPAND() cannot be used that
                 * way (since it would evaluate to a single macro argument).
                 */
                int (*f) (const char *, const char *) = strcmp;

                assert(!f(C_EXPAND(()) "foobar", "foo" "bar"));
                assert(!f(C_EXPAND(("foobar")), "foo" "bar"));
                assert(!f(C_EXPAND(("foobar", "foo" "bar"))));
                assert(!f C_EXPAND((("foobar", "foo" "bar"))));
        }

        /*
         * Test array-size helper. This simply computes the number of elements
         * of an array, instead of the binary size.
         */
        {
                int bar[8];

                static_assert(C_ARRAY_SIZE(bar) == 8, "");
                static_assert(C_CC_IS_CONST(C_ARRAY_SIZE(bar)), "");
        }

        /*
         * Test decimal-representation calculator. Make sure it is
         * type-independent and just uses the size of the type to calculate how
         * many bytes are needed to print that integer in decimal form. Also
         * verify that it is a constant expression.
         */
        {
                static_assert(C_DECIMAL_MAX(char) == 4, "");
                static_assert(C_DECIMAL_MAX(signed char) == 4, "");
                static_assert(C_DECIMAL_MAX(unsigned char) == 4, "");
                static_assert(C_DECIMAL_MAX(unsigned long) == 21, "");
                static_assert(C_DECIMAL_MAX(unsigned long long) == 21, "");
                static_assert(C_DECIMAL_MAX(int32_t) == 11, "");
                static_assert(C_DECIMAL_MAX(uint32_t) == 11, "");
                static_assert(C_DECIMAL_MAX(uint64_t) == 21, "");
        }

        /*
         * Test c_container_of(). We cannot test for type-safety, nor for
         * other invalid uses, as they'd require negative compile-testing.
         * However, we can test that the macro yields the correct values under
         * normal use.
         */
        {
                struct foobar {
                        int a;
                        char b;
                } sub = {};

                assert(&sub == c_container_of(&sub.a, struct foobar, a));
                assert(&sub == c_container_of(&sub.b, struct foobar, b));
                assert(&sub == c_container_of((const char *)&sub.b, struct foobar, b));
        }

        /*
         * Test min/max macros. Especially check that macro arguments are never
         * evaluated multiple times, and if both arguments are constant, the
         * return value is constant as well.
         */
        {
                foo = 0;
                assert(c_max(1, 5) == 5);
                assert(c_max(-1, 5) == 5);
                assert(c_max(-1, -5) == -1);
                assert(c_max(foo++, -1) == 0);
                assert(foo == 1);
                assert(c_max(foo++, foo++) > 0);
                assert(foo == 3);

                static_assert(C_CC_IS_CONST(c_max(1, 5)), "");
                static_assert(!C_CC_IS_CONST(c_max(1, non_constant_expr)), "");

                foo = 0;
                assert(c_min(1, 5) == 1);
                assert(c_min(-1, 5) == -1);
                assert(c_min(-1, -5) == -5);
                assert(c_min(foo++, 1) == 0);
                assert(foo == 1);
                assert(c_min(foo++, foo++) > 0);
                assert(foo == 3);

                static_assert(C_CC_IS_CONST(c_min(1, 5)), "");
                static_assert(!C_CC_IS_CONST(c_min(1, non_constant_expr)), "");
        }

        /*
         * Test c_less_by(), c_clamp(). Make sure they
         * evaluate arguments exactly once, and yield a constant expression,
         * if all arguments are constant.
         */
        {
                foo = 8;
                assert(c_less_by(1, 5) == 0);
                assert(c_less_by(5, 1) == 4);
                assert(c_less_by(foo++, 1) == 7);
                assert(foo == 9);
                assert(c_less_by(foo++, foo++) >= 0);
                assert(foo == 11);

                static_assert(C_CC_IS_CONST(c_less_by(1, 5)), "");
                static_assert(!C_CC_IS_CONST(c_less_by(1, non_constant_expr)), "");

                foo = 8;
                assert(c_clamp(foo, 1, 5) == 5);
                assert(c_clamp(foo, 9, 20) == 9);
                assert(c_clamp(foo++, 1, 5) == 5);
                assert(foo == 9);
                assert(c_clamp(foo++, foo++, foo++) >= 0);
                assert(foo == 12);

                static_assert(C_CC_IS_CONST(c_clamp(0, 1, 5)), "");
                static_assert(!C_CC_IS_CONST(c_clamp(1, 0, non_constant_expr)), "");
        }

        /*
         * Count Leading Zeroes: The c_clz() macro is a type-generic
         * variant of clz(). It counts leading zeroes of an integer. The result
         * highly depends on the integer-width of the input. Make sure it
         * selects the correct implementation.
         * Also note: clz(0) is undefined!
         */
        {
                assert(c_clz(UINT32_C(0)) == 32);
                assert(c_clz(UINT32_C(1)) == 31);
                assert(c_clz(UINT64_C(1)) == 63);
                assert(c_clz(UINT64_C(-1)) == 0);
                assert(c_clz(UINT64_C(0x100000000)) == 31);

                assert(c_clz(UINT32_C(-1)) == 0);
                assert(c_clz(UINT32_C(-1) + 2) == 31);

                assert(c_clz((uint64_t)UINT32_C(-1)) == 32);
                assert(c_clz((uint64_t)UINT32_C(-1) + 2) == 31);

                static_assert(!C_CC_IS_CONST(c_clz((unsigned int)non_constant_expr)), "");
        }

        /*
         * Binary logarithm: Normal binary logarithm tests, including special
         * cases log2(0) (which is defined in the implementation as 0) and
         * overflow-protection tests.
         */
        {
                assert(c_log2(UINT32_C(0)) == 0);
                assert(c_log2(UINT32_C(1)) == 0);
                assert(c_log2(UINT32_C(2)) == 1);
                assert(c_log2(UINT32_C(3)) == 1);
                assert(c_log2(UINT32_C(4)) == 2);
                assert(c_log2(UINT32_C(5)) == 2);
                assert(c_log2(UINT32_C(6)) == 2);
                assert(c_log2(UINT32_C(7)) == 2);
                assert(c_log2(UINT32_C(8)) == 3);
                assert(c_log2(UINT32_C(9)) == 3);

                assert(c_log2(UINT64_C(0)) == 0);
                assert(c_log2(UINT64_C(1)) == 0);
                assert(c_log2(UINT64_C(2)) == 1);
                assert(c_log2(UINT64_C(3)) == 1);
                assert(c_log2(UINT64_C(4)) == 2);
                assert(c_log2(UINT64_C(5)) == 2);
                assert(c_log2(UINT64_C(6)) == 2);
                assert(c_log2(UINT64_C(7)) == 2);
                assert(c_log2(UINT64_C(8)) == 3);
                assert(c_log2(UINT64_C(9)) == 3);

                assert(c_log2(UINT32_C(0xffffffff)) == 31);
                assert(c_log2(UINT64_C(0xffffffff)) == 31);
                assert(c_log2(UINT64_C(0x100000000)) == 32);
                assert(c_log2(UINT64_C(0x8000000000000000)) == 63);
                assert(c_log2(UINT64_C(0xffffffffffffffff)) == 63);

                static_assert(!C_CC_IS_CONST(c_log2((unsigned int)non_constant_expr)), "");
        }

        /*
         * Align to multiple of: Test the alignment macro. Check that it does
         * not suffer from incorrect integer overflows, neither should it
         * exceed the boundaries of the input type.
         */
        {
                int i;

                assert(c_align_to(UINT32_C(0), 1) == 0);
                assert(c_align_to(UINT32_C(0), 2) == 0);
                assert(c_align_to(UINT32_C(0), 4) == 0);
                assert(c_align_to(UINT32_C(0), 8) == 0);
                assert(c_align_to(UINT32_C(1), 8) == 8);

                assert(c_align_to(UINT32_C(0xffffffff), 8) == 0);
                assert(c_align_to(UINT32_C(0xfffffff1), 8) == 0xfffffff8);
                assert(c_align_to(UINT32_C(0xfffffff1), 8) == 0xfffffff8);

                static_assert(C_CC_IS_CONST(c_align_to(16, 8)), "");
                static_assert(!C_CC_IS_CONST(c_align_to(non_constant_expr, 8)), "");
                static_assert(!C_CC_IS_CONST(c_align_to(16, non_constant_expr)), "");
                static_assert(!C_CC_IS_CONST(c_align_to(16, non_constant_expr ? 8 : 16)), "");
                static_assert(C_CC_IS_CONST(c_align_to(16, 7 + 1)), "");
                assert(c_align_to(15, non_constant_expr ? 8 : 16) == 16);

                for (i = 0; i < 0xffff; ++i)
                        assert(c_align8(i) == c_align_to(i, 8));
        }

        /*
         * Align Power2: The c_align_power2() macro aligns passed values to the
         * next power of 2. Special cases: 0->0, overflow->0
         * Also make sure it never performs an up-cast on overflow.
         */
        {
                assert(c_align_power2(UINT32_C(2)) == 2);
                assert(c_align_power2(UINT32_C(0)) == 0);
                assert(c_align_power2(UINT32_C(0x80000001)) == 0);
                assert(c_align_power2(UINT64_C(0)) == 0);
                assert(c_align_power2(UINT64_C(0x8000000000000001)) == 0);
                assert(c_align_power2(UINT64_C(0x8000000000000001)) == 0);

                assert(c_align_power2((uint64_t)UINT32_C(0)) == 0);
                assert(c_align_power2((uint64_t)UINT32_C(0x80000001)) == UINT64_C(0x100000000));
                assert(c_align_power2((uint64_t)UINT32_C(0x80000001)) == UINT64_C(0x100000000));

                assert(c_align_power2(UINT32_C(1)) == 1);
                assert(c_align_power2(UINT32_C(2)) == 2);
                assert(c_align_power2(UINT32_C(3)) == 4);
                assert(c_align_power2(UINT32_C(4)) == 4);
                assert(c_align_power2(UINT32_C(5)) == 8);
                assert(c_align_power2(UINT32_C(0x80000000)) == UINT32_C(0x80000000));

                static_assert(!C_CC_IS_CONST(c_align_power2((unsigned int)non_constant_expr)), "");
        }

        /*
         * Div Round Up: Normal division, but round up to next integer, instead
         * of clipping. Also verify that it does not suffer from the integer
         * overflow in the prevalant, alternative implementation:
         *      [(x + y - 1) / y].
         */
        {
                int i, j;

#define TEST_ALT_DIV(_x, _y) (((_x) + (_y) - 1) / (_y))
                foo = 8;
                assert(c_div_round_up(0, 5) == 0);
                assert(c_div_round_up(1, 5) == 1);
                assert(c_div_round_up(5, 5) == 1);
                assert(c_div_round_up(6, 5) == 2);
                assert(c_div_round_up(foo++, 1) == 8);
                assert(foo == 9);
                assert(c_div_round_up(foo++, foo++) >= 0);
                assert(foo == 11);

                static_assert(C_CC_IS_CONST(c_div_round_up(1, 5)), "");
                static_assert(!C_CC_IS_CONST(c_div_round_up(1, non_constant_expr)), "");

                /* alternative calculation is [(x + y - 1) / y], but it may overflow */
                for (i = 0; i <= 0xffff; ++i) {
                        for (j = 1; j <= 0xff; ++j)
                                assert(c_div_round_up(i, j) == TEST_ALT_DIV(i, j));
                        for (j = 0xff00; j <= 0xffff; ++j)
                                assert(c_div_round_up(i, j) == TEST_ALT_DIV(i, j));
                }

                /* make sure it doesn't suffer from high overflow */
                assert(UINT32_C(0xfffffffa) % 10 == 0);
                assert(UINT32_C(0xfffffffa) / 10 == UINT32_C(429496729));
                assert(c_div_round_up(UINT32_C(0xfffffffa), 10) == UINT32_C(429496729));
                assert(TEST_ALT_DIV(UINT32_C(0xfffffffa), 10) == 0); /* overflow */

                assert(UINT32_C(0xfffffffd) % 10 == 3);
                assert(UINT32_C(0xfffffffd) / 10 == UINT32_C(429496729));
                assert(c_div_round_up(UINT32_C(0xfffffffd), 10) == UINT32_C(429496730));
                assert(TEST_ALT_DIV(UINT32_C(0xfffffffd), 10) == 0);
#undef TEST_ALT_DIV
        }

        /*
         * Test c_alloca8(). This is a variant of alloca() but guarantees
         * 8-byte alignment.
         */
        {
                void *p;
                int i;

                for (i = 1; i < 32; ++i) {
                        p = c_alloca8(i);
                        assert(p);
                        assert(!((unsigned long)p & 8));
                }
        }

        /*
         * Test c_errno(). Simply verify that the correct value is returned. It
         * must always be >0 and equivalent to `errno' if set.
         */
        {
                assert(c_errno() > 0);

                close(-1);
                assert(c_errno() == errno);

                errno = 0;
                assert(c_errno() != errno);
        }
}

int main(int argc, char **argv) {
        test_destructors();
        test_misc(argc);
        return 0;
}
