#ifndef _SYS_STATE_ENTRY
#define _SYS_STATE_ENTRY

#ifdef __cplusplus
extern "C"
#endif

enum _state_errors {
    E_STACC = 1,  /* raised if we try get access to set active state (!=0) */
};

int __set_sstate();
int __sstate_check();

#endif
