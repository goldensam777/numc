#ifndef NUMC_VECTOR_H
#define NUMC_VECTOR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "numc_status.h"

/*
 * numc_vector_t follows the same ownership convention as every numc
 * container: numc_vector_from() COPIES the data given by the caller into
 * a freshly allocated buffer. The caller keeps ownership of what they
 * passed in and must call numc_vector_destroy() on the result when done.
 *
 * The object embeds a `magic` tag stamped by numc_vector_from() and
 * cleared by numc_vector_destroy(). numc_vector_is_valid() requires the
 * tag to match, so a zeroed, uninitialised or destroyed object is
 * reported as invalid instead of being dereferenced blindly.
 */

typedef struct numc_vector_t {
    uint32_t magic; /* tag set by numc_vector_from(), cleared by destroy */
    size_t size;
    double *data;
} numc_vector_t;

/* Construction (copies data). Fails on alloc failure or size == 0. */
numc_status_t numc_vector_from(size_t size, const double *data, numc_vector_t *out);

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_vector_destroy(numc_vector_t *v);

/* Validity check: tag stamped by from(), cleared by destroy() */
bool numc_vector_is_valid(const numc_vector_t *v);

/* Element access, bounds-checked */
numc_status_t numc_vector_get(const numc_vector_t *v, size_t i, double *out);
numc_status_t numc_vector_set(numc_vector_t *v, size_t i, double val);

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on size mismatch. */
numc_status_t numc_vector_add(numc_vector_t a, numc_vector_t b, numc_vector_t *out);
numc_status_t numc_vector_sub(numc_vector_t a, numc_vector_t b, numc_vector_t *out);

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_vector_scale(numc_vector_t a, double scalar, numc_vector_t *out);

/* Dot product (true scalar product -> double, total on matching size) */
numc_status_t numc_vector_dot(numc_vector_t a, numc_vector_t b, double *out);

/* Euclidean norm */
numc_status_t numc_vector_norm(numc_vector_t a, double *out);

#endif /* NUMC_VECTOR_H */
