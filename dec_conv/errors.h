#ifndef ERRORS_ENTRY
#define ERRORS_ENTRY

#ifdef __cplusplus
extern "C"
#endif

/* Description of application errors */
enum dc_errors {
    E_NOERR     = 0,    /* supported [-] | [+] (or digit) only .*/
    E_INV_SIGN  = -1,
    E_OVF       = -2,
    E_INV_SYS   = -3,   /* different from -b[B] | -h[H] | -o[O] */
    E_UNSUPP    = -4,
    E_STDOUT    = -5,
    E_NULLPT    = -6,   /* we got NULL ar ptr in func */
    E_INVFLG    = -7,
    /* E_PRECONV (by num -8) was deleted */
    E_UNKPRES   = -9,   /* unknown preset */
    E_NOMTHP    = -10,
};


#endif
