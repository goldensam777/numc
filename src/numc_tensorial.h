#ifndef NUMC_TENSORIAL_H
#define NUMC_TENSORIAL_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "numc_status.h"

/*
 * numc_tensor_t follows the same ownership convention as every numc
 * container: numc_tensor_from() COPIES the data given by the caller into
 * a freshly allocated buffer. The caller keeps ownership of what they
 * passed in and must call numc_tensor_destroy() on the result when done.
 *
 * The object embeds a `magic` tag stamped by numc_tensor_from() and
 * cleared by numc_tensor_destroy(). numc_tensor_is_valid() requires the
 * tag to match, so a zeroed, uninitialised or destroyed object is
 * reported as invalid instead of being dereferenced blindly.
 */

typedef struct numc_tensor_t {
    uint32_t magic; /* tag set by numc_tensor_from(), cleared by destroy */
    size_t order;
    size_t *dimensions; /* length = order, owned copy */
    size_t *strides;    /* length = order, computed at creation */
    double *data;       /* length = product(dimensions) */
} numc_tensor_t;

/* Construction (copies data). Fails on alloc failure, order == 0 or any
 * zero dimension (a zero dimension would make all strides zero and break
 * the index decomposition of transpose/contract). */
numc_status_t numc_tensor_from(size_t order, const size_t *dimensions, const double *data, numc_tensor_t *out);

/* Destruction. Safe on already-destroyed / zeroed objects. */
void numc_tensor_destroy(numc_tensor_t *t);

/* Validity check: tag stamped by from(), cleared by destroy() */
bool numc_tensor_is_valid(const numc_tensor_t *t);

/* Addition / subtraction. NUMC_ERR_DIM_MISMATCH on shape mismatch. */
numc_status_t numc_tensor_add(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out);
numc_status_t numc_tensor_sub(numc_tensor_t a, numc_tensor_t b, numc_tensor_t *out);

/* Scalar multiplication. Total: only fails on result alloc. */
numc_status_t numc_tensor_scale(numc_tensor_t a, double scalar, numc_tensor_t *out);

/* Contraction along one axis of each operand.
 * Requires dimensions[axis_a] == dimensions[axis_b]. Contracting two
 * order-1 tensors produces an order-0 (scalar) tensor. */
numc_status_t numc_tensor_contract(numc_tensor_t a, size_t axis_a,
                                    numc_tensor_t b, size_t axis_b,
                                    numc_tensor_t *out);

/* Frobenius norm */
numc_status_t numc_tensor_norm(numc_tensor_t a, double *out);

/* Transpose by permutation: permutation must have length a.order and be
 * a genuine permutation of [0, order). */
numc_status_t numc_tensor_transpose(numc_tensor_t a, const size_t *permutation, numc_tensor_t *out);

#endif /* NUMC_TENSORIAL_H */
