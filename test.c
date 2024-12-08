#include "malloc.h"
#include <stdio.h>
#include <assert.h>

int main() {
    init();
    
    // Basic allocation test
    void *p1 = xmalloc(16);
    void *p2 = xmalloc(32);
    void *p3 = xmalloc(64);

    // Free and reallocation test
    xfree(p2);
    void *p4 = xmalloc(32);
    assert(p4 == p2);  // Should reuse the same block

    // Multiple free and coalescing test
    xfree(p1);
    xfree(p3);
    xfree(p4);

    // Large allocation after coalescing
    void *p5 = xmalloc(112);
    assert(p5 == p1);  // Should reuse the coalesced block

    printf("All tests passed.\n");
    return 0;
}