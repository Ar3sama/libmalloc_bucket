#ifndef RECYCLER_H
#define RECYCLER_H

#include <stddef.h>

struct recycler
{
    size_t block_size;
    size_t capacity;
    void *chunk;
    void *free;
};

struct free_list
{
    struct free_list *next;
};

void *recycler_allocate(struct recycler *r);
void recycler_free(struct recycler *r, void *block);
int recycler_init(struct recycler *r, size_t block_size, void *chunk,
                  size_t total_size);

#endif // RECYCLER_H
