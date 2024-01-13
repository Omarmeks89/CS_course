#include <stdlib.h>

#include "_sys_mem_init.h"
#include "_sys_state.h"

typedef unsigned short s_size;
typedef unsigned int i_size;

struct _mem_block_descriptor {
    s_size block_registry;  /* | 5 bits reserved | 11 bit - free / alloc | 10 bits size | */ 
    void *mem_array;
};

typedef struct _mem_block_descriptor *block_descr;

/* TODO need to round by 8 here. */
struct _mem_image {
    s_size pos;
    s_size free;
    s_size mem_limit;
    unsigned char f_idx;
    block_descr descriptors[_set_ptr_array_sz(DEF_PG_SIZE)];
    void *raw_mem_array;  /* DEF_PG_SIZE - sizeof(i_size) */
};

/* BSS segment setted always with zeroes. */
static struct _mem_image g_mem_image;
static void *_raw_mem;

static void
_set_mem_image(void *r_mem, struct _mem_image *img_ptr) {
    unsigned char offset = 0, rnd = 0;
    struct _mem_image *ptr = NULL;
    if (!(img_ptr == NULL)) {
        offset = (unsigned char)(rand() & 0xFF);
        if (offset % PTR_SZ)
            /* this means we have non zero value - have to round it. */
            offset += (PTR_SZ - ((unsigned char)(offset % PTR_SZ)));
        ptr = (struct _mem_image*)(r_mem + offset);
        rnd = (unsigned char)sizeof(*ptr);
        if (rnd % PTR_SZ)
            /* we set another offset from 0 bit of struct. */
            rnd += (PTR_SZ - ((unsigned char)(rnd % PTR_SZ)));
        ptr->raw_mem_array = (void*)(r_mem + (offset + rnd));
        ptr->pos = 0;
        ptr->f_idx = 0;
        ptr->free = MAX_BLOCK_SZ_BYTES();
        ptr->mem_limit = ptr->free;
        *img_ptr = *ptr;
    }
}

int _init_memory() {
    if ((__sstate_check()) || (__set_sstate()))
        /* this means our system is just set. */
        return E_MEMINI;
    _raw_mem = calloc(DEF_PG_SIZE, sizeof(char));
    if (_raw_mem == NULL)
        return E_MEMINI;
    _set_mem_image(_raw_mem, &g_mem_image);
    return 0;
}

/* Free global ptr on raw memory array. */
int _free_memory() {
    if (_raw_mem == NULL)
        return E_FREE;
    free(_raw_mem);
    return 0;
}

block_descr _set_new_mem_block(size_t size) {
    unsigned char offset = 0;
    block_descr block = NULL;
    if ((size > g_mem_image.mem_limit) || (size > g_mem_image.free) || (_raw_mem == NULL))
        return NULL;
    for (s_size i = 0; i < (g_mem_image.pos + 1); i++) {
        block = g_mem_image.descriptors[i];
        if (block != NULL) {
            s_size blk_sz = block->block_registry & 0x04FF;
            if (
                    (block->block_registry & ALLOC_BIT_FLG) &&
                    (blk_sz >= size)
               ) {
                block->block_registry ^= blk_sz;
                size += PTR_SZ - ((s_size)size % PTR_SZ);
                size = size > g_mem_image.mem_limit ? g_mem_image.mem_limit : size;
                block->block_registry |= (s_size)size;
                block->block_registry ^= ALLOC_BIT_FLG;
                return block;
            }
        }
    }
    block = (struct _mem_block_descriptor*)g_mem_image.raw_mem_array;
    g_mem_image.descriptors[g_mem_image.pos] = block;
    /* set size-bits into registry. */
    block->block_registry |= ((s_size)size & 0x04FF);
    offset = sizeof(*block);
    offset += PTR_SZ - ((unsigned char)offset % PTR_SZ);
    block->mem_array = g_mem_image.raw_mem_array + offset;
    g_mem_image.free -= (s_size)size;
    g_mem_image.pos++;
    g_mem_image.raw_mem_array += offset;
    return block;
}

int
_mark_mem_block_as_free(int block_no) {
    block_descr block = NULL;
    if (_raw_mem == NULL)
        return E_NOMEM;
    if ((s_size)block_no > g_mem_image.pos)
        return E_MEMCRR;
    block = g_mem_image.descriptors[block_no];
    /* mark as free block. */
    block->block_registry |= ALLOC_BIT_FLG;
    g_mem_image.free += (block->block_registry & 0x04FF);
    return 0;
}
