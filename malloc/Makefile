CC = gcc

CPPFLAGS = -D_DEFAULT_SOURCE
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fvisibility=hidden -fPIC
LDFLAGS = -shared -Wl,--no-undefined

SRC = src/malloc.c src/my_malloc.c src/bucket.c src/alignment.c src/recycler.c src/utils.c
OBJ = malloc.o my_malloc.o bucket.o alignment.o recycler.o utils.o
TARGET = libmalloc.so


library: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

check: $(TARGET)
	$(MAKE) -C tests check
	./tests/test.sh

clean:
	rm -f $(OBJ) $(TARGET)
	rm -f malloc_functional
	rm -f malloc_free_functional
	$(MAKE) -C tests clean

.PHONY: library check clean

