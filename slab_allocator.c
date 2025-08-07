#include "slab_allocator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SLAB_SIZE 4096

static slab_t* slab_create(size_t object_size) {
    slab_t *slab = malloc(sizeof(slab_t));
    if (!slab) return NULL;

    slab->memory = aligned_alloc(4096, SLAB_SIZE);
    if (!slab->memory) {
        free(slab);
        return NULL;
    }

    slab->object_size = object_size;
    slab->objects_per_slab = SLAB_SIZE / object_size;
    slab->free_objects = slab->objects_per_slab;
    slab->next = NULL;
    slab->free_list = NULL;

    char *ptr = (char*)slab->memory;
    for (size_t i = 0; i < slab->objects_per_slab; ++i) {
        object_header_t *header = (object_header_t*)ptr;
        header->next_free = slab->free_list;
        slab->free_list = header;
        ptr += object_size;
    }

    return slab;
}

slab_cache_t* slab_cache_create(const char *name, size_t object_size) {
    slab_cache_t *cache = malloc(sizeof(slab_cache_t));
    if (!cache) return NULL;

    cache->slabs = NULL;
    cache->object_size = object_size;
    strncpy(cache->name, name, sizeof(cache->name) - 1);
    cache->name[sizeof(cache->name) - 1] = '\0';

    return cache;
}

void* slab_alloc(slab_cache_t *cache) {
    slab_t *slab = cache->slabs;

    while (slab && slab->free_objects == 0)
        slab = slab->next;

    if (!slab) {
        slab = slab_create(cache->object_size);
        if (!slab) return NULL;
        slab->next = cache->slabs;
        cache->slabs = slab;
    }

    object_header_t *obj = slab->free_list;
    slab->free_list = obj->next_free;
    slab->free_objects--;

    return obj;
}

void slab_free(slab_cache_t *cache, void *obj) {
    slab_t *slab = cache->slabs;

    while (slab) {
        char *start = (char*)slab->memory;
        char *end = start + SLAB_SIZE;

        if ((char*)obj >= start && (char*)obj < end) {
            object_header_t *header = (object_header_t*)obj;
            header->next_free = slab->free_list;
            slab->free_list = header;
            slab->free_objects++;
            return;
        }

        slab = slab->next;
    }
}

void slab_cache_destroy(slab_cache_t *cache) {
    slab_t *slab = cache->slabs;
    while (slab) {
        slab_t *next = slab->next;
        free(slab->memory);
        free(slab);
        slab = next;
    }
    free(cache);
}
