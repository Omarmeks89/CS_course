#include <stddef.h>

#include "parser.h"
#include "errors.h"
#include "dcc.h"

/* Indexes (codes) of supported presets
 * to choose operation. */
enum preset_op {
    SET_PREC,
};


/* Compare fethed arg with preset from presets[].
 * If preset matched, return last matched position.
 * Params:
 *      const char *arg:        fetched preset;
 *      const char *templ:      template from presets[].
 * Return:
 *      int (last matched position). */
static int
match_p(const char *arg, const char *templ) {
    const char *tmp = arg;
    for(; *templ; templ++, arg++) {
        if (*templ != *arg)
            return 0;
    }
    return arg - tmp;
}

/* Parse fetched preset and compare with system-defined.
 * Params:
 *      const char *arg:        preset like --[preset]=[value];
 *      struct command_t *cmd:  ptr to command we will set;
 *      const char *preset[]:   array of system-defined presets.
 * Return:
 *      int (error code). Have to check returned value. */
static int
parse_preset(const char *arg, void *cmd, const char *preset[]) {
    int match = 0, err = E_NOERR;
    for(int i = 0; preset[i]; i++) {
        match = match_p(arg, preset[i]);
        if (!match)
            continue;
        switch(i) {
            case SET_PREC:
                /* There is comcrete command-handlers here. */
                err = parse_float_precision(arg, cmd, match + 1);
                break;
        }
    }
    if (!match)
        err = E_NOMTHP;
    return err;
}

/* Compare and set fetched flags and presets
 * to command struct. Each module represents
 * it`s own command and setup-functions to
 * correctly setup a command.
 * Params:
 *      const char *mode:       flag | preset;
 *      struct command_t *cmd:  command which we`ll set;
 *      const char *presets[]:  system-defined preset symbols.
 * Return:
 *      int (error code). Have ti check returned value. */
static int
handle_arg(const char *mode, void *cmd, const char *presets[]) {
    const char *f = ++mode;
    char to_lover;
    char diff = 'a' - 'A';
    for(; *f; f++ ) {
        switch(*f) {
            case 'B':
            case 'H':
            case 'O':
                to_lover = (char)(*f + diff);
                /* TODO create concrete func for each of parameters */
                cmd->sys = to_lover;
                break;
            case 'L':
                cmd->repr_m = CLOWER;
                break;
            case 'b':
            case 'h':
            case 'o':
                cmd->sys = *f;
                break;
            case 'l':
                cmd->repr_m = CLOWER;
                break;
            case '-':
                return parse_preset(++f, cmd, presets);
            default:
                cmd->sys = *f;
                return E_INV_SYS;
        }
    }
    return E_NOERR;
}

/* Parse fetched commmand.
 * Params:
 *      int argc:               cli-args count;
 *      char *argv[]:           array of flags and presets from cli;
 *      const char *presets[]:  array of system-defined presets
 *                              to compare witch fetched.
 * Return:
 *      struct command_t *ptr:  pointer to command struct. */
int
parse_command(int argc, char *argv[], void *cmd, const char *presets[]) {
    int err = E_NOERR;
    char *arg;
    if (cmd == NULL) {
        return E_NULLPT;
    }
    /* need macro over cmd->num_pos, we shouldn`t know about cmd here. */
    for(int i = 1; i < (argc - 1); i++) {
        arg = argv[i];
        switch(arg[0]) {
            case '-':
                err = handle_arg(arg, cmd, presets);
                break;
            default:
                err = E_INVFLG;
                break;
        }
        if (err != E_NOERR) {
            break;
        }
    }
     /* check if no one of supported
     * systems set. */
    /*
    if (cmd->sys == NO_SYS)
        cmd->err = E_INV_SYS;
        */
    return err;
}
