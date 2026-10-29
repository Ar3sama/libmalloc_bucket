#ifndef BUCKET_H
#define BUCKET_H

#include <stddef.h>

#include "recycler.h"

struct bucket_meta
{
    struct bucket_meta *next;
    size_t mapping_length;
    size_t block_used;
    struct recycler recycler;
};

struct bucket_allocator
{
    struct bucket_meta *meta;
};

struct bucket_meta *bucket_alloc(struct bucket_allocator *allocator,
                                 size_t block_size, size_t nb_blocks);
int bucket_calculate_mapping_size(size_t block_size, size_t nb_blocks,
                                  size_t *metadata_offset_out,
                                  size_t *mapping_length_out);
struct bucket_meta *bucket_map_and_link(struct bucket_allocator *allocator,
                                        size_t block_size,
                                        size_t metadata_offset,
                                        size_t mapping_length);
int bucket_free(struct bucket_meta *blk);
void bucket_pop(struct bucket_allocator *blka);

#endif // BUCKET_H
