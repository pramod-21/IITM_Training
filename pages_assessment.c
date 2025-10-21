#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 65536   // 64 KB
#define PAGE_SIZE 4096      // 4 KB
#define NUM_PAGES (MEMORY_SIZE / PAGE_SIZE)

typedef struct {
    int used;       // 0 = free, 1 = used
    int block_size; // in bytes (only for starting page of a block)
} Page;

Page page_table[NUM_PAGES];
unsigned char memory[MEMORY_SIZE]; // simulated memory pool

//Function to print page table
void print_page_table() {
    printf("\nPage Table:\n");
    printf("Page#  Status  BlockSize\n");
    printf("------------------------\n");
    for (int i = 0; i < NUM_PAGES; i++) {
        printf("%4d    %s    %6d\n", i, page_table[i].used ? "Used" : "Free", page_table[i].block_size);
    }
    printf("------------------------\n");
}

//Allocate memory
void* my_malloc(int size) {
    int pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int start = -1, count = 0;

    // First-fit allocation
    for (int i = 0; i < NUM_PAGES; i++) {
        if (!page_table[i].used) {
            if (start == -1) start = i;
            count++;
            if (count == pages_needed) break;
        } else {
            start = -1;
            count = 0;
        }
    }

    if (count != pages_needed) {
        printf("Allocation failed: not enough free pages for %d bytes\n", size);
        return NULL;
    }

    // Mark pages as used
    for (int i = start; i < start + pages_needed; i++) {
        page_table[i].used = 1;
        page_table[i].block_size = (i == start) ? size : 0; // only first page stores block size
    }

    printf("Allocated %d bytes starting at page %d\n", size, start);
    return &memory[start * PAGE_SIZE];
}

//Free memory
void my_free(void* ptr) {
    if (!ptr) return;

    int offset = (unsigned char*)ptr - memory;
    if (offset < 0 || offset >= MEMORY_SIZE) {
        printf("Pointer not within memory pool!\n");
        return;
    }

    int page_num = offset / PAGE_SIZE;
    if (!page_table[page_num].used || page_table[page_num].block_size == 0) {
        printf("Pointer does not point to start of a valid block!\n");
        return;
    }

    int size = page_table[page_num].block_size;
    int pages_to_free = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (int i = page_num; i < page_num + pages_to_free; i++) {
        page_table[i].used = 0;
        page_table[i].block_size = 0;
    }

    printf("Freed block of %d bytes starting at page %d\n", size, page_num);
}


int main() {
    memset(page_table, 0, sizeof(page_table));

    void *p1 = my_malloc(5000);    // ~2 pages
    print_page_table();

    void *p2 = my_malloc(8000);    // ~2 pages
    print_page_table();

    my_free(p1);
    print_page_table();

    void *p3 = my_malloc(16000);   // 4 pages
    print_page_table();

    my_free(p2);
    print_page_table();

    my_free(p3);
    print_page_table();

    return 0;
}

