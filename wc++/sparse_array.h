/* Copyright (c) 2018, Robert Harris. */

#ifndef SPARSE_ARRAY_H
#define	SPARSE_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Interfaces for a sparse array of pointers.
 *
 * addr = sparse_get_addr(&sap, index)
 *     returns the effective address of the pointer-sized element[index] in the
 *     sparse array.  sap must be initialised to NULL on first use.
 *
 * sparse_iter(sap, f, arg)
 *    iterates over all non-zero entries in the sparse array and for each one
 *    calls f(index, &element[index], arg).
 *
 * sparse destroy(sap)
 *     destroys the storage associated with the sparse array.
 */
typedef unsigned int sparse_index_t;
typedef void (*sparse_iter_f)(sparse_index_t, void *, void *);

struct sparse_array;

void *sparse_get_addr(struct sparse_array **, sparse_index_t);
void sparse_iter(struct sparse_array *, sparse_iter_f, void *);
void sparse_destroy(struct sparse_array *);

#ifdef __cplusplus
}
#endif

#endif /* SPARSE_ARRAY_H */
