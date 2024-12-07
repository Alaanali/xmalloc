#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

void *my_malloc(size_t size);
void my_free(void *ptr);
void init(void);

#endif