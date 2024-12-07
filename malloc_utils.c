#include "malloc_internal.h"
#include <unistd.h>

m_footer_p get_footer(m_header_p h) {
    char *f = (char *)h;
    f += SIZE(h) + META_HEADER_SIZE;
    return (m_footer_p)f;
}

m_header_p get_header(m_footer_p f) {
    char *h = (char *)f;
    h -= SIZE(f) + META_HEADER_SIZE;
    return (m_header_p)h;
}

m_header_p get_next_chunk(m_header_p p) {
    if (((char *)p >= end) || !HAS_NEXT(p))
        return NULL;

    char *n = (char *)p;
    n += META_HEADER_SIZE + SIZE(p) + META_FOOTER_SIZE;
    return (m_header_p)n;
}

m_header_p get_prev_chunk(m_header_p p) {
    if (((char *)p <= start) || !HAS_PREV(p))
        return NULL;
    m_footer_p f = (m_footer_p)p - 1;
    char *n = (char *)p;
    n -= META_FOOTER_SIZE + SIZE(f) + META_HEADER_SIZE;
    return (m_header_p)n;
}

