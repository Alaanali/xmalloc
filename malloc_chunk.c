#include "malloc_internal.h"

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
    for ( ;i < BUCKET_COUNT; i++)
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

void split(m_header_p p, size_t size)
{
    size_t remaining = SIZE(p) - size - META_HEADER_SIZE - META_FOOTER_SIZE;

    if (remaining < META_HEADER_SIZE + META_FOOTER_SIZE + MINIMUM_CHUNK_SIZE)
        return;

    m_header_p new = (void *)p + META_HEADER_SIZE + size + META_FOOTER_SIZE;
    new->size = remaining;
    new->size |= HAS_PREV_MASK | FREE_MASK | (p->size & HAS_NEXT_MASK);

    p->size = size | (p->size & HAS_PREV_MASK);
    p->size |= HAS_NEXT_MASK;

    m_footer_p p_f = get_footer(p);
    p_f->size = p->size;

    m_footer_p new_f = get_footer(new);
    new_f->size = new->size;

    List *list = free_list[IDX(SIZE(new))];

    new->next = list->head->next;
    new->prev = list->head;

    list->head->next->prev = new;
    list->head->next = new;
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