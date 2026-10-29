#include <stdint.h>
#include <stdio.h>

#include "../src/malloc.h"

static void print_result(const char *name, int success)
{
    printf("%s: %s\n", name, success ? "OK" : "KO");
}

static int test_malloc(void)
{
    unsigned char *m1 = malloc(32);
    unsigned char *m2 = malloc(32);
    unsigned char *m3 = malloc(145);
    unsigned char *m4 = malloc(145);

    unsigned char *m5 = malloc(145);
    unsigned char *m6 = malloc(500);

    int success = m1 && m2 && m3 && m4 && m5 && m6;

    if (success)
    {
        m1[0] = 17;
        m2[31] = 93;
        m3[0] = 4;
        m4[100] = 42;
        m5[140] = 12;
        m6[0] = 2;
        success = m1[0] == 17 && m2[31] == 93 && m3[0] == 4 && m4[100] == 42
            && m5[140] == 12 && m6[0] == 2;
    }

    print_result("malloc", success);
    return success;
}

static int test_free(void)
{
    unsigned char *block = malloc(24);
    int success = block != NULL;

    if (block)
    {
        block[0] = 51;
        free(block);
    }

    free(NULL);
    block = malloc(24);
    if (!block)
        success = 0;
    else
    {
        block[0] = 72;
        if (block[0] != 72)
            success = 0;
        free(block);
    }

    print_result("free", success);
    return success;
}

static int test_realloc(void)
{
    unsigned char *block = malloc(8);
    unsigned char *resized;
    int success = block != NULL;

    if (block)
    {
        block[0] = 11;
        block[1] = 29;
        resized = realloc(block, 64);
        if (!resized)
        {
            success = 0;
            free(block);
        }
        else
        {
            if (resized[0] != 11 || resized[1] != 29)
                success = 0;
            free(resized);
        }
    }

    resized = realloc(NULL, 16);
    if (!resized)
        success = 0;
    else
        free(resized);

    print_result("realloc", success);
    return success;
}

static int test_calloc(void)
{
    size_t count = 4;
    size_t bytes_count = count * sizeof(int);
    volatile size_t overflow_count = SIZE_MAX;
    unsigned char *bytes = calloc(count, sizeof(int));
    int success = bytes != NULL;

    if (bytes)
    {
        for (size_t i = 0; i < bytes_count; i++)
        {
            if (bytes[i] != 0)
                success = 0;
        }
        free(bytes);
    }

    if (calloc(overflow_count, 2) != NULL)
        success = 0;

    print_result("calloc", success);
    return success;
}

int main(void)
{
    int failures = 0;

    if (!test_malloc())
        failures++;
    if (!test_free())
        failures++;
    if (!test_realloc())
        failures++;
    if (!test_calloc())
        failures++;

    return failures == 0 ? 0 : 1;
}
