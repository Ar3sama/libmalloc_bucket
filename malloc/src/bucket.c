#include "bucket.h"

#include <sys/mman.h>
#include <unistd.h>

#include "alignment.h"

int bucket_calculate_mapping_size(size_t block_size, size_t nb_blocks,
                                  size_t *metadata_offset_out,
                                  size_t *mapping_length_out)
{
    if (!metadata_offset_out || !mapping_length_out || block_size == 0
        || nb_blocks == 0)
        return 1;

    size_t metadata_offset = align(sizeof(struct bucket_meta));

    if (metadata_offset == 0)
        return 1;

    size_t chunk_size;
    if (__builtin_mul_overflow(block_size, nb_blocks, &chunk_size))
        return 1;

    size_t total_size;
    if (__builtin_add_overflow(metadata_offset, chunk_size, &total_size))
        return 1;

    long page_size = sysconf(_SC_PAGESIZE);

    if (page_size <= 0)
        return 1;

    size_t page = page_size;
    size_t to_allocate = page;

    while (to_allocate < total_size)
    {
        if (__builtin_add_overflow(to_allocate, page, &to_allocate))
        {
            return 1;
        }
    }

    *metadata_offset_out = metadata_offset;
    *mapping_length_out = to_allocate;
    return 0;
}

struct bucket_meta *bucket_map_and_link(struct bucket_allocator *allocator,
                                        size_t block_size,
                                        size_t metadata_offset,
                                        size_t mapping_length)
{
    if (!allocator || block_size == 0 || metadata_offset > mapping_length)
        return NULL;

    void *mapping = mmap(NULL, mapping_length, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mapping == MAP_FAILED)
        return NULL;

    struct bucket_meta *new_bucket = mapping;
    char *mapping_bytes = mapping;
    char *chunk = mapping_bytes + metadata_offset;
    size_t available = mapping_length - metadata_offset;
    size_t usable = (available / block_size) * block_size;

    if (usable == 0)
    {
        munmap(mapping, mapping_length);
        return NULL;
    }

    if (recycler_init(&new_bucket->recycler, block_size, chunk, usable) != 0)
    {
        munmap(mapping, mapping_length);
        return NULL;
    }
    new_bucket->next = allocator->meta;
    new_bucket->block_used = 0;
    allocator->meta = mapping;
    new_bucket->mapping_length = mapping_length;

    return new_bucket;
}

struct bucket_meta *bucket_alloc(struct bucket_allocator *allocator,
                                 size_t block_size, size_t nb_blocks)
{
    if (!allocator)
        return NULL;

    size_t metadata_offset;
    size_t mapping_length;
    if (bucket_calculate_mapping_size(block_size, nb_blocks, &metadata_offset,
                                      &mapping_length)
        != 0)
        return NULL;

    return bucket_map_and_link(allocator, block_size, metadata_offset,
                               mapping_length);
}

int bucket_free(struct bucket_meta *bucket)
{
    return munmap(bucket, bucket->mapping_length);
}

void bucket_pop(struct bucket_allocator *blka)
{
    if (!blka->meta)
        return;
    struct bucket_meta *head = blka->meta;
    blka->meta = head->next;
    bucket_free(head);
}

void blka_delete(struct bucket_allocator *blka)
{
    while (blka->meta)
    {
        bucket_pop(blka);
    }
}
