#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#define align8(x) (((((x) - 1) >> 3) << 3) + 8)
#define SIZE_MASK -8
#define FREE_MASK 1
#define HAS_PREV_MASK 2
#define HAS_NEXT_MASK 4
#define BUCKET_COUNT 8

#define SIZE(p) ((p->size) & SIZE_MASK)
#define IS_FREE(p) ((p->size) & FREE_MASK)
#define HAS_PREV(p) ((p->size) & HAS_PREV_MASK)
#define HAS_NEXT(p) ((p->size) & HAS_NEXT_MASK)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define IDX(size) ( \
    MIN(7, ((size) < 9 ? 0 : (63 - __builtin_clzll((size) - 1) - 3))))

typedef struct Meta_Footer
{
    size_t size;
} Meta_Footer;
typedef struct Meta_Footer *m_footer_p;

typedef struct Meta_Header *m_header_p;
typedef struct Meta_Header
{
    size_t size;
    m_header_p next;
    m_header_p prev;
} Meta_Header;

#define META_HEADER_SIZE sizeof(Meta_Header)
#define META_FOOTER_SIZE sizeof(Meta_Footer)

typedef struct List
{
    m_header_p head;
    m_header_p tail;
} List;

List *free_list[BUCKET_COUNT];

char *start = NULL;
char *end = NULL;

void init()
{

    for (int i = 0; i < BUCKET_COUNT; i++)
    {
        free_list[i] = sbrk(0);
        sbrk(sizeof(List));

        m_header_p head = sbrk(0);
        sbrk(sizeof(Meta_Header));

        m_header_p tail = sbrk(0);
        sbrk(sizeof(Meta_Header));

        head->next = tail;
        tail->prev = head;
        free_list[i]->head = head;
        free_list[i]->tail = tail;
    }

    start = NULL;
    end = NULL;
}

m_footer_p get_footer(m_header_p h)
{
    char *f = (char *)h;
    f += SIZE(h) + META_HEADER_SIZE;
    return (m_footer_p)f;
}

m_header_p get_header(m_footer_p f)
{
    char *h = (char *)f;
    h -= SIZE(f) + META_HEADER_SIZE;
    return (m_header_p)h;
}

m_header_p get_next_chunk(m_header_p p)
{
    if (((char *)p >= end) || !HAS_NEXT(p))
        return NULL;

    char *n = (char *)p;
    n += META_HEADER_SIZE + SIZE(p) + META_FOOTER_SIZE;
    return (m_header_p)n;
}

m_header_p get_prev_chunk(m_header_p p)
{
    if (((char *)p <= start) || !HAS_PREV(p))
        return NULL;
    m_footer_p f = ((m_footer_p)p) - 1;
    char *n = (char *)p;
    n -= META_FOOTER_SIZE + SIZE(f) + META_HEADER_SIZE;
    return (m_header_p)n;
}

m_header_p create_chunk(size_t size)
{
    m_header_p p = sbrk(0);
    if (sbrk(size + META_HEADER_SIZE + META_FOOTER_SIZE) == (void *)-1)
        return NULL;

    if (!start)
    {
        start = (char *)p;
    }
    end = (char *)p;

    p->size = size;
    p->size |= FREE_MASK;
    p->next = NULL;
    p->prev = NULL;

    m_footer_p f = get_footer(p);
    f->size = size;
    f->size |= FREE_MASK;
    return p;
}

m_header_p find_chunk(size_t size)
{
    int i = IDX(size);
    for (i; i < BUCKET_COUNT; i++)
    {
        List *list = free_list[i];
        m_header_p current = list->head->next;
        while (current && current->next && SIZE(current) < size)
        {
            current = current->next;
        }

        if (current != list->tail)
        {
            current->next->prev = current->prev;
            current->prev->next = current->next;
            current->next = NULL;
            current->prev = NULL;
            return current;
        }
    }

    return NULL;
}

void *my_malloc(size_t size)
{

    size = align8(size);
    m_header_p p = find_chunk(size);

    if (!p)
    {
        p = create_chunk(size);
    }

    if ((char *)p != start)
    {
        p->size |= HAS_PREV_MASK;
        m_header_p last_h = get_prev_chunk(p);
        last_h->size |= HAS_NEXT_MASK;
        m_footer_p last_f = get_footer(last_h);
        last_f->size = last_h->size;
    }

    p->size &= ~FREE_MASK;
    m_footer_p f = get_footer(p);
    f->size = p->size;
    return (void *)(p + 1);
}

m_header_p fusion(m_header_p chunk)
{
    m_header_p next = get_next_chunk(chunk);

    if (next && IS_FREE(next))
    {
        chunk->size += META_HEADER_SIZE + SIZE(next) + META_FOOTER_SIZE;

        if ((void *)next == end)
        {
            end = (void *)chunk;
        }
        next->next->prev = next->prev;
        next->prev->next = next->next;

        m_header_p n_next = get_next_chunk(next);
        if (!n_next)
        {
            chunk->size &= ~HAS_NEXT_MASK;
        }
    }

    m_header_p prev = get_prev_chunk(chunk);
    if (prev && IS_FREE(prev))
    {

        prev->next->prev = prev->prev;
        prev->prev->next = prev->next;

        prev->size += META_HEADER_SIZE + SIZE(chunk) + META_FOOTER_SIZE;
        prev->size |= chunk->size & HAS_NEXT_MASK;
        chunk = prev;
    }

    m_footer_p footer = get_footer(chunk);
    footer->size = chunk->size;

    return chunk;
}

void my_free(void *p)
{
    m_header_p chunk = (m_header_p)p - 1;
    assert(IS_FREE(chunk) == 0);

    chunk = fusion(chunk);
    List *list = free_list[IDX(SIZE(chunk))];

    chunk->size |= FREE_MASK;
    m_footer_p footer = get_footer(chunk);
    footer->size |= FREE_MASK;

    chunk->next = list->head->next;
    chunk->prev = list->head;
    list->head->next->prev = chunk;
    list->head->next = chunk;
}

int main()
{
    init();
    // Allocate multiple small chunks
    void *p1 = my_malloc(16);
    void *p2 = my_malloc(32);
    void *p3 = my_malloc(64);

    // Free the second chunk
    my_free(p2);

    // Allocate a chunk that fits exactly into the freed space
    void *p4 = my_malloc(32);
    assert(p4 == p2);

    // Free adjacent chunks and test coalescing
    my_free(p1);
    my_free(p3);
    my_free(p4);

    // Allocate a large chunk spanning coalesced space
    void *p5 = my_malloc(112);
    assert(p5 == p1); // Since p1 was the start of coalesced space

    printf("All tests passed.\n");
}