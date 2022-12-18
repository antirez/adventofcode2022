#include <stdlib.h>
#include <stdio.h>

/* Wrappers crashing on out of memory. */
void *xalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr,"Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return p;
}

void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr,size);
    if (!p) {
        fprintf(stderr,"Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return p;
}
