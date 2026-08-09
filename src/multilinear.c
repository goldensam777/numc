#include "multilinear.h"

#include <stdlib.h>
#include <math.h>

/* Tags stamped in the magic field by numc_*_from() and cleared by
 * numc_*_destroy(). Checked by numc_*_is_valid(). */
#define NUMC_VECTOR_MAGIC 0x56435452u /* 'VCTR' */
#define NUMC_MATRIX_MAGIC 0x4D545258u /* 'MTRX' */
#define NUMC_TENSOR_MAGIC 0x54534F52u /* 'TSOR' */

/* ---- internal helpers ------------------------------------------------ */

static void
compute_strides(size_t order, const size_t *dims, size_t *strides)
{
    if (order == 0) return;
    strides[order - 1] = 1;
    for (size_t i = order - 1; i > 0; i--)
        strides[i - 1] = strides[i] * dims[i];
}

static size_t
product(size_t order, const size_t *dims)
{
    size_t total = 1;
    for (size_t i = 0; i < order; i++) total *= dims[i];
    return total;
}

/* Allocates an uninitialized tensor with its own dimensions/strides copy. */
static numc_status_t
alloc_tensor(size_t order, const size_t *dims, numc_tensor_t *out)
{
    numc_tensor_t t;
    t.magic = NUMC_TENSOR_MAGIC;
    t.order = order;
    t.dimensions = malloc(order * sizeof(size_t));
    t.strides = malloc(order * sizeof(size_t));
    if (t.dimensions == NULL || t.strides == NULL) {
        free(t.dimensions);
        free(t.strides);
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < order; i++) t.dimensions[i] = dims[i];
    compute_strides(order, t.dimensions, t.strides);

    size_t total = product(order, dims);
    t.data = malloc(total * sizeof(double));
    if (t.data == NULL) {
        free(t.dimensions);
        free(t.strides);
        return NUMC_ERR_ALLOC;
    }
    *out = t;
    return NUMC_OK;
}

static bool
same_shape(size_t order_a, const size_t *dims_a, size_t order_b, const size_t *dims_b)
{
    if (order_a != order_b) return false;
    for (size_t i = 0; i < order_a; i++)
        if (dims_a[i] != dims_b[i]) return false;
    return true;
}

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

/* ---- construction ------------------------------------------------------ */

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


numc_status_t
numc_tensor_from(size_t order, const size_t *dimensions, const double *data, numc_tensor_t *out)
{
    if (order == 0 || dimensions == NULL || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    /* A zero dimension would make product() = 0 and all strides zero,
     * causing a division by zero in the index decomposition of
     * numc_tensor_transpose()/numc_tensor_contract(). */
    for (size_t i = 0; i < order; i++) {
        if (dimensions[i] == 0) {
            numc_set_error(NUMC_ERR_INVALID_ARG,
                           "tensor dimension %zu is zero", i);
            return NUMC_ERR_INVALID_ARG;
        }
    }
    numc_tensor_t t;
    numc_status_t st = alloc_tensor(order, dimensions, &t);
    if (st != NUMC_OK) return st;

    size_t total = product(order, dimensions);
    for (size_t i = 0; i < total; i++) t.data[i] = data[i];
    *out = t;
    return NUMC_OK;
}

/* ---- destruction / validity -------------------------------------------- */

void
numc_vector_destroy(numc_vector_t *v)
{
    if (v == NULL) return;
    free(v->data);
    v->magic = 0;
    v->data = NULL;
    v->size = 0;
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


void
numc_tensor_destroy(numc_tensor_t *t)
{
    if (t == NULL) return;
    free(t->data);
    free(t->dimensions);
    free(t->strides);
    t->magic = 0;
    t->data = NULL;
    t->dimensions = NULL;
    t->strides = NULL;
    t->order = 0;
}


bool
numc_vector_is_valid(const numc_vector_t *v)
{
    return v != NULL && v->magic == NUMC_VECTOR_MAGIC && v->data != NULL;
}


bool
numc_matrix_is_valid(const numc_matrix_t *m)
{
    return m != NULL && m->magic == NUMC_MATRIX_MAGIC && m->data != NULL;
}


bool
numc_tensor_is_valid(const numc_tensor_t *t)
{
    return t != NULL && t->magic == NUMC_TENSOR_MAGIC && t->data != NULL;
}

/* ---- element access ------------------------------------------------- */

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

/* ---- addition / subtraction ------------------------------------------- */

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
numc_tensor_add(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a) || !numc_tensor_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (!same_shape(a.order, a.dimensions, b.order, b.dimensions))
        return fail_dim_mismatch();

    numc_tensor_t r;
    numc_status_t st = alloc_tensor(a.order, a.dimensions, &r);
    if (st != NUMC_OK) return st;

    size_t total = product(a.order, a.dimensions);
    for (size_t i = 0; i < total; i++) r.data[i] = a.data[i] + b.data[i];
    *out = r;
    return NUMC_OK;
}


numc_status_t
numc_tensor_sub(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a) || !numc_tensor_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (!same_shape(a.order, a.dimensions, b.order, b.dimensions))
        return fail_dim_mismatch();

    numc_tensor_t r;
    numc_status_t st = alloc_tensor(a.order, a.dimensions, &r);
    if (st != NUMC_OK) return st;

    size_t total = product(a.order, a.dimensions);
    for (size_t i = 0; i < total; i++) r.data[i] = a.data[i] - b.data[i];
    *out = r;
    return NUMC_OK;
}

/* ---- scalar multiplication --------------------------------------------- */

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
numc_tensor_scale(numc_tensor_t a, double scalar, numc_tensor_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    numc_tensor_t r;
    numc_status_t st = alloc_tensor(a.order, a.dimensions, &r);
    if (st != NUMC_OK) return st;

    size_t total = product(a.order, a.dimensions);
    for (size_t i = 0; i < total; i++) r.data[i] = a.data[i] * scalar;
    *out = r;
    return NUMC_OK;
}

/* ---- products ----------------------------------------------------------- */

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
numc_tensor_contract(numc_tensor_t a, size_t axis_a, numc_tensor_t b, size_t axis_b, numc_tensor_t *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a) || !numc_tensor_is_valid(&b)) {
        return NUMC_ERR_INVALID_ARG;
    }
    if (axis_a >= a.order || axis_b >= b.order) return fail_out_of_bounds();
    if (a.dimensions[axis_a] != b.dimensions[axis_b]) return fail_dim_mismatch();

    size_t order_out = (a.order - 1) + (b.order - 1);
    size_t K = a.dimensions[axis_a];

    /* Degenerate case: both operands are order 1 (a plain dot product),
     * contracted fully -> a 0-order (scalar) tensor. */
    if (order_out == 0) {
        numc_tensor_t r;
        r.magic = NUMC_TENSOR_MAGIC;
        r.order = 0;
        r.dimensions = NULL;
        r.strides = NULL;
        r.data = malloc(sizeof(double));
        if (r.data == NULL) return NUMC_ERR_ALLOC;
        double sum = 0.0;
        for (size_t k = 0; k < K; k++) sum += a.data[k] * b.data[k];
        r.data[0] = sum;
        *out = r;
        return NUMC_OK;
    }

    size_t *dims_out = malloc(order_out * sizeof(size_t));
    if (dims_out == NULL) return NUMC_ERR_ALLOC;

    size_t d = 0;
    for (size_t i = 0; i < a.order; i++)
        if (i != axis_a) dims_out[d++] = a.dimensions[i];
    for (size_t i = 0; i < b.order; i++)
        if (i != axis_b) dims_out[d++] = b.dimensions[i];

    numc_tensor_t r;
    numc_status_t st = alloc_tensor(order_out, dims_out, &r);
    free(dims_out);
    if (st != NUMC_OK) return st;

    size_t total_out = product(order_out, r.dimensions);
    size_t free_a = a.order - 1;

    size_t *a_idx = malloc((a.order ? a.order : 1) * sizeof(size_t));
    size_t *b_idx = malloc((b.order ? b.order : 1) * sizeof(size_t));
    size_t *idx_out = malloc(order_out * sizeof(size_t));
    if (a_idx == NULL || b_idx == NULL || idx_out == NULL) {
        free(a_idx); free(b_idx); free(idx_out);
        numc_tensor_destroy(&r);
        return NUMC_ERR_ALLOC;
    }

    for (size_t out_flat = 0; out_flat < total_out; out_flat++) {
        /* decompose out_flat into a multi-index using out's strides */
        size_t rem = out_flat;
        for (size_t k = 0; k < order_out; k++) {
            idx_out[k] = rem / r.strides[k];
            rem %= r.strides[k];
        }

        size_t ai = 0;
        for (size_t k = 0; k < a.order; k++)
            if (k != axis_a) a_idx[k] = idx_out[ai++];
        size_t bi = 0;
        for (size_t k = 0; k < b.order; k++)
            if (k != axis_b) b_idx[k] = idx_out[free_a + bi++];

        double sum = 0.0;
        for (size_t k = 0; k < K; k++) {
            a_idx[axis_a] = k;
            b_idx[axis_b] = k;

            size_t a_off = 0;
            for (size_t t = 0; t < a.order; t++) a_off += a_idx[t] * a.strides[t];
            size_t b_off = 0;
            for (size_t t = 0; t < b.order; t++) b_off += b_idx[t] * b.strides[t];

            sum += a.data[a_off] * b.data[b_off];
        }
        r.data[out_flat] = sum;
    }

    free(a_idx);
    free(b_idx);
    free(idx_out);
    *out = r;
    return NUMC_OK;
}

/* ---- norms ---------------------------------------------------------- */

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
numc_tensor_norm(numc_tensor_t a, double *out)
{
    if (out == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a)) return NUMC_ERR_INVALID_ARG;
    size_t total = product(a.order, a.dimensions);
    double sum = 0.0;
    for (size_t i = 0; i < total; i++) sum += a.data[i] * a.data[i];
    *out = sqrt(sum);
    return NUMC_OK;
}

/* ---- transpose -------------------------------------------------------- */

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


numc_status_t
numc_tensor_transpose(numc_tensor_t a, const size_t *permutation, numc_tensor_t *out)
{
    if (out == NULL || permutation == NULL) return NUMC_ERR_INVALID_ARG;
    if (!numc_tensor_is_valid(&a)) return NUMC_ERR_INVALID_ARG;

    /* Validate that permutation is a genuine permutation of [0, order). */
    bool *seen = calloc(a.order ? a.order : 1, sizeof(bool));
    if (seen == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.order; i++) {
        if (permutation[i] >= a.order || seen[permutation[i]]) {
            free(seen);
            return NUMC_ERR_INVALID_ARG;
        }
        seen[permutation[i]] = true;
    }
    free(seen);

    size_t *dims_out = calloc((a.order ? a.order : 1), sizeof(size_t));
    if (dims_out == NULL) return NUMC_ERR_ALLOC;
    for (size_t i = 0; i < a.order; i++) dims_out[i] = a.dimensions[permutation[i]];

    numc_tensor_t r;
    numc_status_t st = alloc_tensor(a.order, dims_out, &r);
    free(dims_out);
    if (st != NUMC_OK) return st;

    size_t total = product(a.order, a.dimensions);
    size_t *idx_a = malloc((a.order ? a.order : 1) * sizeof(size_t));
    if (idx_a == NULL) { numc_tensor_destroy(&r); return NUMC_ERR_ALLOC; }

    for (size_t flat = 0; flat < total; flat++) {
        size_t rem = flat;
        for (size_t k = 0; k < a.order; k++) {
            idx_a[k] = rem / a.strides[k];
            rem %= a.strides[k];
        }
        size_t out_off = 0;
        for (size_t k = 0; k < a.order; k++)
            out_off += idx_a[permutation[k]] * r.strides[k];
        r.data[out_off] = a.data[flat];
    }
    free(idx_a);
    *out = r;
    return NUMC_OK;
}
