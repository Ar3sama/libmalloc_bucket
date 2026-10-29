#include "my_malloc.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "alignment.h"
#include "bucket.h"
#include "recycler.h"
#include "utils.h"

struct bucket_allocator allocator = { .meta = NULL };

void *my_malloc(size_t size)
{
    size_t class = size_class(size);
    if (class == 0)
        return NULL;

    struct bucket_meta *bucket = find_avalable_bucket(&allocator, class);
    size_t metadata_offset = align(sizeof(struct bucket_meta));
    if (metadata_offset == 0)
        return NULL;

    if (!bucket)
    {
        size_t blocks_n = nb_blocks(class, metadata_offset);
        if (blocks_n == 0)
            return NULL;
        bucket = bucket_alloc(&allocator, class, blocks_n);
        if (!bucket)
            return NULL;
    }

    void *block = recycler_allocate(&bucket->recycler);
    if (block)
        bucket->block_used++;

    return block;
}

void my_free(void *ptr)
{
    if (!ptr)
        return;
    struct bucket_meta *prev = NULL;
    struct bucket_meta *bucket =
        find_corresponding_bucket(&allocator, ptr, &prev);
    if (!bucket)
        return;

    recycler_free(&bucket->recycler, ptr);
    bucket->block_used--;

    if (bucket->block_used == 0)
    {
        struct bucket_meta *next = bucket->next;
        if (prev)
            prev->next = next;
        else
            allocator.meta = next;

        if (bucket_free(bucket) != 0)
        {
            bucket->next = next;
            if (prev)
                prev->next = bucket;
            else
                allocator.meta = bucket;
        }
        return;
    }
}
void *my_realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        return my_malloc(size);
    };
    if (size == 0)
    {
        my_free(ptr);
        return NULL;
    }

    struct bucket_meta *prev = NULL;
    struct bucket_meta *bucket =
        find_corresponding_bucket(&allocator, ptr, &prev);
    if (!bucket)
        return NULL;

    size_t prev_size = bucket->recycler.block_size;
    if (prev_size >= size)
    {
        return ptr;
    }
    else
    {
        void *new_ptr = my_malloc(size);
        if (!new_ptr)
        {
            return NULL;
        }
        void *res = memcpy(new_ptr, ptr, prev_size);
        my_free(ptr);
        return res;
    }
}
void *my_calloc(size_t nmemb, size_t size)
{
    size_t total_size;
    if (__builtin_mul_overflow(nmemb, size, &total_size))
    {
        errno = ENOMEM;
        return NULL;
    }

    void *ptr = my_malloc(total_size);
    if (!ptr)
    {
        errno = ENOMEM;
        return NULL;
    }

    void *res = set_to_zeros(ptr, total_size);
    return res;
}
