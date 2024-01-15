#ifndef _SYS_MEM_INIT_ENTRY
#define _SYS_MEM_INIT_ENTRY

#include <stddef.h>

#ifdef __cplusplus
extern "C"
#endif

#if defined(DEF_PG_SIZE) && DEF_PG_SIZE > 8048
#    error out of memlimit = 4096
#elif defined(DEF_PG_SIZE) && DEF_PG_SIZE <= 0
#    define DEF_PG_SIZE 1024
#else
#    define DEF_PG_SIZE 2048
#endif

#if defined(x32)
#    define PTR_SZ 4
#else
#    define PTR_SZ 8
#endif
#define ALIGNED_MEM_PAGE ((unsigned int)DEF_PG_SIZE + (PTR_SZ - (DEF_PG_SIZE % PTR_SZ)))
#define _set_ptr_array_sz() ((unsigned char)(ALIGNED_MEM_PAGE / 2) / PTR_SZ)

#define ALLOC_LIMIT_MASK 0x0400
#define LIMIT_RANGE_BITS 0x04FF
#define ALLOC_BIT_FLG 0x0800
#define MAX_BLOCK_SZ_BYTES() ((unsigned short)ALIGNED_MEM_PAGE / 2) 

enum _sys_mem_error {
    E_MEMINI = 1,
    E_FREE,
    E_NOMEM,
    E_MEMDMS,
    E_MEMCRR               /* attemp to hack mem -> mem corrupted error. */
};

int _init_memory();
int _free_memory();
struct _mem_block_descriptor *_set_new_memblock(size_t size);
int _mark_memblock_as_free(int block_no);
int get_free_memsize();
int get_memlimit();
struct _mem_block_descriptor *_get_memblock(int block_no);

#endif
