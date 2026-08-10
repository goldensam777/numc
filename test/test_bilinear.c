/*
 * test_bilinear.c
 *
 * Tests du module numc_bilinear (numc_matrix_t, futures formes bilineaires).
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <math.h>
#include <cmocka.h>

#include "numc.h"
#include "test_bilinear.h"


static void
test_matrix_mul(void** state)
{
    (void) state;
    /* [[1,2],[3,4]] * [[5,6],[7,8]] = [[19,22],[43,50]] */
    double da[] = {1, 2, 3, 4};
    double db[] = {5, 6, 7, 8};
    numc_matrix_t a, b, out;
    numc_matrix_from(2, 2, da, &a);
    numc_matrix_from(2, 2, db, &b);

    assert_int_equal(numc_matrix_mul(a, b, &out), NUMC_OK);
    double x;
    numc_matrix_get(&out, 0, 0, &x); assert_double_equal(x, 19.0, 1e-12);
    numc_matrix_get(&out, 0, 1, &x); assert_double_equal(x, 22.0, 1e-12);
    numc_matrix_get(&out, 1, 0, &x); assert_double_equal(x, 43.0, 1e-12);
    numc_matrix_get(&out, 1, 1, &x); assert_double_equal(x, 50.0, 1e-12);

    numc_matrix_destroy(&a);
    numc_matrix_destroy(&b);
    numc_matrix_destroy(&out);
}


static void
test_matrix_mul_dim_mismatch(void** state)
{
    (void) state;
    double da[] = {1, 2, 3, 4, 5, 6}; /* 2x3 */
    double db[] = {1, 2, 3, 4};       /* 2x2 */
    numc_matrix_t a, b, out;
    numc_matrix_from(2, 3, da, &a);
    numc_matrix_from(2, 2, db, &b);

    assert_int_equal(numc_matrix_mul(a, b, &out), NUMC_ERR_DIM_MISMATCH);

    numc_matrix_destroy(&a);
    numc_matrix_destroy(&b);
}


static void
test_matrix_transpose(void** state)
{
    (void) state;
    double d[] = {1, 2, 3, 4, 5, 6}; /* 2x3 */
    numc_matrix_t a, t;
    numc_matrix_from(2, 3, d, &a);

    assert_int_equal(numc_matrix_transpose(a, &t), NUMC_OK);
    assert_int_equal((int)t.rows, 3);
    assert_int_equal((int)t.cols, 2);

    double x;
    numc_matrix_get(&t, 0, 0, &x); assert_double_equal(x, 1.0, 1e-12);
    numc_matrix_get(&t, 2, 1, &x); assert_double_equal(x, 6.0, 1e-12);

    numc_matrix_destroy(&a);
    numc_matrix_destroy(&t);
}


static void
test_matrix_norm(void** state)
{
    (void) state;
    double d[] = {3, 0, 0, 4}; /* diag(3,4) */
    numc_matrix_t a;
    numc_matrix_from(2, 2, d, &a);

    double out;
    assert_int_equal(numc_matrix_norm(a, &out), NUMC_OK);
    assert_double_equal(out, 5.0, 1e-12);

    numc_matrix_destroy(&a);
}


static void
test_matrix_zeroed_is_invalid(void** state)
{
    (void) state;
    numc_matrix_t m = {0};
    assert_false(numc_matrix_is_valid(&m));

    double x;
    assert_int_equal(numc_matrix_get(&m, 0, 0, &x), NUMC_ERR_INVALID_ARG);
}


static const struct CMUnitTest numc_bilinear_tests[] = {
    cmocka_unit_test(test_matrix_mul),
    cmocka_unit_test(test_matrix_mul_dim_mismatch),
    cmocka_unit_test(test_matrix_transpose),
    cmocka_unit_test(test_matrix_norm),
    cmocka_unit_test(test_matrix_zeroed_is_invalid),
};


int
numc_bilinear_tests_run(void)
{
    return cmocka_run_group_tests(numc_bilinear_tests, NULL, NULL);
}
