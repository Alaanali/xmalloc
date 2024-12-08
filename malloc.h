#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

void *xmalloc(size_t size);
void xfree(void *ptr);
void init(void);

#endif