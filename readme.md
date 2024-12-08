# Custom Memory Allocator Implementation

A custom implementation of malloc and free functions providing dynamic memory allocation with efficient memory management features including splitting, coalescing, and segregated free lists.

## Features

- Custom `malloc` and `free` implementation
- Memory alignment to 8-byte boundaries
- Segregated free lists for different block sizes
- Memory coalescing to prevent fragmentation
- Block splitting to minimize waste
- Header/footer block format for efficient coalescing
- Boundary tag implementation for constant-time coalescing

## Architecture

### Memory Block Structure

Each allocated block contains:
- Header (contains size and status flags)
- Payload (actual user data)
- Footer (contains size and status flags)

### Memory Block Flags

- `FREE_MASK`: Indicates if the block is free
- `HAS_PREV_MASK`: Indicates if there's a previous block
- `HAS_NEXT_MASK`: Indicates if there's a next block
- `SIZE_MASK`: Mask for extracting the actual size

### Project Structure

```
.
├── malloc.h           # Public interface
├── malloc_internal.h  # Internal structures and definitions
├── malloc.c          # Main implementation
├── malloc_utils.c    # Utility functions
├── malloc_chunk.c    # Chunk management functions
├── test.c           # Test suite
└── Makefile         # Build configuration
```

## Building

```bash
# Build the project
make

# Clean build artifacts
make clean
```

## Usage

```c
#include "malloc.h"

int main() {
    // Initialize the allocator
    init();

    // Allocate memory
    void* ptr = xmalloc(size);

    // Free memory
    xfree(ptr);

    return 0;
}
```

## API Reference

### `void init(void)`
Initializes the memory allocator. Must be called before any other operations.

### `void* xmalloc(size_t size)`
Allocates a block of memory of the specified size.

**Parameters:**
- `size`: Number of bytes to allocate

**Returns:**
- Pointer to the allocated memory, or NULL if allocation fails

### `void xfree(void* ptr)`
Frees the memory block pointed to by ptr.

**Parameters:**
- `ptr`: Pointer to the memory block to free

## Implementation Details

### Memory Block Size Classes

The allocator uses segregated free lists with 8 size classes:
- Class 0: 8 bytes
- Class 1: 9-16 bytes
- Class 2: 17-32 bytes
- Class 3: 33-64 bytes
- Class 4: 65-128 bytes
- Class 5: 129-256 bytes
- Class 6: 257-512 bytes
- Class 7: 513+ bytes

### Memory Management Strategies

1. **Block Splitting**: When allocating a block that's larger than requested, the block is split if the remainder would be large enough for a new minimum-size block.

2. **Coalescing**: When freeing a block, the allocator checks adjacent blocks and merges them if they're free, preventing fragmentation.

3. **Best Fit**: Within each size class, the allocator uses a best-fit strategy to minimize wasted space.

## Testing

The test suite (`test.c`) includes tests for:
- Basic allocation and deallocation
- Block reuse
- Memory coalescing
- Block splitting
- Edge cases

Run the tests:
```bash
./malloc_test
```

## Performance Considerations

- O(1) free operation
- O(n) malloc operation where n is the number of free blocks in the appropriate size class
- Constant-time coalescing due to boundary tags
- Efficient memory reuse through segregated free lists
- Minimal overhead through optimized block headers

## Limitations

- Minimum allocation size of 8 bytes
- No support for realloc
- No thread safety (not thread-safe)
- No error handling for double frees

