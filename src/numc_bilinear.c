#include "numc_bilinear.h"

#include <stdlib.h>
#include <math.h>

/* Tag stamped in the magic field by numc_matrix_from() and cleared by
 * numc_matrix_destroy(). Checked by numc_matrix_is_valid(). */
#define NUMC_MATRIX_MAGIC 0x4D545258u /* 'MTRX' */

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
numc_matrix_from(size_t rows, size_t cols, const double *data, numc_matrix_t *out)
{
    if (rows == 0 || cols == 0 || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    numc_matrix_t m;
    m.magic = NUMC_MATRIX_MAGIC;
    m.rows = rows;
    m.cols = cols;
    m.data = malloc(rows * cols * sizeof(double));
    if (m.data == NULL) {
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < rows * cols; i++) m.data[i] = data[i];
    *out = m;
    return NUMC_OK;
}

void
numc_matrix_destroy(numc_matrix_t *m)
{
    if (m == NULL) return;
    free(m->data);
    m->magic = 0;
    m->data = NULL;
    m->rows = 0;
    m->cols = 0;
}

bool
numc_matrix_is_valid(const numc_matrix_t *m)
{
    return m != NULL && m->magic == NUMC_MATRIX_MAGIC && m->data != NULL;
}

numc_status_t
numc_matrix_get(const numc_matrix_t *m, size_t i, size_t j, double *out)
{
    if (!numc_matrix_is_valid(m) || out == NULL) return NUMC_ERR_INVALID_ARG;
    if (i >= m->rows || j >= m->cols) return fail_out_of_bounds();
    *out = m->data[i * m->cols + j];
    return NUMC_OK;
}

numc_status_t
numc_matrix_set(numc_matrix_t *m, size_t i, size_t j, double val)
{
    if (!numc_matrix_is_valid(m)) return NUMC_ERR_INVALID_ARG;
    if (i >= m->rows || j >= m->cols) return fail_out_of_bounds();
    m->data[i * m->cols + j] = val;
    return NUMC_OK;
}

numc_status_t
numc_matrix_add(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a) || !numc_matrix_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.rows != b.rows || a.cols != b.cols) return fail_dim_mismatch();

    numc_matrix_t r;
    r.magic = NUMC_MATRIX_MAGIC;
    r.rows = a.rows;
    r.cols = a.cols;
    r.data = malloc(a.rows * a.cols * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.rows * a.cols; i++) r.data[i] = a.data[i] + b.data[i];
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_matrix_sub(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a) || !numc_matrix_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.rows != b.rows || a.cols != b.cols) return fail_dim_mismatch();

    numc_matrix_t r;
    r.magic = NUMC_MATRIX_MAGIC;
    r.rows = a.rows;
    r.cols = a.cols;
    r.data = malloc(a.rows * a.cols * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.rows * a.cols; i++) r.data[i] = a.data[i] - b.data[i];
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_matrix_scale(numc_matrix_t a, double scalar, numc_matrix_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    numc_matrix_t r;
    r.magic = NUMC_MATRIX_MAGIC;
    r.rows = a.rows;
    r.cols = a.cols;
    r.data = malloc(a.rows * a.cols * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.rows * a.cols; i++) r.data[i] = a.data[i] * scalar;
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_matrix_mul(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a) || !numc_matrix_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (a.cols != b.rows) return fail_dim_mismatch();

    numc_matrix_t r;
    r.magic = NUMC_MATRIX_MAGIC;
    r.rows = a.rows;
    r.cols = b.cols;
    r.data = malloc(r.rows * r.cols * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;

    for (size_t i = 0; i < r.rows; i++) {
        for (size_t j = 0; j < r.cols; j++) {
            double sum = 0.0;
            for (size_t k = 0; k < a.cols; k++)
                sum += a.data[i * a.cols + k] * b.data[k * b.cols + j];
            r.data[i * r.cols + j] = sum;
        }
    }
    *out = r;
    return NUMC_OK;
}

numc_status_t
numc_matrix_norm(numc_matrix_t a, double *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    double sum = 0.0;
    for (size_t i = 0; i < a.rows * a.cols; i++) sum += a.data[i] * a.data[i];
    *out = sqrt(sum);
    return NUMC_OK;
}

numc_status_t
numc_matrix_transpose(numc_matrix_t a, numc_matrix_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_matrix_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    numc_matrix_t r;
    r.magic = NUMC_MATRIX_MAGIC;
    r.rows = a.cols;
    r.cols = a.rows;
    r.data = malloc(r.rows * r.cols * sizeof(double));
    if (r.data == NULL) return NUMC_ERR_ALLOC;

    for (size_t i = 0; i < r.rows; i++)
        for (size_t j = 0; j < r.cols; j++)
            r.data[i * r.cols + j] = a.data[j * a.cols + i];
    *out = r;
    return NUMC_OK;
}
