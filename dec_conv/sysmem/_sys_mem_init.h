#ifndef _SYS_MEM_INIT_ENTRY
#define _SYS_MEM_INIT_ENTRY

#include <stddef.h>

#ifdef __cplusplus
extern "C"
#endif

#define DEF_PG_SIZE 2048

#if defined(x32)
#    define PTR_SZ 4
#else
#    define PTR_SZ 8
#endif
#define _set_ptr_array_sz(pg_size) ((unsigned char)(pg_size / 2) / PTR_SZ)

#define ALLOC_LIMIT_MASK 0x0400
#define ALLOC_TOP_BORDER 0x03FF
#define REGISTRY_MASK_SHIFT 0xA
#define ALLOC_BIT_FLG 0x0800
#define MAX_BLOCK_SZ_BYTES() ((unsigned short)DEF_PG_SIZE / 2) 

enum _sys_mem_error {
    E_MEMINI = 1,
    E_FREE,
    E_NOMEM,
    E_MEMDMS,
    E_MEMCRR               /* attemp to hack mem -> mem corrupted error. */
};

int _init_memory();
int _free_memory();
struct _mem_block_descriptor *_set_new_mem_block(size_t size);
int _mark_mem_block_as_free(int block_no);

#endif
