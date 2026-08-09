#include "multilinear.h"

/* Construction (copies data). Fails on alloc failure or order/size == 0. */
numc_status_t
numc_vector_from(size_t size, const double *data, Vector *out){
    Vector v;
    if (size == 0 || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    v.size = size;
    v.data = malloc(size * sizeof(double));
    if (v.data == NULL) {
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < size; i++) {
        v.data[i] = data[i];
    }
    *out = v;
    return NUMC_OK;
}


numc_status_t 
numc_matrix_from(size_t rows, size_t cols, const double *data, Matrix *out){
    Matrix m;
    if (rows == 0 || cols == 0 || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    m.rows = rows;
    m.cols = cols;
    m.data = malloc(rows * cols * sizeof(double));
    if (m.data == NULL) {
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            m.data[i * cols + j] = data[i * cols + j];
        }
    }
    *out = m;
    return NUMC_OK;
}


numc_status_t 
numc_tensor_from(size_t order, const size_t *dimensions, const double *data, Tensor *out){
    Tensor t;
    if (order == 0 || dimensions == NULL || data == NULL || out == NULL) {
        return NUMC_ERR_INVALID_ARG;
    }
    t.order = order;
    t.dimensions = malloc(order * sizeof(size_t));
    t.strides = malloc(order * sizeof(size_t));
    if (t.dimensions == NULL || t.strides == NULL) {
        free(t.dimensions);
        free(t.strides);
        return NUMC_ERR_ALLOC;
    }
    size_t total_size = 1;
    for (size_t i = 0; i < order; i++) {
        t.dimensions[i] = dimensions[i];
        total_size *= dimensions[i];
    }
    t.data = malloc(total_size * sizeof(double));
    if (t.data == NULL) {
        free(t.dimensions);
        free(t.strides);
        return NUMC_ERR_ALLOC;
    }
    for (size_t i = 0; i < total_size; i++) {
        t.data[i] = data[i];
    }
    // Compute strides
    t.strides[order - 1] = 1;
    for (size_t i = order - 1; i > 0; i--) {
        t.strides[i - 1] = t.strides[i] * t.dimensions[i];
    }
    *out = t;
    return NUMC_OK;
}

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_vector_destroy(Vector *v)
void numc_matrix_destroy(Matrix *m)
void numc_tensor_destroy(Tensor *t)

/* Validity checks (data != NULL) */
bool numc_vector_is_valid(const Vector *v)
bool numc_matrix_is_valid(const Matrix *m)
bool numc_tensor_is_valid(const Tensor *t)

/* Element access, bounds-checked */
numc_status_t numc_vector_get(const Vector *v, size_t i, double *out)
numc_status_t numc_vector_set(Vector *v, size_t i, double val)
numc_status_t numc_matrix_get(const Matrix *m, size_t i, size_t j, double *out)
numc_status_t numc_matrix_set(Matrix *m, size_t i, size_t j, double val)

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on shape mismatch. */
numc_status_t numc_vector_add(Vector a, Vector b, Vector *out)
numc_status_t numc_matrix_add(Matrix a, Matrix b, Matrix *out)
numc_status_t numc_tensor_add(Tensor a, Tensor b, Tensor *out)

numc_status_t numc_vector_sub(Vector a, Vector b, Vector *out)
numc_status_t numc_matrix_sub(Matrix a, Matrix b, Matrix *out)
numc_status_t numc_tensor_sub(Tensor a, Tensor b, Tensor *out)

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_vector_scale(Vector a, double scalar, Vector *out)
numc_status_t numc_matrix_scale(Matrix a, double scalar, Matrix *out)
numc_status_t numc_tensor_scale(Tensor a, double scalar, Tensor *out)

/* Vector dot product (true scalar product -> double, total on matching size) */
numc_status_t numc_vector_dot(Vector a, Vector b, double *out)

/* Matrix product (NOT a scalar product -- named mul, not dot, to avoid
 * confusion with numc_vector_dot). Fails if a.cols != b.rows. */
numc_status_t numc_matrix_mul(Matrix a, Matrix b, Matrix *out)

/* Tensor contraction along one axis of each operand.
 * Requires dimensions[axis_a] == dimensions[axis_b]. */
numc_status_t numc_tensor_contract(Tensor a, size_t axis_a,
                                    Tensor b, size_t axis_b,
                                    Tensor *out)

/* Norms (Euclidean for Vector, Frobenius for Matrix/Tensor) */
numc_status_t numc_vector_norm(Vector a, double *out)
numc_status_t numc_matrix_norm(Matrix a, double *out)
numc_status_t numc_tensor_norm(Tensor a, double *out)

/* Transpose */
numc_status_t numc_matrix_transpose(Matrix a, Matrix *out)
/* permutation must have length a.order and be a valid permutation of [0, order) */
numc_status_t numc_tensor_transpose(Tensor a, const size_t *permutation, Tensor *out)