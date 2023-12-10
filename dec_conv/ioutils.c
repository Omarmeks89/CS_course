#include <stdio.h>
#include <unistd.h>

#include "ioutils.h"
#include "errors.h"

/* Write result to stdout.
 * Params:
 *      char *src:      array of bytes we write;
 *      int max_pos:    bytes count;
 *      int *err:       ptr to error variable (for set).
 * (void) */
void
print_result(char *src, int max_pos, int *err) {
    ssize_t res;
    res = write(s_stdout, src, (ssize_t)max_pos);
    if ((res < 0) || (res < (ssize_t)max_pos))
        *err = E_STDOUT;
    *err = E_NOERR;
}

