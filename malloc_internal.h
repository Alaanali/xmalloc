#ifndef MALLOC_INTERNAL_H
#define MALLOC_INTERNAL_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>


#define SIZE_MASK -8
#define FREE_MASK 1
#define HAS_PREV_MASK 2
#define HAS_NEXT_MASK 4
#define BUCKET_COUNT 8
#define MINIMUM_CHUNK_SIZE 8

#define ALIGN8(x) (((((x) - 1) >> 3) << 3) + 8)
#define SIZE(p) ((p->size) & SIZE_MASK)
#define IS_FREE(p) ((p->size) & FREE_MASK)
#define HAS_PREV(p) ((p->size) & HAS_PREV_MASK)
#define HAS_NEXT(p) ((p->size) & HAS_NEXT_MASK)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define IDX(size) (MIN(7, ((size) < 9 ? 0 : (63 - __builtin_clzll((size) - 1) - 3))))

// Forward declarations
typedef struct Meta_Header Meta_Header;
typedef struct Meta_Footer Meta_Footer;
typedef struct List List;

typedef Meta_Header *m_header_p;
typedef Meta_Footer *m_footer_p;

// Structure definitions
struct Meta_Header {
    size_t size;
    m_header_p next;
    m_header_p prev;
};

struct Meta_Footer {
    size_t size;
};

struct List {
    m_header_p head;
    m_header_p tail;
};

#define META_HEADER_SIZE sizeof(Meta_Header)
#define META_FOOTER_SIZE sizeof(Meta_Footer)

// Internal function declarations
m_footer_p get_footer(m_header_p h);
m_header_p get_header(m_footer_p f);
m_header_p get_next_chunk(m_header_p p);
m_header_p get_prev_chunk(m_header_p p);
m_header_p create_chunk(size_t size);
m_header_p find_chunk(size_t size);
void split(m_header_p p, size_t size);
m_header_p fusion(m_header_p chunk);

// Global variables declarations
extern List *free_list[BUCKET_COUNT];
extern char *start;
extern char *end;

#endif