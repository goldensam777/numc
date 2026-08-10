/*
 * test_vector.c
 *
 * Tests du module numc_vector (numc_vector_t).
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <math.h>
#include <cmocka.h>

#include "numc.h"
#include "test_vector.h"


static void
test_vector_from_and_get(void** state)
{
    (void) state;
    double d[] = {1.0, 2.0, 3.0};
    numc_vector_t v;
    assert_int_equal(numc_vector_from(3, d, &v), NUMC_OK);

    double x;
    assert_int_equal(numc_vector_get(&v, 1, &x), NUMC_OK);
    assert_double_equal(x, 2.0, 1e-12);

    assert_int_equal(numc_vector_get(&v, 3, &x), NUMC_ERR_OUT_OF_BOUNDS);

    numc_vector_destroy(&v);
    assert_false(numc_vector_is_valid(&v));
}


static void
test_vector_add_sub(void** state)
{
    (void) state;
    double da[] = {1.0, 2.0, 3.0};
    double db[] = {4.0, 5.0, 6.0};
    numc_vector_t a, b, sum, diff;
    numc_vector_from(3, da, &a);
    numc_vector_from(3, db, &b);

    assert_int_equal(numc_vector_add(a, b, &sum), NUMC_OK);
    double x;
    numc_vector_get(&sum, 0, &x); assert_double_equal(x, 5.0, 1e-12);
    numc_vector_get(&sum, 2, &x); assert_double_equal(x, 9.0, 1e-12);

    assert_int_equal(numc_vector_sub(b, a, &diff), NUMC_OK);
    numc_vector_get(&diff, 0, &x); assert_double_equal(x, 3.0, 1e-12);

    numc_vector_destroy(&a);
    numc_vector_destroy(&b);
    numc_vector_destroy(&sum);
    numc_vector_destroy(&diff);
}


static void
test_vector_add_dim_mismatch(void** state)
{
    (void) state;
    double da[] = {1.0, 2.0};
    double db[] = {1.0, 2.0, 3.0};
    numc_vector_t a, b, out;
    numc_vector_from(2, da, &a);
    numc_vector_from(3, db, &b);

    assert_int_equal(numc_vector_add(a, b, &out), NUMC_ERR_DIM_MISMATCH);

    numc_vector_destroy(&a);
    numc_vector_destroy(&b);
}


static void
test_vector_dot(void** state)
{
    (void) state;
    double da[] = {1.0, 2.0, 3.0};
    double db[] = {4.0, 5.0, 6.0};
    numc_vector_t a, b;
    numc_vector_from(3, da, &a);
    numc_vector_from(3, db, &b);

    double out;
    assert_int_equal(numc_vector_dot(a, b, &out), NUMC_OK);
    /* 1*4 + 2*5 + 3*6 = 32 */
    assert_double_equal(out, 32.0, 1e-12);

    numc_vector_destroy(&a);
    numc_vector_destroy(&b);
}


static void
test_vector_norm(void** state)
{
    (void) state;
    double d[] = {3.0, 4.0};
    numc_vector_t v;
    numc_vector_from(2, d, &v);

    double out;
    assert_int_equal(numc_vector_norm(v, &out), NUMC_OK);
    assert_double_equal(out, 5.0, 1e-12); /* 3-4-5 */

    numc_vector_destroy(&v);
}


static void
test_vector_scale(void** state)
{
    (void) state;
    double d[] = {1.0, -2.0};
    numc_vector_t v, out;
    numc_vector_from(2, d, &v);

    assert_int_equal(numc_vector_scale(v, -3.0, &out), NUMC_OK);
    double x;
    numc_vector_get(&out, 0, &x); assert_double_equal(x, -3.0, 1e-12);
    numc_vector_get(&out, 1, &x); assert_double_equal(x, 6.0, 1e-12);

    numc_vector_destroy(&v);
    numc_vector_destroy(&out);
}


static void
test_vector_zeroed_is_invalid(void** state)
{
    (void) state;
    numc_vector_t v = {0};
    assert_false(numc_vector_is_valid(&v));

    double x;
    assert_int_equal(numc_vector_get(&v, 0, &x), NUMC_ERR_INVALID_ARG);

    numc_vector_t a, out;
    double da[] = {1.0, 2.0, 3.0};
    numc_vector_from(3, da, &a);
    assert_int_equal(numc_vector_add(v, a, &out), NUMC_ERR_INVALID_ARG);
    numc_vector_destroy(&a);
}


static void
test_vector_use_after_destroy_rejected(void** state)
{
    (void) state;
    double d[] = {1.0, 2.0};
    numc_vector_t v;
    numc_vector_from(2, d, &v);
    numc_vector_destroy(&v);

    assert_false(numc_vector_is_valid(&v));

    double x;
    assert_int_equal(numc_vector_get(&v, 0, &x), NUMC_ERR_INVALID_ARG);
    assert_int_equal(numc_vector_set(&v, 0, 3.0), NUMC_ERR_INVALID_ARG);
}


static const struct CMUnitTest numc_vector_tests[] = {
    cmocka_unit_test(test_vector_from_and_get),
    cmocka_unit_test(test_vector_add_sub),
    cmocka_unit_test(test_vector_add_dim_mismatch),
    cmocka_unit_test(test_vector_dot),
    cmocka_unit_test(test_vector_norm),
    cmocka_unit_test(test_vector_scale),
    cmocka_unit_test(test_vector_zeroed_is_invalid),
    cmocka_unit_test(test_vector_use_after_destroy_rejected),
};


int
numc_vector_tests_run(void)
{
    return cmocka_run_group_tests(numc_vector_tests, NULL, NULL);
}
