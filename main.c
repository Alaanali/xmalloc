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

typedef struct Chunk *t_chunk;
typedef struct Chunk
{
    size_t size;
} Chunk;

#define META_SIZE sizeof(Chunk)

t_chunk head = NULL;

t_chunk get_footer(t_chunk h)
{
    char *f = (char *)h;
    f += SIZE(h) + META_SIZE;
    return (t_chunk)f;
}

t_chunk get_header(t_chunk f)
{
    char *h = (char *)f;
    h -= SIZE(f) + META_SIZE;
    return (t_chunk)h;
}

t_chunk get_next_chunk(t_chunk p)
{
    if (!HAS_NEXT(p))
        return NULL;

    char *n = (char *)p;
    n += META_SIZE + SIZE(p) + META_SIZE;
    return (t_chunk)n;
}

t_chunk get_prev_chunk(t_chunk p)
{
    if (!HAS_PREV(p))
        return NULL;
    t_chunk f = p - 1;

    char *n = (char *)f;
    n -= META_SIZE + SIZE(f);
    return (t_chunk)n;
}

t_chunk create_chunk(size_t size)
{
    t_chunk p = sbrk(0);
    if (sbrk(size + (2 * META_SIZE)) == (void *)-1)
        return NULL;

    p->size = size;
    p->size |= FREE_MASK;

    t_chunk f = get_footer(p);
    f->size = size;
    f->size |= FREE_MASK;
    return p;
}

t_chunk find_chunk(t_chunk *last, size_t size)
{
    t_chunk current = head;
    while (current && !(IS_FREE(current) && (SIZE(current)) >= size))
    {
        *last = current;
        current = get_next_chunk(current);
    }
    return current;
}

void *my_malloc(size_t size)
{
    size = align8(size);
    if (!head)
    {
        head = create_chunk(size);
    }
    t_chunk last = head;
    t_chunk p = find_chunk(&last, size);
    if (!p)
    {
        p = create_chunk(size);
    }

    if (last != p)
    {
        last->size |= HAS_NEXT_MASK;
        p->size |= HAS_PREV_MASK;
    }

    p->size &= ~FREE_MASK;
    t_chunk f = get_footer(p);
    f->size = p->size;
    return (void *)(p + 1);
}

t_chunk fusion(t_chunk chunk)
{
    t_chunk next = get_next_chunk(chunk);

    if (next && IS_FREE(next))
    {
        t_chunk n_next = get_next_chunk(next);
        chunk->size += META_SIZE + SIZE(next) + META_SIZE;
        if (!n_next)
        {
            chunk->size &= ~HAS_NEXT_MASK;
        }
    }

    t_chunk prev = get_prev_chunk(chunk);
    if (prev && IS_FREE(prev))
    {
        prev->size += META_SIZE + SIZE(chunk) + META_SIZE;
        prev->size |= chunk->size & HAS_NEXT_MASK;
        chunk = prev;
    }

    return chunk;
}

void my_free(void *p)
{
    t_chunk chunk = (t_chunk)p - 1;

    assert(IS_FREE(chunk) == 0);

    chunk = fusion(chunk);

    chunk->size |= FREE_MASK;

    t_chunk footer = get_footer(chunk);
    footer->size = chunk->size;
}

int main()
{
    int *a = my_malloc(sizeof(int));
    int *b = my_malloc(sizeof(int));
    int *c = my_malloc(sizeof(int));
    Chunk *aa = (Chunk *)a - 1;
    Chunk *bb = (Chunk *)b - 1;

    Chunk *cc = (Chunk *)c - 1;

    *a = 5;
    my_free(c);
    my_free(a);
    my_free(b);
    printf("b %ld\n", SIZE(bb));
    printf("c %ld\n", SIZE(cc));
    printf("a %ld\n", SIZE(aa));
    return 0;
}