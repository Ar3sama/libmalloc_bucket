#!/bin/sh

test_bin="./malloc_functional"
result=0

gcc -std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fno-builtin-malloc \
    -o "$test_bin" tests/malloc_functional.c -L. -lmalloc >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test compilation : OK"
else
    echo "test compilation : KO"
    exit 1
fi

LD_LIBRARY_PATH=. "$test_bin" >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test suite : OK"
else
    echo "test suite : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so factor 20 30 40 50 60 70 80 90 >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test factor : OK"
else
    echo "test factor : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so cat Makefile >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test cat : OK"
else
    echo "test cat : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so grep -n "check" Makefile >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test grep : OK"
else
    echo "test grep : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so ip a >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test ip : OK"
else
    echo "test ip : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so ls -la >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test ls : OK"
else
    echo "test ls : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so find . -maxdepth 2 -type f >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test find : OK"
else
    echo "test find : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so find /etc >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test find_slash_etc : OK"
else
    echo "test find_slash_etc : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so tree /etc >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test tree_slash_etc : OK"
else
    echo "test tree_slash_etc : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so od -An -tx1 -N16 libmalloc.so >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test od : OK"
else
    echo "test od : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so git status --short >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test git : OK"
else
    echo "test git : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so clang --help >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test clang_help : OK"
else
    echo "test clang_help : KO"
    result=1
fi

LD_PRELOAD=./libmalloc.so make -B library >/dev/null 2>&1
if [ "$?" -eq 0 ]; then
    echo "test recompile self : OK"
else
    echo "test recomplie self : KO"
    result=1
fi

exit "$result"
