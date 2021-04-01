#ifndef BUDDY_H
#define BUDDY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitmap.h"

/* log2 of the maximum buddy node size. */
#define MAX_BUDDY_SZ_LOG2 12 /* 2^12 = 4KB */
/* log2 of the minimum buddy node size. */
#define MIN_BUDDY_SZ_LOG2 6 /* 2^6 = 64Byte */

/* 注：以上参数的位图表所需大小:
 * 4MB => 16263 Byte
 * 2MB => 8135 Byte
 * 1MB => 4071 Byte
 */

#define NUM_BUDDY_BUCKETS (MAX_BUDDY_SZ_LOG2 - MIN_BUDDY_SZ_LOG2 + 1)

typedef struct range {
	uint64_t start;
	uint64_t extent;
} range_t;

typedef struct buddy {
	uint64_t start, size;
	bitmap_t orders[NUM_BUDDY_BUCKETS];
} buddy_t;

#ifdef __cplusplus
extern "C" {
#endif

size_t buddy_calc_overhead(range_t r);
int buddy_init       (buddy_t *bd, uint8_t *overhead_storage, range_t r, int start_freed);
uint64_t buddy_alloc (buddy_t *bd, unsigned sz);
void buddy_free      (buddy_t *bd, uint64_t addr, unsigned sz);
void buddy_free_range(buddy_t *bd, range_t range);

extern buddy_t kernel_buddy;

#ifdef __cplusplus
}
#endif

#endif
