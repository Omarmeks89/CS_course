#include <stdlib.h>

#include "_sys_mem_init.h"
#include "_sys_state.h"

typedef unsigned short s_size;
typedef unsigned int i_size;

struct _mem_block_descriptor {
    s_size block_registry;  /* | 5 bits reserved | 11 bit - free / alloc | 10 bits size | */ 
    void *mem_array;
};

/* TODO need to round by 8 here. */
struct _mem_image {
    i_size pg_registry;  /* | 2 bits - reserved | 10 bits - curr ptr pos | 10 free | 10 limit | */
    unsigned char block_ptrs_size;  /* 256 bytes */
    void *raw_mem_array;  /* DEF_PG_SIZE - sizeof(i_size) */
};
