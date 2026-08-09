#include "complex.h"

#include <math.h>
#include <stddef.h>


numc_complex_t
numc_complex_from(double re, double im)
{
    numc_complex_t z = { re, im };
    return z;
}


numc_complex_t
numc_complex_from_polar(double r, double theta)
{
    numc_complex_t z = { r * cos(theta), r * sin(theta) };
    return z;
}


numc_complex_t
numc_complex_zero(void)
{
    numc_complex_t z = { 0.0, 0.0 };
    return z;
}


numc_complex_t
numc_complex_one(void)
{
    numc_complex_t z = { 1.0, 0.0 };
    return z;
}


numc_complex_t
numc_complex_i(void)
{
    numc_complex_t z = { 0.0, 1.0 };
    return z;
}


numc_complex_t
numc_complex_add(numc_complex_t a, numc_complex_t b)
{
    numc_complex_t z = { a.re + b.re, a.im + b.im };
    return z;
}


numc_complex_t
numc_complex_sub(numc_complex_t a, numc_complex_t b)
{
    numc_complex_t z = { a.re - b.re, a.im - b.im };
    return z;
}


numc_complex_t
numc_complex_mul(numc_complex_t a, numc_complex_t b)
{
    numc_complex_t z = {
        a.re * b.re - a.im * b.im,
        a.re * b.im + a.im * b.re,
    };
    return z;
}


numc_complex_t
numc_complex_conj(numc_complex_t a)
{
    numc_complex_t z = { a.re, -a.im };
    return z;
}


double
numc_complex_abs(numc_complex_t z)
{
    /* hypot() avoids the overflow that sqrt(re^2 + im^2) can hit
     * when a single component overflows even though |z| is finite. */
    return hypot(z.re, z.im);
}


double
numc_complex_arg(numc_complex_t z)
{
    return atan2(z.im, z.re);
}


bool
numc_complex_is_zero(numc_complex_t z)
{
    return z.re == 0.0 && z.im == 0.0;
}


bool
numc_complex_is_finite(numc_complex_t z)
{
    return isfinite(z.re) && isfinite(z.im);
}


bool
numc_complex_equal(numc_complex_t a, numc_complex_t b, double epsilon)
{
    return fabs(a.re - b.re) <= epsilon && fabs(a.im - b.im) <= epsilon;
}


numc_complex_t
numc_complex_sqrt(numc_complex_t z)
{
    /* Standard stable formula: derive from |z| and re(z) rather than
     * from arg(z)/2, avoiding the extra trig call and its rounding. */
    double m = numc_complex_abs(z);

    if (m == 0.0) {
        return numc_complex_zero();
    }

    double re = sqrt((m + z.re) / 2.0);
    double im = sqrt((m - z.re) / 2.0);

    if (z.im < 0.0) {
        im = -im;
    }

    numc_complex_t out = { re, im };
    return out;
}


numc_complex_t
numc_complex_exp(numc_complex_t z)
{
    double scale = exp(z.re);
    numc_complex_t out = { scale * cos(z.im), scale * sin(z.im) };
    return out;
}


numc_status_t
numc_complex_log(numc_complex_t z, numc_complex_t* out)
{
    if (out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }

    if (numc_complex_is_zero(z)) {
        numc_set_error(NUMC_ERR_DOMAIN, "log(0) is undefined");
        return NUMC_ERR_DOMAIN;
    }

    out->re = log(numc_complex_abs(z));
    out->im = numc_complex_arg(z);
    return NUMC_OK;
}


numc_status_t
numc_complex_div(numc_complex_t a, numc_complex_t b, numc_complex_t* out)
{
    if (out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }

    if (b.re == 0.0 && b.im == 0.0) {
        numc_set_error(NUMC_ERR_DOMAIN, "division by zero: b = 0");
        return NUMC_ERR_DOMAIN;
    }

    /* Smith's algorithm: divide by the larger component so that neither
     * denom nor the ratios overflow, and small components do not
     * underflow into an incorrect zero. */
    if (fabs(b.re) >= fabs(b.im)) {
        double ratio = b.im / b.re;
        double denom = b.re + b.im * ratio;
        out->re = (a.re + a.im * ratio) / denom;
        out->im = (a.im - a.re * ratio) / denom;
    } else {
        double ratio = b.re / b.im;
        double denom = b.re * ratio + b.im;
        out->re = (a.re * ratio + a.im) / denom;
        out->im = (a.im * ratio - a.re) / denom;
    }

    return NUMC_OK;
}


numc_status_t
numc_complex_pow(numc_complex_t a, int n, numc_complex_t* out)
{
    if (out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }

    if (n == 0) {
        out->re = 1.0;
        out->im = 0.0;
        return NUMC_OK;
    }

    if (a.re == 0.0 && a.im == 0.0) {
        if (n < 0) {
            numc_set_error(NUMC_ERR_DOMAIN, "0^n undefined for n < 0");
            return NUMC_ERR_DOMAIN;
        }
        out->re = 0.0;
        out->im = 0.0;
        return NUMC_OK;
    }

    /* Exponentiation by squaring: more accurate than De Moivre's
     * formula for integer exponents (no trig rounding). */
    unsigned long long m = (n < 0)
        ? (unsigned long long)(-(long long)n)
        : (unsigned long long)n;

    numc_complex_t base = a;
    numc_complex_t result = { 1.0, 0.0 };

    while (m > 0) {
        if (m & 1ULL) {
            result = numc_complex_mul(result, base);
        }
        base = numc_complex_mul(base, base);
        m >>= 1;
    }

    /* a^n = 1 / a^|n| for n < 0 (reuses the robust division). */
    if (n < 0) {
        return numc_complex_div(numc_complex_from(1.0, 0.0), result, out);
    }

    *out = result;
    return NUMC_OK;
}