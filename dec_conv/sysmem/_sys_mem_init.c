#include <stdlib.h>

#include "_sys_mem_init.h"
#include "_sys_state.h"

typedef unsigned short s_size;
typedef unsigned int i_size;

/* Simple mem block described
 * stored object. */
struct _mem_block_descriptor {
    s_size block_size;
    s_size elem_size;
    s_size append_pos;
    void *mem_array;
};

typedef struct _mem_block_descriptor *block_descr;

/* TODO need to round by 8 here. */
#if defined(DEF_PG_SIZE)
#    define __max_blocks_cnt _set_ptr_array_sz()
struct _mem_image {
    s_size pos;
    s_size free;
    s_size mem_limit;
    block_descr descriptors[__max_blocks_cnt];
    void *raw_mem_array;
};
#else
#    error memory size not specified. Use -D DEF_PG_SIZE
#endif

enum _memblock_state {
    EMPTY = 0,
    FREE,
    ALLOCATE,
};

/* BSS segment setted always with zeroes. */
static struct _mem_image g_mem_image;
static void *_raw_mem;
#if defined(__max_blocks_cnt)
static int blocks_tbl[__max_blocks_cnt];
static const s_size blocks_limit = __max_blocks_cnt;
#    undef __max_blocks_cnt
#endif

int *
get_free_memsize() {
    if (_raw_mem == NULL)
        return NULL;
    return (int*)&g_mem_image.free;
}

int *
get_memlimit() {
    if (_raw_mem == NULL)
        return NULL;
    return (int*)&g_mem_image.mem_limit;
}

static void
_set_mem_image(void *r_mem, struct _mem_image *img_ptr) {
    unsigned char offset = 0, rnd = 0;
    struct _mem_image *ptr = NULL;
    if (!(img_ptr == NULL)) {
        offset = (unsigned char)(rand() & 0xFF);
        if (offset % PTR_SZ)
            /* this means we have non zero value - have to round it. */
            offset += (PTR_SZ - (offset % PTR_SZ));
        ptr = (struct _mem_image*)(r_mem + offset);
        rnd = (unsigned char)sizeof(*ptr);
        if (rnd % PTR_SZ)
            /* we set another offset from 0 bit of struct. */
            rnd += (PTR_SZ - (rnd % PTR_SZ));
        ptr->raw_mem_array = (void*)(r_mem + (offset + rnd));
        ptr->pos = 0;
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
    for (int i = 0; i < (blocks_limit + 1); i++) {
        blocks_tbl[i] = EMPTY;
    }
    g_mem_image.free = 0;
    free(_raw_mem);
    _raw_mem = NULL;
    return 0;
}

static s_size
_align_size(size_t size) {
    s_size aligned = 0;
    aligned += (PTR_SZ - ((s_size)size % PTR_SZ));
    return aligned > g_mem_image.mem_limit ? g_mem_image.mem_limit : aligned;
}

block_descr
_get_memblock(int block_no) {
    if (
            (block_no >= 0) ||
            (block_no <= blocks_limit) ||
            (blocks_tbl[block_no] == ALLOCATE)
        )
        return g_mem_image.descriptors[block_no];
    return NULL;
}

block_descr _set_new_mem_block(size_t size) {
    unsigned char offset = 0;
    block_descr block = NULL;
    s_size new_block_sz = 0;

    if (g_mem_image.pos + 1 > blocks_limit)
        return NULL;
    if ((size > g_mem_image.mem_limit) || (size > g_mem_image.free) || (_raw_mem == NULL))
        return NULL;
    new_block_sz = _align_size(size);
    for (s_size i = 0; i < (g_mem_image.pos + 1); i++) {
        block = g_mem_image.descriptors[i];
        if (block != NULL) {
            if (
                    (blocks_tbl[i] == FREE) &&
                    (block->block_size >= new_block_sz)
               ) {
                block->block_size = new_block_sz;
                blocks_tbl[i] = ALLOCATE;
                return block;
            }
        }
    }
    block = (struct _mem_block_descriptor*)g_mem_image.raw_mem_array;
    g_mem_image.descriptors[g_mem_image.pos] = block;
    block->block_size = new_block_sz;
    offset = sizeof(*block);
    offset += (PTR_SZ - (offset % PTR_SZ));
    block->mem_array = g_mem_image.raw_mem_array + offset;
    blocks_tbl[g_mem_image.pos] = ALLOCATE;
    g_mem_image.free -= new_block_sz;
    g_mem_image.pos++;
    g_mem_image.raw_mem_array += offset;
    return block;
}

int
_mark_mem_block_as_free(int block_no) {
    block_descr block = NULL;
    if (_raw_mem == NULL)
        return E_NOMEM;
    if ((block_no < 0) || (block_no > blocks_limit) || (blocks_tbl[block_no] == EMPTY))
        return E_MEMCRR;
    if (blocks_tbl[block_no] == FREE)
        return E_FREE;
    block = g_mem_image.descriptors[block_no];
    g_mem_image.free += block->block_size;
    blocks_tbl[block_no] = FREE;
    return 0;
}
