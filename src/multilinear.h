#ifndef NUMC_MULTILINEAR_H
#define NUMC_MULTILINEAR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "numc_status.h"

/*
 * numc_vector_t / numc_matrix_t / numc_tensor_t share the same ownership convention:
 * numc_*_from() COPIES the data given by the caller into a freshly
 * allocated buffer. The caller keeps ownership of what they passed in
 * and must call numc_*_destroy() on the returned object when done.
 *
 * Each object embeds a `magic` tag stamped by numc_*_from() and cleared
 * by numc_*_destroy(). numc_*_is_valid() requires the tag to match, so a
 * zeroed, uninitialised or destroyed object is reported as invalid instead
 * of being dereferenced blindly.
 */

typedef struct numc_vector_t {
    uint32_t magic; /* tag set by numc_vector_from(), cleared by destroy */
    size_t size;
    double *data;
} numc_vector_t;

typedef struct numc_matrix_t {
    uint32_t magic; /* tag set by numc_matrix_from(), cleared by destroy */
    size_t rows;
    size_t cols;
    double *data;
} numc_matrix_t;

typedef struct numc_tensor_t {
    uint32_t magic; /* tag set by numc_tensor_from(), cleared by destroy */
    size_t order;
    size_t *dimensions; /* length = order, owned copy */
    size_t *strides;    /* length = order, computed at creation */
    double *data;       /* length = product(dimensions) */
} numc_tensor_t;

/* Construction (copies data). Fails on alloc failure or order/size == 0. */
numc_status_t numc_vector_from(size_t size, const double *data, numc_vector_t *out);
numc_status_t numc_matrix_from(size_t rows, size_t cols, const double *data, numc_matrix_t *out);
numc_status_t numc_tensor_from(size_t order, const size_t *dimensions, const double *data, numc_tensor_t *out);

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_vector_destroy(numc_vector_t *v);
void numc_matrix_destroy(numc_matrix_t *m);
void numc_tensor_destroy(numc_tensor_t *t);

/* Validity checks: tag stamped by numc_*_from(), cleared by numc_*_destroy() */
bool numc_vector_is_valid(const numc_vector_t *v);
bool numc_matrix_is_valid(const numc_matrix_t *m);
bool numc_tensor_is_valid(const numc_tensor_t *t);

/* Element access, bounds-checked */
numc_status_t numc_vector_get(const numc_vector_t *v, size_t i, double *out);
numc_status_t numc_vector_set(numc_vector_t *v, size_t i, double val);
numc_status_t numc_matrix_get(const numc_matrix_t *m, size_t i, size_t j, double *out);
numc_status_t numc_matrix_set(numc_matrix_t *m, size_t i, size_t j, double val);

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on shape mismatch. */
numc_status_t numc_vector_add(numc_vector_t a, numc_vector_t b, numc_vector_t *out);
numc_status_t numc_matrix_add(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);
numc_status_t numc_tensor_add(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out);

numc_status_t numc_vector_sub(numc_vector_t a, numc_vector_t b, numc_vector_t *out);
numc_status_t numc_matrix_sub(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);
numc_status_t numc_tensor_sub(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out);

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_vector_scale(numc_vector_t a, double scalar, numc_vector_t *out);
numc_status_t numc_matrix_scale(numc_matrix_t a, double scalar, numc_matrix_t *out);
numc_status_t numc_tensor_scale(numc_tensor_t a, double scalar, numc_tensor_t *out);

/* numc_vector_t dot product (true scalar product -> double, total on matching size) */
numc_status_t numc_vector_dot(numc_vector_t a, numc_vector_t b, double *out);

/* numc_matrix_t product (NOT a scalar product -- named mul, not dot, to avoid
 * confusion with numc_vector_dot). Fails if a.cols != b.rows. */
numc_status_t numc_matrix_mul(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);

/* numc_tensor_t contraction along one axis of each operand.
 * Requires dimensions[axis_a] == dimensions[axis_b]. */
numc_status_t numc_tensor_contract(numc_tensor_t a, size_t axis_a,
                                    numc_tensor_t b, size_t axis_b,
                                    numc_tensor_t *out);

/* Norms (Euclidean for numc_vector_t, Frobenius for numc_matrix_t/numc_tensor_t) */
numc_status_t numc_vector_norm(numc_vector_t a, double *out);
numc_status_t numc_matrix_norm(numc_matrix_t a, double *out);
numc_status_t numc_tensor_norm(numc_tensor_t a, double *out);

/* Transpose */
numc_status_t numc_matrix_transpose(numc_matrix_t a, numc_matrix_t *out);
/* permutation must have length a.order and be a valid permutation of [0, order) */
numc_status_t numc_tensor_transpose(numc_tensor_t a, const size_t *permutation, numc_tensor_t *out);

#endif /* NUMC_MULTILINEAR_H */
