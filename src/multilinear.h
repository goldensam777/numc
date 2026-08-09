#ifndef NUMC_MULTILINEAR_H
#define NUMC_MULTILINEAR_H

#include <stddef.h>
#include <stdbool.h>
#include "numc_status.h"

/*
 * Vector / Matrix / Tensor share the same ownership convention:
 * numc_*_from() COPIES the data given by the caller into a freshly
 * allocated buffer. The caller keeps ownership of what they passed in
 * and must call numc_*_destroy() on the returned object when done.
 */

typedef struct Vector {
    size_t size;
    double *data;
} Vector;

typedef struct Matrix {
    size_t rows;
    size_t cols;
    double *data;
} Matrix;

typedef struct Tensor {
    size_t order;
    size_t *dimensions; /* length = order, owned copy */
    size_t *strides;    /* length = order, computed at creation */
    double *data;       /* length = product(dimensions) */
} Tensor;

/* Construction (copies data). Fails on alloc failure or order/size == 0. */
numc_status_t numc_vector_from(size_t size, const double *data, Vector *out);
numc_status_t numc_matrix_from(size_t rows, size_t cols, const double *data, Matrix *out);
numc_status_t numc_tensor_from(size_t order, const size_t *dimensions, const double *data, Tensor *out);

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_vector_destroy(Vector *v);
void numc_matrix_destroy(Matrix *m);
void numc_tensor_destroy(Tensor *t);

/* Validity checks (data != NULL) */
bool numc_vector_is_valid(const Vector *v);
bool numc_matrix_is_valid(const Matrix *m);
bool numc_tensor_is_valid(const Tensor *t);

/* Element access, bounds-checked */
numc_status_t numc_vector_get(const Vector *v, size_t i, double *out);
numc_status_t numc_vector_set(Vector *v, size_t i, double val);
numc_status_t numc_matrix_get(const Matrix *m, size_t i, size_t j, double *out);
numc_status_t numc_matrix_set(Matrix *m, size_t i, size_t j, double val);

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on shape mismatch. */
numc_status_t numc_vector_add(Vector a, Vector b, Vector *out);
numc_status_t numc_matrix_add(Matrix a, Matrix b, Matrix *out);
numc_status_t numc_tensor_add(Tensor a, Tensor b, Tensor *out);

numc_status_t numc_vector_sub(Vector a, Vector b, Vector *out);
numc_status_t numc_matrix_sub(Matrix a, Matrix b, Matrix *out);
numc_status_t numc_tensor_sub(Tensor a, Tensor b, Tensor *out);

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_vector_scale(Vector a, double scalar, Vector *out);
numc_status_t numc_matrix_scale(Matrix a, double scalar, Matrix *out);
numc_status_t numc_tensor_scale(Tensor a, double scalar, Tensor *out);

/* Vector dot product (true scalar product -> double, total on matching size) */
numc_status_t numc_vector_dot(Vector a, Vector b, double *out);

/* Matrix product (NOT a scalar product -- named mul, not dot, to avoid
 * confusion with numc_vector_dot). Fails if a.cols != b.rows. */
numc_status_t numc_matrix_mul(Matrix a, Matrix b, Matrix *out);

/* Tensor contraction along one axis of each operand.
 * Requires dimensions[axis_a] == dimensions[axis_b]. */
numc_status_t numc_tensor_contract(Tensor a, size_t axis_a,
                                    Tensor b, size_t axis_b,
                                    Tensor *out);

/* Norms (Euclidean for Vector, Frobenius for Matrix/Tensor) */
numc_status_t numc_vector_norm(Vector a, double *out);
numc_status_t numc_matrix_norm(Matrix a, double *out);
numc_status_t numc_tensor_norm(Tensor a, double *out);

/* Transpose */
numc_status_t numc_matrix_transpose(Matrix a, Matrix *out);
/* permutation must have length a.order and be a valid permutation of [0, order) */
numc_status_t numc_tensor_transpose(Tensor a, const size_t *permutation, Tensor *out);

#endif /* NUMC_MULTILINEAR_H */