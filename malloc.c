#include "malloc.h"
#include "malloc_internal.h"
#include <unistd.h>
#include <assert.h>

List *free_list[BUCKET_COUNT];
char *start = NULL;
char *end = NULL;

void init() {
    for (int i = 0; i < BUCKET_COUNT; i++) {
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

void *xmalloc(size_t size) {
    size = ALIGN8(size);
    m_header_p p = find_chunk(size);

    if (!p) {
        p = create_chunk(size);
    }

    if ((char *)p != start) {
        p->size |= HAS_PREV_MASK;
        m_header_p last_h = get_prev_chunk(p);
        last_h->size |= HAS_NEXT_MASK;
        m_footer_p last_f = get_footer(last_h);
        last_f->size = last_h->size;
    }

    if (SIZE(p) > size) {
        split(p, size);
    }

    p->size &= ~FREE_MASK;
    m_footer_p f = get_footer(p);
    f->size = p->size;
    return (void *)(p + 1);
}

void xfree(void *p) {
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