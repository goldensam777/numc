#include "numc_vector.h"

#include <stdlib.h>
#include <math.h>

/* Tag stamped in the magic field by numc_vector_from() and cleared by
 * numc_vector_destroy(). Checked by numc_vector_is_valid(). */
#define NUMC_VECTOR_MAGIC 0x56435452u /* 'VCTR' */

/* Error helpers: record a message via numc_set_error() and return the code. */
static numc_status_t
fail_dim_mismatch(void)
{
    numc_set_error(NUMC_ERR_DIM_MISMATCH, "operand dimensions do not match");
    return NUMC_ERR_DIM_MISMATCH;
}

static numc_status_t
fail_out_of_bounds(void)
{
    numc_set_error(NUMC_ERR_OUT_OF_BOUNDS, "index out of bounds");
    return NUMC_ERR_OUT_OF_BOUNDS;
}

numc_status_t
numc_vector_from(size_t size, const double *data, numc_vector_t *out)
{
    if (size == 0 || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    numc_vector_t v;
    v.magic = NUMC_VECTOR_MAGIC;
    v.size = size;
    v.data = malloc(size * sizeof(double));
    if (v.data == NULL) {
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < size; i++) v.data[i] = data[i];
    *out = v;
    return NUMC_OK;
}

void
numc_vector_destroy(numc_vector_t *v)
{
    if (v == NULL) return;
    free(v->data);
    v->magic = 0;
    v->data = NULL;
    v->size = 0;
}

bool
numc_vector_is_valid(const numc_vector_t *v)
{
    return v != NULL && v->magic == NUMC_VECTOR_MAGIC && v->data != NULL;
}

numc_status_t
numc_vector_get(const numc_vector_t *v, size_t i, double *out)
{
    if (!numc_vector_is_valid(v) || out == NULL) return NUMC_ERR_INVALID_ARG;
    if (i >= v->size) return fail_out_of_bounds();
    *out = v->data[i];
    return NUMC_OK;
}

numc_status_t
numc_vector_set(numc_vector_t *v, size_t i, double val)
{
    if (!numc_vector_is_valid(v)) return NUMC_ERR_INVALID_ARG;
    if (i >= v->size) return fail_out_of_bounds();
    v->data[i] = val;
    return NUMC_OK;
}

numc_status_t
numc_vector_add(numc_vector_t a, numc_vector_t b, numc_vector_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_vector_is_valid(&a) || !numc_vector_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.size != b.size) return fail_dim_mismatch();

    numc_vector_t r;
    r.magic = NUMC_VECTOR_MAGIC;
    r.size = a.size;
    r.data = malloc(a.size * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.size; i++) r.data[i] = a.data[i] + b.data[i];
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_vector_sub(numc_vector_t a, numc_vector_t b, numc_vector_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_vector_is_valid(&a) || !numc_vector_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.size != b.size) return fail_dim_mismatch();

    numc_vector_t r;
    r.magic = NUMC_VECTOR_MAGIC;
    r.size = a.size;
    r.data = malloc(a.size * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.size; i++) r.data[i] = a.data[i] - b.data[i];
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_vector_scale(numc_vector_t a, double scalar, numc_vector_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_vector_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    numc_vector_t r;
    r.magic = NUMC_VECTOR_MAGIC;
    r.size = a.size;
    r.data = malloc(a.size * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.size; i++) r.data[i] = a.data[i] * scalar;
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_vector_dot(numc_vector_t a, numc_vector_t b, double *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_vector_is_valid(&a) || !numc_vector_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.size != b.size) return fail_dim_mismatch();

    double sum = 0.0;
    for (size_t i = 0; i < a.size; i++) sum += a.data[i] * b.data[i];
    *out = sum;
    return NUMC_OK;
}

numc_status_t
numc_vector_norm(numc_vector_t a, double *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_vector_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    double sum = 0.0;
    for (size_t i = 0; i < a.size; i++) sum += a.data[i] * a.data[i];
    *out = sqrt(sum);
    return NUMC_OK;
}
