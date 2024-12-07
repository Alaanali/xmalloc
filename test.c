#include "malloc.h"
#include <stdio.h>
#include <assert.h>

int main() {
    init();
    
    // Basic allocation test
    void *p1 = my_malloc(16);
    void *p2 = my_malloc(32);
    void *p3 = my_malloc(64);

    // Free and reallocation test
    my_free(p2);
    void *p4 = my_malloc(32);
    assert(p4 == p2);  // Should reuse the same block

    // Multiple free and coalescing test
    my_free(p1);
    my_free(p3);
    my_free(p4);

    // Large allocation after coalescing
    void *p5 = my_malloc(112);
    assert(p5 == p1);  // Should reuse the coalesced block

    printf("All tests passed.\n");
    return 0;
}