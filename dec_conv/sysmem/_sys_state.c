#include "_sys_state.h"

/* we`ll change value, so we shouldn`t 
 * try to optimize it. */
static volatile unsigned char _state = 0;

/* Set initial state as active.
 * I wish i`ll add mutex here to avoid
 * multiple access to that global var. */
int
__set_sstate() {
    if (!_state) {
        _state = (char)1;
        return 0;
    }
    return E_STACC;
}

/* return current system state. */
int 
__sstate_check() {
    return _state;
}
