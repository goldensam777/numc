/*
 * test_tensorial.c
 *
 * Tests du module numc_tensorial (numc_tensor_t).
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <math.h>
#include <cmocka.h>

#include "numc.h"
#include "test_tensorial.h"


static void
test_tensor_from_and_shape(void** state)
{
    (void) state;
    size_t dims[] = {2, 3};
    double d[] = {1, 2, 3, 4, 5, 6};
    numc_tensor_t t;
    assert_int_equal(numc_tensor_from(2, dims, d, &t), NUMC_OK);
    assert_int_equal((int)t.order, 2);
    assert_int_equal((int)t.strides[0], 3); /* row-major: stride of axis 0 = dims[1] */
    assert_int_equal((int)t.strides[1], 1);

    numc_tensor_destroy(&t);
    assert_false(numc_tensor_is_valid(&t));
}


static void
test_tensor_add_dim_mismatch(void** state)
{
    (void) state;
    size_t dims_a[] = {2, 3};
    size_t dims_b[] = {3, 2};
    double da[6] = {0}, db[6] = {0};
    numc_tensor_t a, b, out;
    numc_tensor_from(2, dims_a, da, &a);
    numc_tensor_from(2, dims_b, db, &b);

    assert_int_equal(numc_tensor_add(a, b, &out), NUMC_ERR_DIM_MISMATCH);

    numc_tensor_destroy(&a);
    numc_tensor_destroy(&b);
}


static void
test_tensor_contract_matches_matrix_mul(void** state)
{
    (void) state;
    /* Contracting two order-2 tensors along a's axis 1 and b's axis 0
     * must reproduce ordinary matrix multiplication. */
    size_t dims_a[] = {2, 2};
    size_t dims_b[] = {2, 2};
    double da[] = {1, 2, 3, 4};
    double db[] = {5, 6, 7, 8};

    numc_tensor_t ta, tb, tc;
    numc_tensor_from(2, dims_a, da, &ta);
    numc_tensor_from(2, dims_b, db, &tb);

    assert_int_equal(numc_tensor_contract(ta, 1, tb, 0, &tc), NUMC_OK);
    assert_int_equal((int)tc.order, 2);
    assert_int_equal((int)tc.dimensions[0], 2);
    assert_int_equal((int)tc.dimensions[1], 2);

    /* Same expected result as test_matrix_mul: [[19,22],[43,50]] */
    assert_double_equal(tc.data[0 * tc.strides[0] + 0 * tc.strides[1]], 19.0, 1e-12);
    assert_double_equal(tc.data[0 * tc.strides[0] + 1 * tc.strides[1]], 22.0, 1e-12);
    assert_double_equal(tc.data[1 * tc.strides[0] + 0 * tc.strides[1]], 43.0, 1e-12);
    assert_double_equal(tc.data[1 * tc.strides[0] + 1 * tc.strides[1]], 50.0, 1e-12);

    numc_tensor_destroy(&ta);
    numc_tensor_destroy(&tb);
    numc_tensor_destroy(&tc);
}


static void
test_tensor_contract_full_to_scalar(void** state)
{
    (void) state;
    /* Two order-1 tensors (vectors) contracted fully -> order-0 tensor,
     * value equal to the dot product. */
    size_t dims[] = {3};
    double da[] = {1, 2, 3};
    double db[] = {4, 5, 6};
    numc_tensor_t a, b, out;
    numc_tensor_from(1, dims, da, &a);
    numc_tensor_from(1, dims, db, &b);

    assert_int_equal(numc_tensor_contract(a, 0, b, 0, &out), NUMC_OK);
    assert_int_equal((int)out.order, 0);
    assert_double_equal(out.data[0], 32.0, 1e-12);

    numc_tensor_destroy(&a);
    numc_tensor_destroy(&b);
    numc_tensor_destroy(&out);
}


static void
test_tensor_transpose_permutation(void** state)
{
    (void) state;
    size_t dims[] = {2, 3};
    double d[] = {1, 2, 3, 4, 5, 6}; /* rows=2, cols=3 */
    numc_tensor_t a;
    numc_tensor_from(2, dims, d, &a);

    size_t perm[] = {1, 0}; /* transpose */
    numc_tensor_t t;
    assert_int_equal(numc_tensor_transpose(a, perm, &t), NUMC_OK);
    assert_int_equal((int)t.dimensions[0], 3);
    assert_int_equal((int)t.dimensions[1], 2);

    /* a[i][j] must equal t[j][i] */
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 3; j++) {
            double av = a.data[i * a.strides[0] + j * a.strides[1]];
            double tv = t.data[j * t.strides[0] + i * t.strides[1]];
            assert_double_equal(av, tv, 1e-12);
        }
    }

    numc_tensor_destroy(&a);
    numc_tensor_destroy(&t);
}


static void
test_tensor_transpose_invalid_permutation(void** state)
{
    (void) state;
    size_t dims[] = {2, 3};
    double d[6] = {0};
    numc_tensor_t a, t;
    numc_tensor_from(2, dims, d, &a);

    size_t bad_perm[] = {0, 0}; /* not a permutation: repeats 0, missing 1 */
    assert_int_equal(numc_tensor_transpose(a, bad_perm, &t), NUMC_ERR_INVALID_ARG);

    numc_tensor_destroy(&a);
}


static void
test_tensor_norm(void** state)
{
    (void) state;
    size_t dims[] = {2, 2};
    double d[] = {3, 0, 0, 4};
    numc_tensor_t a;
    numc_tensor_from(2, dims, d, &a);

    double out;
    assert_int_equal(numc_tensor_norm(a, &out), NUMC_OK);
    assert_double_equal(out, 5.0, 1e-12);

    numc_tensor_destroy(&a);
}


static const struct CMUnitTest numc_tensorial_tests[] = {
    cmocka_unit_test(test_tensor_from_and_shape),
    cmocka_unit_test(test_tensor_add_dim_mismatch),
    cmocka_unit_test(test_tensor_contract_matches_matrix_mul),
    cmocka_unit_test(test_tensor_contract_full_to_scalar),
    cmocka_unit_test(test_tensor_transpose_permutation),
    cmocka_unit_test(test_tensor_transpose_invalid_permutation),
    cmocka_unit_test(test_tensor_norm),
};


int
numc_tensorial_tests_run(void)
{
    return cmocka_run_group_tests(numc_tensorial_tests, NULL, NULL);
}
