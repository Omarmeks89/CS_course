#include <stdlib.h>

#include "sysutils.h"
#include "errors.h"

/* Allocate memory for string.
 * calloc uses because it fill memory zeroes,
 * it`s important bsc we check zero value in
 * other operations.
 * Params:
 *      int size:   wished size of string.
 * Return:
 *      char *ptr:  ptr to allocated memory. */
char*
create_string(int size) {
    char *ptr;
    ptr = (char*)calloc((size_t)size, (size_t)sizeof(char));
    return ptr;
}

/* copy data from source to dest buffer
 * Params:
 *      char *src:      data buffer of size x;
 *      char *dest:     empty buffer of size y = x.
 * Return:
 *      int errno:      if dest size < src size return
 *                          E_OVF as dest overflow. */
int
copy_to(char *src, char *dest) {
    char *st_src = src, *st_dest = dest;
    for (; (*src && *dest); ) {
        /* we avoid undefined operation order. */
        *dest = *src;
        dest++;
        src++;
    }
    if ((src - st_src) > (dest - st_dest))
        return E_OVF;
    return E_NOERR;
}

