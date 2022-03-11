/* Copyright (c) 2018, Robert Harris. */

#include <stdlib.h>
#include <sparse_array_impl.h>

/*
 * See the associated header files for a description of these interfaces and
 * the data structures on which they operate.
 */

void *
sparse_get_addr(struct sparse_array **sapp, sparse_index_t index)
{
	int level;
	struct sparse_array *sap;
	sparse_index_t key;

	for (level = 0; level < SA_LEVELS; level++) {
		if ((sap = *sapp) == NULL &&
		    (sap = *sapp = calloc(1,
		    sizeof (struct sparse_array))) == NULL) {
			perror("failed to grow sparse array");
			exit(1);
		}
		key = (index & SA_KEY_MASK) >> SA_KEY_MASK_SHIFT;
		index <<= SA_KEY_BITS;
		sapp = &sap->sa_branches[key];
	}

	return (sapp);
}

static void
sparse_iter_r(struct sparse_array *sap, sparse_iter_f f, void *arg, int level,
    int index)
{
	sparse_index_t key;
	struct sparse_array *bp;

	index <<= SA_KEY_BITS;
	for (key = 0; key < SA_KEYS; key++) {
		if ((bp = sap->sa_branches[key]) == NULL)
			continue;
		if (level == SA_LEVELS - 1)
			f(index | key, &bp, arg);
		else
			sparse_iter_r(bp, f, arg, level + 1, index | key);
	}
}

void
sparse_iter(struct sparse_array *sap, sparse_iter_f f, void *arg)
{
	if (sap)
		sparse_iter_r(sap, f, arg, 0, 0);
}

static void
sparse_destroy_r(struct sparse_array *sap, int level)
{
	sparse_index_t key;
	struct sparse_array *bp;

	if (level < SA_LEVELS - 1) {
		for (key = 0; key < SA_KEYS; key++) {
			if ((bp = sap->sa_branches[key]) != NULL)
				sparse_destroy_r(bp, level + 1);
		}
	}
	free(sap);
}

void
sparse_destroy(struct sparse_array *sap)
{
	if (sap)
		sparse_destroy_r(sap, 0);
}
