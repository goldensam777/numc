#ifndef NUMC_COMPLEX_H
#define NUMC_COMPLEX_H

#include <stdbool.h>

#include "numc_status.h"

/*
 * Complex numbers (a = re + i*im).
 *
 * Operations that can always succeed return numc_complex_t by value.
 * Operations that can fail (div, pow) return numc_status_t and write
 * their result into an output parameter.
 */

typedef struct numc_complex {
    double re;
    double im;
} numc_complex_t;

/* Constructors */
numc_complex_t numc_complex_from(double re, double im);
numc_complex_t numc_complex_from_polar(double r, double theta);
numc_complex_t numc_complex_zero(void);
numc_complex_t numc_complex_one(void);
numc_complex_t numc_complex_i(void);

/* Arithmetic (total, return by value) */
numc_complex_t numc_complex_add(numc_complex_t a, numc_complex_t b);
numc_complex_t numc_complex_sub(numc_complex_t a, numc_complex_t b);
numc_complex_t numc_complex_mul(numc_complex_t a, numc_complex_t b);
numc_complex_t numc_complex_conj(numc_complex_t a);

/* Real-valued quantities */
double numc_complex_abs(numc_complex_t z);
double numc_complex_arg(numc_complex_t z);

/* Comparison. equal() uses an absolute tolerance on each component,
 * appropriate for the small-magnitude values this module typically
 * produces; scale epsilon up for comparisons involving large values. */
bool numc_complex_is_zero(numc_complex_t z);
bool numc_complex_is_finite(numc_complex_t z);
bool numc_complex_equal(numc_complex_t a, numc_complex_t b, double epsilon);

/*
 * Principal square root (branch with re >= 0). Total: sqrt(0) = 0.
 */
numc_complex_t numc_complex_sqrt(numc_complex_t z);

/*
 * e^z = e^re * (cos(im) + i*sin(im)). Total: never fails.
 */
numc_complex_t numc_complex_exp(numc_complex_t z);

/*
 * Principal logarithm: log|z| + i*arg(z).
 * Returns NUMC_ERR_DOMAIN if z == 0. Returns NUMC_ERR_INVALID_ARG if out is NULL.
 */
numc_status_t numc_complex_log(numc_complex_t z, numc_complex_t* out);

/*
 * z^w for complex w. Generalizes numc_complex_pow (integer exponent).
 * Returns NUMC_ERR_DOMAIN if base == 0 and (exp == 0 is the only defined case: 0^0 = 1).
 * Returns NUMC_ERR_INVALID_ARG if out is NULL.
 */
numc_status_t numc_complex_cpow(numc_complex_t base, numc_complex_t exp, numc_complex_t* out);

/* Trigonometric (total: sin, cos never fail) */
numc_complex_t numc_complex_sin(numc_complex_t z);
numc_complex_t numc_complex_cos(numc_complex_t z);

/*
 * tan(z) = sin(z) / cos(z). Returns NUMC_ERR_DOMAIN if cos(z) == 0
 * (poles at z = pi/2 + k*pi on the real axis).
 */
numc_status_t numc_complex_tan(numc_complex_t z, numc_complex_t* out);

/* Hyperbolic (total: never fail) */
numc_complex_t numc_complex_sinh(numc_complex_t z);
numc_complex_t numc_complex_cosh(numc_complex_t z);

/*
 * a / b. Returns NUMC_ERR_DOMAIN if b == 0.
 * Returns NUMC_ERR_INVALID_ARG if out is NULL.
 */
numc_status_t numc_complex_div(numc_complex_t a, numc_complex_t b,
                               numc_complex_t* out);

/*
 * a^n for integer exponent n, computed by exponentiation by squaring.
 * Returns NUMC_ERR_DOMAIN if a == 0 and n < 0.
 * Returns NUMC_ERR_INVALID_ARG if out is NULL.
 */
numc_status_t numc_complex_pow(numc_complex_t a, int n, numc_complex_t* out);

#endif /* NUMC_COMPLEX_H */