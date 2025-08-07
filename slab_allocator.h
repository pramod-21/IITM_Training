#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include <stddef.h>
#include<stdlib.h>
#include <string.h>

typedef struct object_header {
    struct object_header *next_free;
} object_header_t;

typedef struct slab {
    void *memory;
    object_header_t *free_list;
    size_t object_size;
    size_t objects_per_slab;
    size_t free_objects;
    struct slab *next;
} slab_t;

typedef struct slab_cache {
    slab_t *slabs;
    size_t object_size;
    char name[64];
} slab_cache_t;

slab_cache_t* slab_cache_create(const char *name, size_t object_size);
void* slab_alloc(slab_cache_t *cache);
void slab_free(slab_cache_t *cache, void *obj);
void slab_cache_destroy(slab_cache_t *cache);

#endif // SLAB_ALLOCATOR_H
