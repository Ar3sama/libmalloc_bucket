#ifndef MALLOC_H
#define MALLOC_H
#include <stddef.h>

__attribute__((visibility("default"))) void *malloc(size_t size);

__attribute__((visibility("default"))) void free(void *ptr);

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size);

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size);

#endif // MALLOC_H
