#include "recycler.h"

int recycler_init(struct recycler *r, size_t block_size, void *chunk,
                  size_t total_size)
{
    if (block_size == 0 || total_size == 0 || total_size % block_size != 0
        || block_size % sizeof(long double) != 0
        || block_size < sizeof(struct free_list))
    {
        return 1;
    }
    if (!r || !chunk)
        return 2;

    r->block_size = block_size;
    r->capacity = total_size / block_size;
    r->chunk = chunk;

    r->free = r->chunk;
    char *base = r->chunk;
    struct free_list *p = r->chunk;

    for (size_t i = 0; i < r->capacity - 1; i++)
    {
        void *next = base + (i + 1) * block_size;
        p->next = next;
        p = p->next;
    }

    p->next = NULL;
    return 0;
}

void *recycler_allocate(struct recycler *r)
{
    if (!r || !r->free)
        return NULL;
    struct free_list *curr = r->free;
    r->free = curr->next;
    return curr;
}

void recycler_free(struct recycler *r, void *block)
{
    if (!r || !block)
        return;

    struct free_list *freed = block;
    freed->next = r->free;
    r->free = freed;
}
