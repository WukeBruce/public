#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct bitmap {
	uint8_t *data;
	int64_t max_extent;
} bitmap_t;

#ifdef __cplusplus
extern "C" {
#endif


void bitmap_init(bitmap_t *xb, uint8_t *storage, int64_t max_extent);

/* Sets a bit at index idx. */
void bitmap_set(bitmap_t *xb, unsigned idx);

/* Clears a bit at index idx. */
void bitmap_clear(bitmap_t *xb, unsigned idx);

/* Predicate: returns nonzero if the bit at index idx is set. */
int bitmap_isset(bitmap_t *xb, unsigned idx);

/* Predicate: returns nonzero if the bit at index idx is clear. */
int bitmap_isclear(bitmap_t *xb, unsigned idx);

/* Return the index of the first bit that is set, or -1 if no bits are set at all. */
int64_t bitmap_first_set(bitmap_t *xb);

#ifdef __cplusplus
}
#endif

#endif
