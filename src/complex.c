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
