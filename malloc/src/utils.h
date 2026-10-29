#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "bucket.h"

struct bucket_meta *find_avalable_bucket(struct bucket_allocator *allocator,
                                         size_t size);
struct bucket_meta *
find_corresponding_bucket(struct bucket_allocator *allocator, void *ptr,
                          struct bucket_meta **prev_out);
size_t size_class(size_t size);
size_t nb_blocks(size_t block_size, size_t metadata_offset);

uintptr_t ptr_to_uintptr_t(void *ptr);
void *set_to_zeros(void *ptr, size_t size);

#endif // UTILS_H
