#include "slab_allocator.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    char data[128];
} db_tuple_t;

void run_dbms_slab_test() {
    slab_cache_t *tuple_cache = slab_cache_create("db_tuple_cache", sizeof(db_tuple_t));

    db_tuple_t *tuples[1000];

    for (int i = 0; i < 1000; i++) {
        tuples[i] = (db_tuple_t*)slab_alloc(tuple_cache);
        if (tuples[i]) {
            tuples[i]->id = i;
            snprintf(tuples[i]->data, sizeof(tuples[i]->data), "Tuple %d", i);
        }
    }

    printf("Allocated 1000 tuples.\n");

    for (int i = 0; i < 500; i++) {
        slab_free(tuple_cache, tuples[i]);
    }

    printf("Freed 500 tuples.\n");

    slab_cache_destroy(tuple_cache);
}
