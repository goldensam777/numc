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


static void
test_matrix_vec_mul(void** state)
{
    (void) state;
    /* [[1, 2], [3, 4]] * [5, 6] = [17, 39] */
    double dm[] = {1, 2, 3, 4};
    double dv[] = {5, 6};
    numc_matrix_t m;
    numc_vector_t v, out;
    numc_matrix_from(2, 2, dm, &m);
    numc_vector_from(2, dv, &v);

    assert_int_equal(numc_matrix_vec_mul(m, v, &out), NUMC_OK);
    double x;
    numc_vector_get(&out, 0, &x); assert_double_equal(x, 17.0, 1e-12);
    numc_vector_get(&out, 1, &x); assert_double_equal(x, 39.0, 1e-12);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&v);
    numc_vector_destroy(&out);
}


static void
test_linear_map_apply(void** state)
{
    (void) state;
    /* f(x) = [[1, 2], [3, 4]] * [5, 6] + [10, 20] = [27, 59] */
    double dm[] = {1, 2, 3, 4};
    double du[] = {10, 20};
    double dx[] = {5, 6};
    numc_matrix_t m;
    numc_vector_t u, x, out;
    numc_linear_map_t map;

    numc_matrix_from(2, 2, dm, &m);
    numc_vector_from(2, du, &u);
    numc_vector_from(2, dx, &x);

    assert_int_equal(numc_linear_map_from(m, u, &map), NUMC_OK);
    assert_true(numc_linear_map_is_valid(&map));

    assert_int_equal(numc_linear_map_apply(&map, x, &out), NUMC_OK);
    double val;
    numc_vector_get(&out, 0, &val); assert_double_equal(val, 27.0, 1e-12);
    numc_vector_get(&out, 1, &val); assert_double_equal(val, 59.0, 1e-12);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&u);
    numc_vector_destroy(&x);
    numc_linear_map_destroy(&map);
    numc_vector_destroy(&out);
    assert_false(numc_linear_map_is_valid(&map));
}


static void
test_affine_map_apply(void** state)
{
    (void) state;
    /* f(x) = [[2, 0], [0, 3]] * ([5, 6] - [1, 2]) + [10, 20]
            = [[2, 0], [0, 3]] * [4, 4] + [10, 20]
            = [8, 12] + [10, 20] = [18, 32] */
    double dm[] = {2, 0, 0, 3};
    double du[] = {1, 2};
    double dv[] = {10, 20};
    double dx[] = {5, 6};
    numc_matrix_t m;
    numc_vector_t u, v, x, out;
    numc_affine_map_t map;

    numc_matrix_from(2, 2, dm, &m);
    numc_vector_from(2, du, &u);
    numc_vector_from(2, dv, &v);
    numc_vector_from(2, dx, &x);

    assert_int_equal(numc_affine_map_from(m, u, v, &map), NUMC_OK);
    assert_true(numc_affine_map_is_valid(&map));

    assert_int_equal(numc_affine_map_apply(&map, x, &out), NUMC_OK);
    double val;
    numc_vector_get(&out, 0, &val); assert_double_equal(val, 18.0, 1e-12);
    numc_vector_get(&out, 1, &val); assert_double_equal(val, 32.0, 1e-12);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&u);
    numc_vector_destroy(&v);
    numc_vector_destroy(&x);
    numc_affine_map_destroy(&map);
    numc_vector_destroy(&out);
    assert_false(numc_affine_map_is_valid(&map));
}


static void
test_matrix_vec_mul_dim_mismatch(void** state)
{
    (void) state;
    double dm[] = {1, 2, 3, 4, 5, 6}; /* 2x3 matrix */
    double dv[] = {1, 2};             /* 2D vector (mismatches 3 cols) */
    numc_matrix_t m;
    numc_vector_t v, out;
    numc_matrix_from(2, 3, dm, &m);
    numc_vector_from(2, dv, &v);

    assert_int_equal(numc_matrix_vec_mul(m, v, &out), NUMC_ERR_DIM_MISMATCH);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&v);
}


static void
test_linear_map_from_dim_mismatch(void** state)
{
    (void) state;
    double dm[] = {1, 2, 3, 4, 5, 6}; /* 2x3 matrix (2 rows) */
    double du[] = {1, 2, 3};          /* 3D vector (mismatches 2 rows) */
    numc_matrix_t m;
    numc_vector_t u;
    numc_linear_map_t map;

    numc_matrix_from(2, 3, dm, &m);
    numc_vector_from(3, du, &u);

    assert_int_equal(numc_linear_map_from(m, u, &map), NUMC_ERR_DIM_MISMATCH);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&u);
}


static void
test_linear_map_apply_dim_mismatch(void** state)
{
    (void) state;
    double dm[] = {1, 2, 3, 4, 5, 6}; /* 2x3 matrix (3 cols) */
    double du[] = {10, 20};           /* 2D vector (2 rows) */
    double dx[] = {1, 2};             /* 2D input vector (mismatches 3 cols) */
    numc_matrix_t m;
    numc_vector_t u, x, out;
    numc_linear_map_t map;

    numc_matrix_from(2, 3, dm, &m);
    numc_vector_from(2, du, &u);
    numc_vector_from(2, dx, &x);

    assert_int_equal(numc_linear_map_from(m, u, &map), NUMC_OK);
    assert_int_equal(numc_linear_map_apply(&map, x, &out), NUMC_ERR_DIM_MISMATCH);

    numc_matrix_destroy(&m);
    numc_vector_destroy(&u);
    numc_vector_destroy(&x);
    numc_linear_map_destroy(&map);
}


static void
test_linear_map_zeroed_is_invalid(void** state)
{
    (void) state;
    numc_linear_map_t map = {0};
    assert_false(numc_linear_map_is_valid(&map));

    double dx[] = {1, 2};
    numc_vector_t x, out;
    numc_vector_from(2, dx, &x);
    assert_int_equal(numc_linear_map_apply(&map, x, &out), NUMC_ERR_INVALID_ARG);
    numc_vector_destroy(&x);
}


static const struct CMUnitTest numc_bilinear_tests[] = {
    cmocka_unit_test(test_matrix_mul),
    cmocka_unit_test(test_matrix_mul_dim_mismatch),
    cmocka_unit_test(test_matrix_transpose),
    cmocka_unit_test(test_matrix_norm),
    cmocka_unit_test(test_matrix_zeroed_is_invalid),
    cmocka_unit_test(test_matrix_vec_mul),
    cmocka_unit_test(test_matrix_vec_mul_dim_mismatch),
    cmocka_unit_test(test_linear_map_apply),
    cmocka_unit_test(test_linear_map_from_dim_mismatch),
    cmocka_unit_test(test_linear_map_apply_dim_mismatch),
    cmocka_unit_test(test_linear_map_zeroed_is_invalid),
    cmocka_unit_test(test_affine_map_apply),
};


int
numc_bilinear_tests_run(void)
{
    return cmocka_run_group_tests(numc_bilinear_tests, NULL, NULL);
}
