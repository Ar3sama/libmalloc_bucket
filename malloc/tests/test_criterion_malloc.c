#include <criterion/criterion.h>
#include <stdint.h>
#include <unistd.h>

#include "../src/alignment.h"
#include "../src/bucket.h"
#include "../src/recycler.h"
#include "../src/utils.h"

union aligned_chunk
{
    long double long_double_alignment;
    struct free_list free_list_alignment;
    unsigned char bytes[sizeof(long double) * 64];
};

static size_t test_block_size(void)
{
    size_t block_size = sizeof(long double);

    while (block_size < sizeof(struct free_list))
        block_size += sizeof(long double);

    return block_size;
}

Test(alignment, rounds_sizes_and_handles_overflow)
{
    size_t unit = sizeof(long double);

    cr_assert_eq(align(0), 0);
    cr_assert_eq(align(unit), unit);
    cr_assert_eq(align(unit + 1), unit * 2);

    if (SIZE_MAX % unit == 0)
        cr_assert_eq(align(SIZE_MAX), SIZE_MAX);
    else
        cr_assert_eq(align(SIZE_MAX), 0);
}

Test(size_class, returns_power_of_two_classes_and_rejects_overflow)
{
    size_t minimum = size_class(0);
    size_t request = 42;

    cr_assert_geq(minimum, sizeof(long double));
    cr_assert_geq(minimum, sizeof(struct free_list));
    cr_assert_eq(minimum & (minimum - 1), 0);
    cr_assert_eq(size_class(minimum), minimum);
    cr_assert_eq(size_class(minimum + 1), minimum * 2);
    cr_assert_geq(size_class(request), request);
    cr_assert_eq(size_class(SIZE_MAX), 0);
}

Test(nb_blocks, returns_expected_block_counts)
{
    long page_result = sysconf(_SC_PAGESIZE);
    size_t page_size;

    cr_assert_gt(page_result, 1);
    page_size = page_result;

    cr_assert_eq(nb_blocks(0, 0), 0);
    cr_assert_eq(nb_blocks(1, page_size + 1), 2);
    cr_assert_eq(nb_blocks(1, 0), page_size);
    cr_assert_eq(nb_blocks(page_size * 4, 0), 2);
    cr_assert_eq(nb_blocks(page_size * 5, 0), 1);
}

Test(find_avalable_bucket, finds_a_matching_bucket_with_free_blocks)
{
    struct bucket_allocator empty = { .meta = NULL };
    struct bucket_allocator allocator;
    struct bucket_meta unavailable = { 0 };
    struct bucket_meta available = { 0 };
    char free_block_marker;

    cr_assert_null(find_avalable_bucket(NULL, 16));
    cr_assert_null(find_avalable_bucket(&empty, 16));
    cr_assert_null(find_avalable_bucket(&empty, 0));

    unavailable.recycler.block_size = 16;
    unavailable.recycler.free = NULL;
    unavailable.next = &available;
    available.recycler.block_size = 16;
    available.recycler.free = &free_block_marker;
    available.next = NULL;
    allocator.meta = &unavailable;

    cr_assert_null(find_avalable_bucket(&allocator, 8));
    cr_assert_eq(find_avalable_bucket(&allocator, 16), &available);
}

Test(find_corresponding_bucket, finds_bucket_and_previous_node)
{
    union aligned_chunk first_storage = { 0 };
    union aligned_chunk second_storage = { 0 };
    struct bucket_meta first = { 0 };
    struct bucket_meta second = { 0 };
    struct bucket_allocator allocator;
    struct bucket_meta *previous = NULL;
    struct bucket_meta *found;
    size_t block_size = test_block_size();

    first.recycler.block_size = block_size;
    first.recycler.capacity = 2;
    first.recycler.chunk = first_storage.bytes;
    first.block_used = 1;
    first.next = &second;
    second.recycler.block_size = block_size;
    second.recycler.capacity = 2;
    second.recycler.chunk = second_storage.bytes;
    second.block_used = 1;
    allocator.meta = &first;

    cr_assert_null(find_corresponding_bucket(&allocator, NULL, &previous));

    found = find_corresponding_bucket(
        &allocator, first_storage.bytes + block_size, &previous);
    cr_assert_eq(found, &first);
    cr_assert_null(previous);

    previous = NULL;
    found =
        find_corresponding_bucket(&allocator, second_storage.bytes, &previous);
    cr_assert_eq(found, &second);
    cr_assert_eq(previous, &first);

    cr_assert_null(find_corresponding_bucket(
        &allocator, first_storage.bytes + 1, &previous));
}

Test(ptr_to_uintptr_t, handles_null_and_non_null_pointers)
{
    char marker;
    uintptr_t zero = 0;

    cr_assert_eq(ptr_to_uintptr_t(NULL), zero);
    cr_assert_neq(ptr_to_uintptr_t(&marker), zero);
}

Test(recycler_init, validates_arguments_and_initializes_free_list)
{
    union aligned_chunk storage = { 0 };
    struct recycler recycler = { 0 };
    size_t block_size = test_block_size();
    size_t total_size = block_size * 3;

    cr_assert_eq(recycler_init(&recycler, 0, storage.bytes, total_size), 1);
    cr_assert_eq(recycler_init(&recycler, block_size, storage.bytes, 0), 1);
    cr_assert_eq(
        recycler_init(&recycler, block_size, storage.bytes, total_size - 1), 1);
    cr_assert_eq(
        recycler_init(&recycler, block_size + 1, storage.bytes, block_size + 1),
        1);
    cr_assert_eq(recycler_init(NULL, block_size, storage.bytes, total_size), 2);
    cr_assert_eq(recycler_init(&recycler, block_size, NULL, total_size), 2);

    cr_assert_eq(
        recycler_init(&recycler, block_size, storage.bytes, total_size), 0);
    cr_assert_eq(recycler.block_size, block_size);
    cr_assert_eq(recycler.capacity, 3);
    cr_assert_eq(recycler.chunk, storage.bytes);
    cr_assert_eq(recycler.free, storage.bytes);
}

Test(recycler_allocate, returns_each_free_block_once)
{
    union aligned_chunk storage = { 0 };
    struct recycler recycler = { 0 };
    size_t block_size = test_block_size();
    size_t total_size = block_size * 3;
    void *first;
    void *second;
    void *third;

    cr_assert_eq(
        recycler_init(&recycler, block_size, storage.bytes, total_size), 0);

    first = recycler_allocate(&recycler);
    second = recycler_allocate(&recycler);
    third = recycler_allocate(&recycler);
    cr_assert_not_null(first);
    cr_assert_not_null(second);
    cr_assert_not_null(third);
    cr_assert_neq(first, second);
    cr_assert_neq(second, third);
    cr_assert_null(recycler_allocate(&recycler));
    cr_assert_null(recycler_allocate(NULL));
}

Test(recycler_free, returns_a_block_to_the_free_list)
{
    union aligned_chunk storage = { 0 };
    struct recycler recycler = { 0 };
    size_t block_size = test_block_size();
    size_t total_size = block_size * 3;
    void *first;

    cr_assert_eq(
        recycler_init(&recycler, block_size, storage.bytes, total_size), 0);
    first = recycler_allocate(&recycler);
    cr_assert_not_null(first);

    recycler_free(NULL, first);
    recycler_free(&recycler, NULL);
    recycler_free(&recycler, first);
    cr_assert_eq(recycler_allocate(&recycler), first);
}

Test(bucket_calculate_mapping_size, calculates_valid_sizes_and_rejects_errors)
{
    size_t block_size = test_block_size();
    size_t metadata_offset = 0;
    size_t mapping_length = 0;

    cr_assert_eq(bucket_calculate_mapping_size(block_size, 2, &metadata_offset,
                                               &mapping_length),
                 0);
    cr_assert_geq(mapping_length, metadata_offset + block_size * 2);
    cr_assert_eq(
        bucket_calculate_mapping_size(0, 1, &metadata_offset, &mapping_length),
        1);
    cr_assert_eq(bucket_calculate_mapping_size(block_size, 0, &metadata_offset,
                                               &mapping_length),
                 1);
    cr_assert_eq(
        bucket_calculate_mapping_size(block_size, 1, NULL, &mapping_length), 1);
    cr_assert_eq(
        bucket_calculate_mapping_size(block_size, 1, &metadata_offset, NULL),
        1);
    cr_assert_eq(bucket_calculate_mapping_size(SIZE_MAX, 2, &metadata_offset,
                                               &mapping_length),
                 1);
}

Test(bucket_map_and_link, maps_and_links_a_valid_bucket)
{
    struct bucket_allocator allocator = { .meta = NULL };
    struct bucket_meta *bucket;
    size_t block_size = test_block_size();
    size_t metadata_offset = 0;
    size_t mapping_length = 0;

    cr_assert_eq(bucket_calculate_mapping_size(block_size, 2, &metadata_offset,
                                               &mapping_length),
                 0);
    cr_assert_null(
        bucket_map_and_link(NULL, block_size, metadata_offset, mapping_length));
    cr_assert_null(
        bucket_map_and_link(&allocator, 0, metadata_offset, mapping_length));
    cr_assert_null(bucket_map_and_link(&allocator, block_size,
                                       mapping_length + 1, mapping_length));

    bucket = bucket_map_and_link(&allocator, block_size, metadata_offset,
                                 mapping_length);
    cr_assert_not_null(bucket);
    cr_assert_eq(allocator.meta, bucket);
    cr_assert_eq(bucket->recycler.block_size, block_size);
    cr_assert_geq(bucket->recycler.capacity, 1);
    bucket_pop(&allocator);
    cr_assert_null(allocator.meta);
}

Test(bucket_alloc, creates_a_bucket_and_rejects_invalid_arguments)
{
    struct bucket_allocator allocator = { .meta = NULL };
    struct bucket_meta *bucket;
    size_t block_size = test_block_size();

    cr_assert_null(bucket_alloc(NULL, block_size, 1));
    cr_assert_null(bucket_alloc(&allocator, 0, 1));
    cr_assert_null(bucket_alloc(&allocator, block_size, 0));

    bucket = bucket_alloc(&allocator, block_size, 1);
    cr_assert_not_null(bucket);
    cr_assert_eq(allocator.meta, bucket);
    cr_assert_eq(bucket->recycler.block_size, block_size);
    bucket_pop(&allocator);
    cr_assert_null(allocator.meta);
}

Test(bucket_free, unmaps_a_valid_bucket)
{
    struct bucket_allocator allocator = { .meta = NULL };
    struct bucket_meta *bucket;
    size_t block_size = test_block_size();

    bucket = bucket_alloc(&allocator, block_size, 1);
    cr_assert_not_null(bucket);
    allocator.meta = NULL;
    cr_assert_eq(bucket_free(bucket), 0);
}

Test(bucket_pop, removes_buckets_from_the_head_of_the_list)
{
    struct bucket_allocator allocator = { .meta = NULL };
    struct bucket_meta *first;
    struct bucket_meta *second;
    size_t block_size = test_block_size();

    first = bucket_alloc(&allocator, block_size, 1);
    cr_assert_not_null(first);
    second = bucket_alloc(&allocator, block_size, 1);
    cr_assert_not_null(second);
    cr_assert_eq(allocator.meta, second);

    bucket_pop(&allocator);
    cr_assert_eq(allocator.meta, first);
    bucket_pop(&allocator);
    cr_assert_null(allocator.meta);
    bucket_pop(&allocator);
    cr_assert_null(allocator.meta);
}
