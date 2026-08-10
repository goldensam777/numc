#ifndef NUMC_BILINEAR_H
#define NUMC_BILINEAR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "numc_status.h"
#include "numc_vector.h"

/*
 * numc_matrix_t follows the same ownership convention as every numc
 * container: numc_matrix_from() COPIES the data given by the caller into
 * a freshly allocated buffer. The caller keeps ownership of what they
 * passed in and must call numc_matrix_destroy() on the result when done.
 *
 * The object embeds a `magic` tag stamped by numc_matrix_from() and
 * cleared by numc_matrix_destroy(). numc_matrix_is_valid() requires the
 * tag to match, so a zeroed, uninitialised or destroyed object is
 * reported as invalid instead of being dereferenced blindly.
 *
 * Matrices are the seeds of the bilinear algebra: bilinear, quadratic
 * and Hermitian forms will build on this type.
 */

typedef struct numc_matrix_t {
    uint32_t magic; /* tag set by numc_matrix_from(), cleared by destroy */
    size_t rows;
    size_t cols;
    double *data; /* row-major, length = rows * cols */
} numc_matrix_t;

typedef struct numc_linear_map_t {
    uint32_t magic; /* tag set by numc_linear_map_from(), cleared by destroy */
    numc_matrix_t matrix;
    numc_vector_t u;
} numc_linear_map_t;

typedef struct numc_affine_map_t {
    uint32_t magic; /* tag set by numc_affine_map_from(), cleared by destroy */
    numc_matrix_t matrix;
    numc_vector_t u;
    numc_vector_t v;
} numc_affine_map_t;

/* Construction (copies data). Fails on alloc failure or rows/cols == 0. */
numc_status_t numc_matrix_from(size_t rows, size_t cols, const double *data, numc_matrix_t *out);

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_matrix_destroy(numc_matrix_t *m);

/* Validity check: tag stamped by from(), cleared by destroy() */
bool numc_matrix_is_valid(const numc_matrix_t *m);

/* Element access, bounds-checked */
numc_status_t numc_matrix_get(const numc_matrix_t *m, size_t i, size_t j, double *out);
numc_status_t numc_matrix_set(numc_matrix_t *m, size_t i, size_t j, double val);

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on shape mismatch. */
numc_status_t numc_matrix_add(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);
numc_status_t numc_matrix_sub(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_matrix_scale(numc_matrix_t a, double scalar, numc_matrix_t *out);

/* Matrix product (NOT a scalar product -- named mul, not dot, to avoid
 * confusion with numc_vector_dot). Fails if a.cols != b.rows. */
numc_status_t numc_matrix_mul(numc_matrix_t a, numc_matrix_t b, numc_matrix_t *out);

/* Matrix-vector multiplication: y = M * x (out length = M.rows). Fails if m.cols != v.size. */
numc_status_t numc_matrix_vec_mul(numc_matrix_t m, numc_vector_t v, numc_vector_t *out);

/* Frobenius norm */
numc_status_t numc_matrix_norm(numc_matrix_t a, double *out);

/* Transpose */
numc_status_t numc_matrix_transpose(numc_matrix_t a, numc_matrix_t *out);

/* Linear map: f(x) = M * x + u (copies matrix and u). Fails if matrix.rows != u.size. */
numc_status_t numc_linear_map_from(numc_matrix_t matrix, numc_vector_t u, numc_linear_map_t *out);
void numc_linear_map_destroy(numc_linear_map_t *map);
bool numc_linear_map_is_valid(const numc_linear_map_t *map);
numc_status_t numc_linear_map_apply(const numc_linear_map_t *map, numc_vector_t x, numc_vector_t *out);

/* Affine map: f(x) = M * (x - u) + v (copies matrix, u and v). Fails if u.size != matrix.cols or v.size != matrix.rows. */
numc_status_t numc_affine_map_from(numc_matrix_t matrix, numc_vector_t u, numc_vector_t v, numc_affine_map_t *out);
void numc_affine_map_destroy(numc_affine_map_t *map);
bool numc_affine_map_is_valid(const numc_affine_map_t *map);
numc_status_t numc_affine_map_apply(const numc_affine_map_t *map, numc_vector_t x, numc_vector_t *out);

#endif /* NUMC_BILINEAR_H */
