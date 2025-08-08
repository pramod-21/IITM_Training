#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdbool.h>
#include "./include/bufferpool.h"
#include "./include/lrucache.h"

// Cached File
typedef struct cached_file {
    char filename[256];
    buffer_t *buffer;
    size_t file_size;
    time_t last_accessed;
    struct cached_file *next;
}cached_file_t;

typedef struct file_cache {
    buffer_pool_t *buffer_pool;
    cached_file_t *files[CACHE_SIZE];
    int cached_files_count;
}file_cache_t;

//To create file cache
file_cache_t* create_file_cache() {
    file_cache_t *cache = malloc(sizeof(file_cache_t));
    if(!cache) return NULL;

    cache->buffer_pool = create_buffer_pool(POOL_SIZE);
    if(!cache->buffer_pool)
    {
        free(cache);
        return NULL;
    }  
    
    cache->cached_files_count = 0;

    for (int iterator = 0; iterator < CACHE_SIZE; iterator++)
    {
        cache->files[iterator] = NULL;
    }

    return cache;
} 

//To read the cached file
char* read_file_cached(file_cache_t *cache, const char *filename)
{
    unsigned int index = hash(filename);

    cached_file_t *cached = cache->files[index];

    while(cached)
    {
        if(strcmp(cached->filename, filename) == 0)
        {
            cached->last_accessed = time(NULL);
            printf("File cache HIT for %s \n", filename);
            return cached->buffer->data;

        }

        cached = cached->next;
    }

    printf("File cache MISS for %s \n", filename);

    buffer_t *buffer = acquire_buffer(cache->buffer_pool);
    if(!buffer)
    {
        printf("No buffers are available");
        return NULL;
    }

    int fd = open(filename, O_RDONLY);
    if(fd == -1)
    {
        release_buffer(cache->buffer_pool, buffer);
        return NULL;
    }

    size_t bytes_read = read(fd, buffer->data, BUFFER_SIZE - 1);
    close(fd);

    if(bytes_read == -1)
    {
        release_buffer(cache->buffer_pool, buffer);
        return NULL;
    }

    buffer->data[bytes_read] = '\0';


//To add file as a new cache entry
cached_file_t *new_cached = malloc(sizeof(cached_file_t));

if(!new_cached)
{
    release_buffer(cache->buffer_pool, buffer);
    return NULL;
}

strncpy(new_cached->filename, filename, sizeof(new_cached->filename) - 1);
new_cached->filename[sizeof(new_cached->filename) - 1] = '\0';

new_cached->buffer = buffer;
new_cached->file_size = bytes_read;
new_cached->last_accessed = time(NULL);
new_cached->next = cache->files[index];

cache->files[index] = new_cached;
cache->cached_files_count++;

printf("Cached file %s of size %zu bytes \n", filename, bytes_read);

return buffer->data;
}

//to remove the cached file
//LRU removal

