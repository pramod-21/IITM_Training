#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_INTS 1000000
#define PAGE_SIZE 4096  // bytes

int main() {
    int *arr = malloc(NUM_INTS * sizeof(int));
    if (!arr) {
        perror("Memory allocation failed");
        return 1;
    }

    printf("Array base address: %p\n", (void *)arr);
    printf("Each page = %d bytes, int size = %lu bytes\n", PAGE_SIZE, sizeof(int));

    // Calculate number of ints per page
    int ints_per_page = PAGE_SIZE / sizeof(int);
    int total_pages = (NUM_INTS + ints_per_page - 1) / ints_per_page;

    printf("\nTotal elements: %d\n", NUM_INTS);
    printf("Ints per page: %d\n", ints_per_page);
    printf("Estimated number of pages: %d\n\n", total_pages);

    // Print starting address of each page
    for (int p = 0; p < total_pages; p++) {
        void *page_start = (void *)(arr + p * ints_per_page);
        printf("Page %4d starts at address %p\n", p, page_start);
    }

    printf("\n--- Element Page Mapping Example ---\n");
    // Show which page each element belongs to (for first few elements)
    for (int i = 0; i < 20; i++) {
        uintptr_t addr = (uintptr_t)&arr[i];
        int page_num = (addr - (uintptr_t)arr) / PAGE_SIZE;
        printf("Element arr[%d] (address %p) belongs to page %d\n", i, (void *)addr, page_num);
    }

    free(arr);
    return 0;
}

