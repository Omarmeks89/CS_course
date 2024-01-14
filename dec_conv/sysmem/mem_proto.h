#ifndef _MEM_PROTO_ENYRY
#define _MEM_PROTO_ENYRY

#include <stddef.h>

#ifdef __cplusplus
extern "C"
#endif

/* fetch allocated system mem for
 * using inside a user function.
 * Args:
 *     size_t size - wished block size 128 - 1024 bytes;
 * Return:
 *     int memblock_no - index inside a block descr page.  
 *     int errno - if was returned negative value. */
int get_memblock(size_t size);
int free_memblock(int memblock_no);
/* Put sym by sym inside a system memory area.
 * Params:
 *     int memblock_no - returned number of memblock;
 *     char symb - sym you wish to write in memory;
 * Return:
 *     int errno - write error if memblock ended
 *                 or was corrupted or not allocated. */
int putsym(int memblock_no, char symb);

#endif
