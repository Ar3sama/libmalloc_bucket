#include "utils.h"

#include <stdint.h>
#include <unistd.h>

#include "bucket.h"

struct bucket_meta *find_avalable_bucket(struct bucket_allocator *allocator,
                                         size_t size)
{
    if (!allocator || !allocator->meta)
        return NULL;
    if (size == 0)
        return NULL;

    struct bucket_meta *bucket = allocator->meta;
    while (bucket)
    {
        if (bucket->recycler.block_size == size && bucket->recycler.free)
        {
            return bucket;
        }
        bucket = bucket->next;
    }
    return NULL;
}

struct bucket_meta *
find_corresponding_bucket(struct bucket_allocator *allocator, void *ptr,
                          struct bucket_meta **prev_out)
{
    if (!ptr)
        return NULL;
    uintptr_t address = ptr_to_uintptr_t(ptr);
    struct bucket_meta *prev = NULL;
    struct bucket_meta *bucket = allocator->meta;

    while (bucket)
    {
        size_t block_size = bucket->recycler.block_size;
        size_t chunk_size;

        if (block_size != 0
            && !__builtin_mul_overflow(bucket->recycler.capacity, block_size,
                                       &chunk_size))
        {
            uintptr_t chunk_start = ptr_to_uintptr_t(bucket->recycler.chunk);
            if (address >= chunk_start)
            {
                uintptr_t offset = address - chunk_start;
                if (offset < chunk_size && offset % block_size == 0)
                {
                    if (bucket->block_used == 0)
                        return NULL;

                    *prev_out = prev;
                    return bucket;
                }
            }
        }
        prev = bucket;
        bucket = bucket->next;
    }
    return NULL;
}

size_t size_class(size_t size)
{
    size_t minimum = sizeof(long double);

    if (minimum < sizeof(struct free_list))
        minimum = sizeof(struct free_list);

    if (size < minimum)
        size = minimum;

    size_t block_size = 1;
    while (block_size < size)
    {
        if (block_size > SIZE_MAX / 2)
            return 0;

        block_size *= 2;
    }

    return block_size;
}
size_t nb_blocks(size_t block_size, size_t metadata_offset)
{
    long page = sysconf(_SC_PAGESIZE);
    if (page < 0)
        return 0;
    size_t page_size = page;

    if (block_size == 0 || page_size == 0)
        return 0;

    size_t per_page = 0;
    if (page_size > metadata_offset)
    {
        per_page = (page_size - metadata_offset) / block_size;
    }

    if (per_page >= 2)
        return per_page;

    size_t pages_per_block = block_size / page_size;
    size_t reste = block_size % page_size;
    if (pages_per_block < 4 || (pages_per_block == 4 && reste == 0))
        return 2;

    return 1;
}

uintptr_t ptr_to_uintptr_t(void *ptr)
{
    return (uintptr_t)ptr;
}

void *set_to_zeros(void *ptr, size_t size)
{
    if (!ptr)
        return NULL;
    char *p = ptr;
    for (size_t i = 0; i < size; i++)
    {
        *p = 0;
        p++;
    }
    return ptr;
}
