#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#define align8(x) (((((x) - 1) >> 3) << 3) + 8)
#define SIZE_MASK -8
#define FREE_MASK 1
#define HAS_PREV_MASK 2
#define HAS_NEXT_MASK 4

#define SIZE(p) ((p->size) & SIZE_MASK)
#define IS_FREE(p) ((p->size) & FREE_MASK)
#define HAS_PREV(p) ((p->size) & HAS_PREV_MASK)
#define HAS_NEXT(p) ((p->size) & HAS_NEXT_MASK)

typedef struct Meta_Footer *m_footer;
typedef struct Meta_Footer
{
    size_t size;
} Meta_Footer;

typedef struct Meta_Header *m_header;

typedef struct Meta_Header
{
    size_t size;
    m_header next; // TODO this can be added in free only
    m_header prev;
} Meta_Header;

#define META_HEADER_SIZE sizeof(Meta_Header)
#define META_FOOTER_SIZE sizeof(Meta_Footer)

static Meta_Header head_sentinel = {0, NULL, NULL};
static Meta_Header tail_sentinel = {0, NULL, NULL};

static m_header const HEAD = &head_sentinel;
static m_header const TAIL = &tail_sentinel;
char *start = NULL;
char *end = NULL;

void init()
{
    head_sentinel.next = TAIL;
    tail_sentinel.prev = HEAD;
    start = NULL;
    end = NULL;
}

m_footer get_footer(m_header h)
{
    char *f = (char *)h;
    f += SIZE(h) + META_HEADER_SIZE;
    return (m_footer)f;
}

m_header get_header(m_footer f)
{
    char *h = (char *)f;
    h -= SIZE(f) + META_HEADER_SIZE;
    return (m_header)h;
}

m_header get_next_chunk(m_header p)
{
    if (((char *)p >= end) || !HAS_NEXT(p))
        return NULL;

    char *n = (char *)p;
    n += META_HEADER_SIZE + SIZE(p) + META_FOOTER_SIZE;
    return (m_header)n;
}

m_header get_prev_chunk(m_header p)
{
    if (((char *)p <= start) || !HAS_PREV(p))
        return NULL;
    m_footer f = ((m_footer)p) - 1;
    char *n = (char *)p;
    n -= META_FOOTER_SIZE + SIZE(f) + META_HEADER_SIZE;
    return (m_header)n;
}

m_header create_chunk(size_t size)
{
    m_header p = sbrk(0);
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

    m_footer f = get_footer(p);
    f->size = size;
    f->size |= FREE_MASK;
    return p;
}

m_header find_chunk(size_t size)
{
    m_header current = HEAD->next;
    while (current && current->next && SIZE(current) < size)
    {
        current = current->next;
    }

    if (current != TAIL)
    {
        current->next->prev = current->prev;
        current->prev->next = current->next;
        current->next = NULL;
        current->prev = NULL;
        return current;
    }
    return NULL;
}

void *my_malloc(size_t size)
{
    size = align8(size);
    m_header p = find_chunk(size);

    if (!p)
    {
        p = create_chunk(size);
    }

    if ((char *)p != start)
    {
        p->size |= HAS_PREV_MASK;
        m_header last_h = get_prev_chunk(p);
        last_h->size |= HAS_NEXT_MASK;
        m_footer last_f = get_footer(last_h);
        last_f->size = last_h->size;
    }

    p->size &= ~FREE_MASK;
    m_footer f = get_footer(p);
    f->size = p->size;
    return (void *)(p + 1);
}

m_header fusion(m_header chunk)
{
    m_header next = get_next_chunk(chunk);

    if (next && IS_FREE(next))
    {
        chunk->size += META_HEADER_SIZE + SIZE(next) + META_FOOTER_SIZE;

        next->next->prev = next->prev;
        next->prev->next = next->next;

        m_header n_next = get_next_chunk(next);
        if (!n_next)
        {
            chunk->size &= ~HAS_NEXT_MASK;
        }
    }

    m_header prev = get_prev_chunk(chunk);
    if (prev && IS_FREE(prev))
    {

        prev->next->prev = prev->prev;
        prev->prev->next = prev->next;

        prev->size += META_HEADER_SIZE + SIZE(chunk) + META_FOOTER_SIZE;
        prev->size |= chunk->size & HAS_NEXT_MASK;
        chunk = prev;
    }

    m_footer footer = get_footer(chunk);
    footer->size = chunk->size;

    return chunk;
}

void my_free(void *p)
{
    m_header chunk = (m_header)p - 1;

    assert(IS_FREE(chunk) == 0);

    chunk = fusion(chunk);

    chunk->size |= FREE_MASK;
    m_footer footer = get_footer(chunk);
    footer->size |= FREE_MASK;

    chunk->next = HEAD->next;
    chunk->prev = HEAD;
    HEAD->next->prev = chunk;
    HEAD->next = chunk;
}

int main()
{
    init();
    int *a = my_malloc(sizeof(int));
    m_header aa = (m_header)a - 1;
    my_free(a);
    int *b = my_malloc(sizeof(int));
    m_header bb = (m_header)b - 1;
    my_free(b);
    int *c = my_malloc(2 * sizeof(int));

    m_header cc = (m_header)c - 1;

    assert(b == a);
    assert(c == a);
    *a = 5;
    my_free(c);
    printf("b %ld\n", SIZE(bb));
    printf("c %ld\n", SIZE(cc));
    printf("a %ld\n", SIZE(aa));
    return 0;
}