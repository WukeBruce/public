#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define GxCore_Malloc(x)   malloc(x)
//#define GxCore_Free(p)     free(p)
//#define GxCore_Strdup(p)   strdup(p)

#ifndef uint
#define uint unsigned int
#endif

struct map_vtable_struct;

typedef struct {
	struct map_vtable_struct const *vtable;
} map_t;

typedef struct {
	int    pos;
	int    key;
	uint   value;
	bool   end;
	void *priv;
} iterator_t;

typedef bool     (*map_erase_t)    (map_t *, int);
typedef void     (*map_set_t)      (map_t *, int, uint);
typedef uint     (*map_get_t)      (map_t *, int, bool *);
typedef void     (*map_free_t)     (map_t *);
typedef int      (*map_count_t)    (map_t *);
typedef iterator_t *(*map_first_t) (map_t *, iterator_t *);
typedef iterator_t *(*map_next_t)  (map_t *, iterator_t *);

typedef struct map_vtable_struct {
	map_set_t    set;
	map_get_t    get;
	map_free_t   free;
	map_first_t  first;
	map_next_t   next;
	map_erase_t  erase;
	map_count_t  count;
} map_vtable_t;

void map_set  (map_t *m, int key, uint value);
uint map_get  (map_t *m, int key, bool *found);
bool map_erase(map_t *m, int key);
int  map_count(map_t *m);
void map_free (map_t *m);

iterator_t *map_first(map_t *m, iterator_t *it);
iterator_t *map_next(map_t *m, iterator_t *it);

map_t *init_hash(void);
map_t *init_btree(int num);

#ifdef __cplusplus
}
#endif

#endif
