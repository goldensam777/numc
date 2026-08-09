/*
 * test_complex.c
 *
 * Tests du module complex (nombres complexes).
 */


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <math.h>
#include <cmocka.h>

#include "numc.h"
#include "test_complex.h"

/* M_PI is POSIX, not strict C11; define it if the environment doesn't. */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


static void
assert_complex_equal(numc_complex_t z, double re, double im)
{
    assert_double_equal(z.re, re, 1e-12);
    assert_double_equal(z.im, im, 1e-12);
}


static void
test_from(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_from(1.5, -2.5);
    assert_complex_equal(z, 1.5, -2.5);
}


static void
test_add(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_add(numc_complex_from(1.0, 2.0),
                                        numc_complex_from(3.0, 4.0));
    assert_complex_equal(z, 4.0, 6.0);
}


static void
test_sub(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_sub(numc_complex_from(5.0, 7.0),
                                        numc_complex_from(2.0, 3.0));
    assert_complex_equal(z, 3.0, 4.0);
}


static void
test_mul(void** state)
{
    (void) state;
    /* (1 + 2i)(3 + 4i) = -5 + 10i */
    numc_complex_t z = numc_complex_mul(numc_complex_from(1.0, 2.0),
                                        numc_complex_from(3.0, 4.0));
    assert_complex_equal(z, -5.0, 10.0);
}


static void
test_conj(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_conj(numc_complex_from(1.0, -2.0));
    assert_complex_equal(z, 1.0, 2.0);
}


static void
test_abs(void** state)
{
    (void) state;
    assert_double_equal(numc_complex_abs(numc_complex_from(3.0, 4.0)),
                        5.0, 1e-12);
}


static void
test_arg(void** state)
{
    (void) state;
    assert_double_equal(numc_complex_arg(numc_complex_from(1.0, 0.0)),
                        0.0, 1e-12);
    assert_double_equal(numc_complex_arg(numc_complex_from(0.0, 1.0)),
                        0.5 * 3.141592653589793, 1e-12);
    assert_double_equal(numc_complex_arg(numc_complex_from(-1.0, 0.0)),
                        3.141592653589793, 1e-12);
}


static void
test_div(void** state)
{
    (void) state;
    /* (3 + 4i) / (1 + 2i) = 2.2 - 0.4i */
    numc_complex_t q;
    numc_status_t st = numc_complex_div(numc_complex_from(3.0, 4.0),
                                        numc_complex_from(1.0, 2.0), &q);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(q, 2.2, -0.4);
}


static void
test_div_by_zero(void** state)
{
    (void) state;
    numc_complex_t q;
    numc_status_t st = numc_complex_div(numc_complex_from(1.0, 1.0),
                                        numc_complex_from(0.0, 0.0), &q);
    assert_int_equal(st, NUMC_ERR_DOMAIN);
}


static void
test_div_null_out(void** state)
{
    (void) state;
    numc_status_t st = numc_complex_div(numc_complex_from(1.0, 1.0),
                                        numc_complex_from(1.0, 0.0), NULL);
    assert_int_equal(st, NUMC_ERR_INVALID_ARG);
}


static void
test_pow_positive_exp(void** state)
{
    (void) state;
    /* (1 + i)^4 = -4 */
    numc_complex_t z;
    numc_status_t st = numc_complex_pow(numc_complex_from(1.0, 1.0), 4, &z);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(z, -4.0, 0.0);
}


static void
test_pow_negative_exp(void** state)
{
    (void) state;
    /* (2 + 0i)^-2 = 0.25 */
    numc_complex_t z;
    numc_status_t st = numc_complex_pow(numc_complex_from(2.0, 0.0), -2, &z);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(z, 0.25, 0.0);
}


static void
test_pow_exp_zero(void** state)
{
    (void) state;
    /* z^0 = 1 for any z != 0 */
    numc_complex_t z;
    numc_status_t st = numc_complex_pow(numc_complex_from(3.0, -1.0), 0, &z);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(z, 1.0, 0.0);
}


static void
test_pow_zero_positive(void** state)
{
    (void) state;
    /* 0^5 = 0 */
    numc_complex_t z;
    numc_status_t st = numc_complex_pow(numc_complex_from(0.0, 0.0), 5, &z);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(z, 0.0, 0.0);
}


static void
test_pow_zero_negative(void** state)
{
    (void) state;
    /* 0^-2 is undefined */
    numc_complex_t z;
    numc_status_t st = numc_complex_pow(numc_complex_from(0.0, 0.0), -2, &z);
    assert_int_equal(st, NUMC_ERR_DOMAIN);
}


static void
test_pow_null_out(void** state)
{
    (void) state;
    numc_status_t st = numc_complex_pow(numc_complex_from(1.0, 0.0), 2, NULL);
    assert_int_equal(st, NUMC_ERR_INVALID_ARG);
}


static void
test_from_polar(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_from_polar(2.0, 0.0);
    assert_complex_equal(z, 2.0, 0.0);

    /* r=1, theta=pi/2 -> i */
    numc_complex_t w = numc_complex_from_polar(1.0, M_PI / 2.0);
    assert_true(numc_complex_equal(w, numc_complex_i(), 1e-9));
}


static void
test_zero_one_i_constants(void** state)
{
    (void) state;
    assert_complex_equal(numc_complex_zero(), 0.0, 0.0);
    assert_complex_equal(numc_complex_one(), 1.0, 0.0);
    assert_complex_equal(numc_complex_i(), 0.0, 1.0);
}


static void
test_is_zero(void** state)
{
    (void) state;
    assert_true(numc_complex_is_zero(numc_complex_zero()));
    assert_false(numc_complex_is_zero(numc_complex_from(0.0, 1e-9)));
}


static void
test_is_finite(void** state)
{
    (void) state;
    assert_true(numc_complex_is_finite(numc_complex_from(1.0, -1.0)));

    numc_complex_t nan_z = numc_complex_from(NAN, 0.0);
    assert_false(numc_complex_is_finite(nan_z));

    numc_complex_t inf_z = numc_complex_from(0.0, INFINITY);
    assert_false(numc_complex_is_finite(inf_z));
}


static void
test_equal(void** state)
{
    (void) state;
    numc_complex_t a = numc_complex_from(1.0, 2.0);
    numc_complex_t b = numc_complex_from(1.0 + 1e-10, 2.0 - 1e-10);
    assert_true(numc_complex_equal(a, b, 1e-9));
    assert_false(numc_complex_equal(a, b, 1e-12));
}


static void
test_sqrt_positive_real(void** state)
{
    (void) state;
    /* sqrt(4) = 2 */
    numc_complex_t z = numc_complex_sqrt(numc_complex_from(4.0, 0.0));
    assert_complex_equal(z, 2.0, 0.0);
}


static void
test_sqrt_negative_real(void** state)
{
    (void) state;
    /* sqrt(-4) = 2i (principal branch: re >= 0) */
    numc_complex_t z = numc_complex_sqrt(numc_complex_from(-4.0, 0.0));
    assert_complex_equal(z, 0.0, 2.0);
}


static void
test_sqrt_negative_imag(void** state)
{
    (void) state;
    /* sqrt(-i) should have im < 0, and squaring must return -i */
    numc_complex_t z = numc_complex_sqrt(numc_complex_from(0.0, -1.0));
    numc_complex_t z2 = numc_complex_mul(z, z);
    assert_true(numc_complex_equal(z2, numc_complex_from(0.0, -1.0), 1e-9));
}


static void
test_sqrt_zero(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_sqrt(numc_complex_zero());
    assert_complex_equal(z, 0.0, 0.0);
}


static void
test_exp_zero(void** state)
{
    (void) state;
    /* e^0 = 1 */
    numc_complex_t z = numc_complex_exp(numc_complex_zero());
    assert_complex_equal(z, 1.0, 0.0);
}


static void
test_exp_i_pi(void** state)
{
    (void) state;
    /* Euler: e^(i*pi) = -1 */
    numc_complex_t z = numc_complex_exp(numc_complex_from(0.0, M_PI));
    assert_true(numc_complex_equal(z, numc_complex_from(-1.0, 0.0), 1e-9));
}


static void
test_log_basic(void** state)
{
    (void) state;
    numc_complex_t out;
    /* log(1) = 0 */
    numc_status_t st = numc_complex_log(numc_complex_one(), &out);
    assert_int_equal(st, NUMC_OK);
    assert_complex_equal(out, 0.0, 0.0);
}


static void
test_log_is_exp_inverse(void** state)
{
    (void) state;
    numc_complex_t z = numc_complex_from(1.0, 2.0);
    numc_complex_t log_z;
    assert_int_equal(numc_complex_log(z, &log_z), NUMC_OK);
    numc_complex_t back = numc_complex_exp(log_z);
    assert_true(numc_complex_equal(back, z, 1e-9));
}


static void
test_log_zero(void** state)
{
    (void) state;
    numc_complex_t out;
    numc_status_t st = numc_complex_log(numc_complex_zero(), &out);
    assert_int_equal(st, NUMC_ERR_DOMAIN);
}


static void
test_log_null_out(void** state)
{
    (void) state;
    numc_status_t st = numc_complex_log(numc_complex_one(), NULL);
    assert_int_equal(st, NUMC_ERR_INVALID_ARG);
}


static const struct CMUnitTest numc_complex_tests[] = {
    cmocka_unit_test(test_from),
    cmocka_unit_test(test_add),
    cmocka_unit_test(test_sub),
    cmocka_unit_test(test_mul),
    cmocka_unit_test(test_conj),
    cmocka_unit_test(test_abs),
    cmocka_unit_test(test_arg),
    cmocka_unit_test(test_div),
    cmocka_unit_test(test_div_by_zero),
    cmocka_unit_test(test_div_null_out),
    cmocka_unit_test(test_pow_positive_exp),
    cmocka_unit_test(test_pow_negative_exp),
    cmocka_unit_test(test_pow_exp_zero),
    cmocka_unit_test(test_pow_zero_positive),
    cmocka_unit_test(test_pow_zero_negative),
    cmocka_unit_test(test_pow_null_out),
    cmocka_unit_test(test_from_polar),
    cmocka_unit_test(test_zero_one_i_constants),
    cmocka_unit_test(test_is_zero),
    cmocka_unit_test(test_is_finite),
    cmocka_unit_test(test_equal),
    cmocka_unit_test(test_sqrt_positive_real),
    cmocka_unit_test(test_sqrt_negative_real),
    cmocka_unit_test(test_sqrt_negative_imag),
    cmocka_unit_test(test_sqrt_zero),
    cmocka_unit_test(test_exp_zero),
    cmocka_unit_test(test_exp_i_pi),
    cmocka_unit_test(test_log_basic),
    cmocka_unit_test(test_log_is_exp_inverse),
    cmocka_unit_test(test_log_zero),
    cmocka_unit_test(test_log_null_out),
};


int
numc_complex_tests_run(void)
{
    return cmocka_run_group_tests(numc_complex_tests, NULL, NULL);
}