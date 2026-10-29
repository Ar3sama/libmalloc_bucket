CC = gcc

CPPFLAGS = -D_DEFAULT_SOURCE -I../src
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
CRITERION_LIBS = -lcriterion

CRITERION_TEST_SRC = test_criterion_malloc.c \
	../src/my_malloc.c ../src/bucket.c ../src/alignment.c \
	../src/recycler.c ../src/utils.c

CRITERION_TEST_BIN = criterion_testsuit

all: $(CRITERION_TEST_BIN)

$(CRITERION_TEST_BIN): $(CRITERION_TEST_SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(CRITERION_TEST_SRC) $(CRITERION_LIBS)

check: all
	./$(CRITERION_TEST_BIN)

clean:
	rm -f $(CRITERION_TEST_BIN) $(FUNCTIONAL_TEST_BINS)

.PHONY: all check clean
