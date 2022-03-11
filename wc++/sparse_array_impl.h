/* Copyright (c) 2018, Robert Harris. */

#ifndef SPARSE_ARRAY_IMPL_H
#define	SPARSE_ARRAY_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/param.h>
#include <wchar.h>
#include <sparse_array.h>

/*
 * The sparse array is implemented as a tree in which each node is a simple
 * table of pointers.  The bits of the index are divided into equally sized keys
 * where each is used as the offset into the corresponding node;  the leaf nodes
 * form the underlying storage returned to the caller.
 *
 * The index width and the key size are both tuneable at compilation.  Thus,
 * with a 32-bit index and an 8-bit key, the effective address of
 *
 *	element[0xdeadbeef]
 *
 * is
 *
 *	&root->branch[0xde]->branch[0xad]->branch[0xbe]->branch[0xef]
 *
 * The choice of key size is determined chiefly by the sparseness of the keys.
 * A conventional array is approximated by a large key size, with a small tree
 * depth;  this is suited to large numbers of consecutive indices.  Conversely,
 * sparse indices are suited by smaller keys, at the cost of an increased tree
 * depth and consequent access time.
 *
 * The current application deals with the distribution of letters.  16 is a more
 * plausible estimate of an alphabet size than 256, the next highest value, but
 * we choose the latter anyway in order to favour speed (by halving the number
 * of levels) over memory.  Furthermore, knowing that the sparse array is to be
 * indexed by wchar_t allows its index type to be defined, in sparse_array.h,
 * as the same-sized unsigned int.
 */
#define	SA_KEY_BITS_SHIFT	3	/* key size is 2^3 */

#define	SA_KEY_BITS		(1U << SA_KEY_BITS_SHIFT)
#define	SA_KEYS			(1U << SA_KEY_BITS)
#define	SA_INDEX_BITS		(sizeof (sparse_index_t) * NBBY)
#define	SA_LEVELS		(SA_INDEX_BITS / SA_KEY_BITS)
#define	SA_KEY_MASK_SHIFT	(SA_KEY_BITS * (SA_LEVELS - 1))
#define	SA_KEY_MASK		((SA_KEYS - 1) << SA_KEY_MASK_SHIFT)

struct sparse_array {
	struct sparse_array *sa_branches[SA_KEYS];
};

#ifdef __cplusplus
}
#endif

#endif /* SPARSE_ARRAY_IMPL_H */
