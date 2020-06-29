#include "mem.h"

void *safe_malloc(size_t size) {
    void *mem = malloc(size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "malloc failed.\n");
    exit(1);
}

void *safe_calloc(size_t members, size_t member_size) {
    void *mem = calloc(members, member_size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "calloc failed.\n");
    exit(1);
}

void *safe_realloc(void *mem, size_t size) {
    mem = realloc(mem, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "realloc failed.\n");
    exit(1);
}

void *safe_box(void *stack_struct, size_t size) {
    void *mem = safe_malloc(size);
    memcpy(mem, stack_struct, size);
    return mem;
}

void *safe_aligned_malloc(size_t size, size_t alignment) {
    void *mem = aligned_alloc(alignment, size);
    if (mem) {
        return mem;
    }
    fprintf(stderr, "aligned_alloc failed.\n");
    exit(1);
}
