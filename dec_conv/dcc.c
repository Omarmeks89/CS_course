#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dcc.h"
#include "sysutils.h"
#include "errors.h"
#include "ioutils.h"

enum sys_base {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16,
};

/* Array of supported presets, order of presets
 * is important, bcs we use it`s index to choose 
 * operation. May be that behaviour will be change later. */
static const char *presets[] = {
    "prec",
};

/* Indexes (codes) of supported presets
 * to choose operation. */
enum preset_op {
    SET_PREC,
};

enum sign {
    SIGNED = 0,
    UNSIGNED,
};

enum ntype_t {
    INT = 0,
    DOUBLE,
};

enum repr_mode {
    CUPPER = 0,
    CLOWER,
};

/* Representation of number:
 *      ntype: INT | DOUBLE;
 *      sppos: position of separator symb ('.');
 *      err:   err-value;
 *      size:  count of symbols in number. */
struct report_t {
    int ntype;
    int sppos;      /* position of separation symbol */
    int err;
    int size;
};

struct command_t {
    char sys;       /* b[B] | o[O] | h[H] */
    char repr_m;    /* l[L] lowercase, uppercase by default */
    int prec;       /* precision for float numbers */
    int err;
    int num_pos;
};

static int check_sign(char *num);
static char *get_ctrl_num(int sign);
static char *skip_sign(char *num);
static int check_overflow(char *num, char *c_num, int lim, int size);
static int detect_num_type(char *num, struct report_t *report);
static int check_num(char *num, struct report_t *rep, char *c_num, int ovf_lim);
static int del_report(struct report_t *r);
static int parse_float_precision(const char *arg, struct command_t *cmd, int mth_pos);
static int match_p(const char *arg, const char *templ);
static int parse_preset(const char *arg, void *cmd, const char *preset[]);
static int convert_mode(const char *mode, void *cmd, const char *presets[]);
int parse_command(int argc, char *argv[], void *cmd, const char *presets[]);                /*move to parser.c / .h */
static int del_command(struct command_t *cmd);
static char decode_symb(num_ul64 dig, char repr_m);
static void set_num_prefix(char *dest, char *pref);
static void convert(num_ul64 num, num_ul64 base, char *dest, int *l_pos, char repr_m);
static void reverse(char *dest, int *l_pos);
static void convert_float_part(double tail, char *dest, double base, int prec, char repr_m);
static void merge_parts(const char *head, const char *tail, char *dest);

/* Return sign of number.
 * If we found [-] we have signed number,
 * else we have unsigned number.
 * Params:
 *      char *num:      1st symbol in fetched string;
 * Return:
 *      int (error code) or int (sign).
 * 
 * Always need to check returned value on error. */
static int
check_sign(char *num) {
    switch(num[0]) {
        case '-':
            return SIGNED;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
            return UNSIGNED;
        default:
            return E_INV_SIGN;
    }
}

/* Return max number depend on fetched digit.
 * For unsigned -> 2^64-1, for signed -> 2^63-1.
 * Params:
 *      int sign:   1 as unsigned number, 0 as signed number;
 * Return:
 *      char*       string, that represents control number
 *                  for overflow check. */
static char*
get_ctrl_num(int sign) {
    if (sign == 1) {
        return SYS_UNS_OVF;
    }
    return SYS_SIGN_OVF;
}

/* If sign [+] | [-] found, skip it - 
 * return string without sign.
 * Params:
 *      char *num:  ptr to fetched string;
 * Return:
 *      char *num:  ptr, shifted no next symbol
 *                  if sign symbol [-] | [+] was
 *                  found. */
static char*
skip_sign(char *num) {
    char *only_digs = num;
    if ((num[0] == '-') || (num[0] == '+'))
        only_digs = ++num;
    return only_digs;
}

/* Check that fetched number is lower or equal to
 * control number (in case both sizes are equal).
 * Params:
 *      char *num:      fetched num without sign (if it was);
 *      char *c_num:    control number (2^64-1 or 2^63-1) to
 *                      compare with fetched num;
 *      int lim:        fetched num symbols count;
 *      int size:       control num symbols count.
 * Return:
 *      int (error code). Have to check returned value. */
static int
check_overflow(char *num, char *c_num, int lim, int size) {
    for(int i = 0; i < lim; i++) {
        if ((num[i] > c_num[i]) && (lim == size))
            return E_OVF;
    }
    return E_NOERR;
}

/* Detect type of num (float or not).
 * If separator ('.') found, set as float number,
 * else set as INT.
 * Params:
 *      char *num:                  fetched num (as a string);
 *      struct report_t *report:    ptr to report struct
 *                                  which we`ll set.
 * Return:
 *      int (error code). Have to check returned value. */
static int
detect_num_type(char *num, struct report_t *report) {
    int i;
    report->ntype = INT;
    report->sppos = 0;
    for(i = 0; num[i]; i++) {
        if (((num[i] < '0') || (num[i] > '9')) &&
                ((num[i] != ssep) && (num[i]))) {
            return E_UNSUPP;
        }
        if (num[i] == ssep) {

            /* size of non-float part x < i */
            report->sppos = i;
            report->ntype = DOUBLE;
        }
    }
    report->size = i;
    return E_NOERR;
}

/* Compare fetched num with control num
 * and set report.
 * Params:
 *      char *num:                  fetched num as string;
 *      struct report_t *rep:       empty report struct;
 *      char *c_num:                control num depend on 
 *                                      fetched num type (signed or unsigned).
 *      int ovf_lim:                limit len (count of chars).
 * Return:
 *      int (error code). Have to check returned value. */
static int
check_num(char *num, struct report_t *rep, char *c_num, int ovf_lim) {
    int err = E_NOERR;
    if (rep == NULL) 
        return E_NULLPT;
    err = detect_num_type(num, rep);
    if (err == E_NOERR) {
        if (rep->ntype == DOUBLE) {

            /* decrement means shift from '\0' to prev symb */
            if ((!c_num[rep->sppos - 1]) || (rep->sppos > ovf_lim)) 
                err = E_OVF;
            else
                err = check_overflow(num, c_num, rep->sppos, rep->size);
        } else {
            if ((!c_num[rep->size - 1]) || (rep->sppos > ovf_lim))
                err = E_OVF;
            else
                err = check_overflow(num, c_num, rep->size, ovf_lim);
        }
    }
    rep->err = err;
    return err;
}

/* Free memory from report.
 * Check fetched ptr on NULL-value, if NULL
 * error code will be returned.
 * Params:
 *      struct report_t *r:     ptr to report struct.
 * Return:
 *      int (error code). Have to check returned value. */
static int
del_report(struct report_t *r) {
    if (r == NULL) 
        return E_NULLPT;
    free(r);
    return E_NOERR;
}

/* Parse fetched precision parameter.
 * By default precision set as 10. Max value
 * 32. If fetched precision is bigger that 32,
 * it will be set as 32.
 * Params:
 *      const char *arg:        presision preset;
 *      struct command_t *cmd:  ptr to command that we will set;
 *      int mth_pos:            position of 1st digit in presicion 
 *                              preset.
 * Return:
 *      int (error code). Have to check returned value. */
static int
parse_float_precision(const char *arg, struct command_t *cmd, int mth_pos) {
    /* we`ll decorate this func with top-level func
     * for cast void* to struct command_t* and back to void*. */
    int p_prec = PRECISION;
    char pvalue[] = {'\0', '\0', '\0'};
    for(int i = 0; arg[mth_pos + i]; i++) {
        if ((arg[mth_pos + i] < '0') || (arg[mth_pos + i] > '9'))
            return E_UNSUPP;
        pvalue[i] = arg[mth_pos + i];
        if (i == PREC_NUMS_CNT)
            break;
    }
    p_prec = atoi(pvalue);
    if (p_prec > MAX_PREC) 
        p_prec = MAX_PREC;
    cmd->prec = p_prec;
    return E_NOERR;
}

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
                err = parse_float_precision(arg, cmd, match + 1);
                break;
        }
    }
    if (!match)
        err = E_NOMTHP;
    return err;
}

/* Compare and set fetched flags and presets
 * to command struct.
 * Params:
 *      const char *mode:       flag | preset;
 *      struct command_t *cmd:  command which we`ll set;
 *      const char *presets[]:  system-defined preset symbols.
 * Return:
 *      int (error code). Have ti check returned value. */
static int
convert_mode(const char *mode, void *cmd, const char *presets[]) {
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
                /* TODO rename func <conver_mode>. */
                err = convert_mode(arg, cmd, presets);
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

/* Free memory from command.
 * Check fetched ptr on NULL, return error code
 * if ptr is NULL.
 * Params:
 *      struct command_t *cmd:  ptr to command struct.
 * Return:
 *      int (error code). Have to check returned value. */
static int
del_command(struct command_t *cmd) {
    if (cmd == NULL)
        return E_NULLPT;
    free(cmd);
    return E_NOERR;
}

/* Convert fetched digit to char.
 * Params:
 *      num_ul64 dig:      digit we`ll convert;
 *      char repr_m:    representation mode (make sence for hex)
 *                      for convertation symbols to lower case.
 * Return:
 *      char:           return symbol. */
static char
decode_symb(num_ul64 dig, char repr_m) {
    char c;
    char upp = (32 * repr_m);
    switch(dig) {
        case 0: { c = '0'; break; }
        case 1: { c = '1'; break; }
        case 2: { c = '2'; break; }
        case 3: { c = '3'; break; }
        case 4: { c = '4'; break; }
        case 5: { c = '5'; break; }
        case 6: { c = '6'; break; }
        case 7: { c = '7'; break; }
        case 8: { c = '8'; break; }
        case 9: { c = '9'; break; }
        case 10: { c = ('A' + upp); break; }
        case 11: { c = ('B' + upp); break; }
        case 12: { c = ('C' + upp); break; }
        case 13: { c = ('D' + upp); break; }
        case 14: { c = ('E' + upp); break; }
        case 15: { c = ('F' + upp); break; }
    }
    return c;
}

 /* Add prefix: Ob | 0x | 0o to template.
  * Params:
  *     char *dest:     allocated mem for new string;
  *     char *pref:     num-system prefix (2 symbols).
  * (void) */
static void
set_num_prefix(char *dest, char *pref) {
    for(; *pref; ) 
        *dest++ = *pref++;
}

/* Convert decimal number representation
 * to wished system.
 * Params:
 *      num_ul64 num:      unsigned long number (converted from fetched string);
 *      num_ul64 base:     base of system (2, 8, 16);
 *      char *dest:     memory for converted num symbols;
 *      int *l_pos:     as symbols border;
 *      char repr_m:    representation mode (upper or lower case).
 * (void) */
static void
convert(num_ul64 num, num_ul64 base, char *dest, int *l_pos, char repr_m) {
    num_ul64 symb_no;
    int i;
    for(i = 2; num >= base; i++) {
        symb_no = num % base;
        dest[i] = decode_symb(symb_no, repr_m);
        num = (num_ul64)num / base;
    }
    dest[i] = decode_symb(num, repr_m);
    *l_pos = i;
}

/* Reverse converted string.
 * (at place)
 * Params:
 *      char *dest:     array with converted string
 *                      symbols;
 *      int *l_pos:     position of last (non '\0') digit.
 * (void) */
static void
reverse(char *dest, int *l_pos) {
    int i, j;
    char c;
    for(i = 2, j = *l_pos; i < j; i++, j--) {
        c = dest[j];
        dest[j] = dest[i];
        dest[i] = c;
    }
    dest[*l_pos + 1] = '\n';

     /* Add tail for pair '\0\n' */
    *l_pos = ADD_TAIL(*l_pos);
}

/* Resize float / double number to
 * wished precision.
 * Params:
 *      char *fl_part:      float part of fetched number;
 *      char *dest:         destination memory;
 *      int prec:           wished precision (v.0.1.0 symbols count
 *                          only. Number will not rounded, only shorted).
 * (void) */
static void
resize_double(char *fl_part, char *dest, int prec) {
    *fl_part = '0';
    for(; *fl_part; dest++, fl_part++) {
        prec--;
        if (prec <= 0)
            break;
        *dest = *fl_part;
    }
}

/* Convert float part of number to
 * wished system (bin | oct | hex).
 * Params:
 *      double tail:        float part converted to double;
 *      double base:        num system base (2.0, 8.0, 16.0);
 *      int prec:           wished prexision of float part;
 *      char repr_m:        representation mode (make sence for hex)
 *                          convert literal hex symbols to lower case.
 * (void) */
static void
convert_float_part(double tail, char *dest, double base, int prec, char repr_m) {
    int tmp_t;
    for(int i = 0; i < MAX_PREC; i++) {
        tail = tail * base;
        tmp_t = (int)tail;
        dest[i] = decode_symb(tmp_t, repr_m);
        tail -= (double)tmp_t;
    }
    dest[prec] = '\0';
}

/* Compile head part (before separator) and tail part (float part)
 * together.
 * Params:
 *      const char *head:       symbols before separator;
 *      const char *tail:       symbols after separator;
 *      char *dest:             new block of memory:
 *                                  size(pref) + size(head) + size(tail) + size('\0\n').
 * (void) */
static void
merge_parts(const char *head, const char *tail, char *dest) {
    for(; *head; head++, dest++ ) {
        if (*head == '\n')
            break;
        *dest = *head;
    }
    if (*tail)
        *dest = ssep;
    dest++;
    for(; *tail; dest++, tail++ )
        *dest = *tail;
    *dest = '\n';
}

/* create new (alloc mem for new) report struct
 * Params:
 *      int *errno: ptr to int value to store errno.
 * Return:
 *      struct report_t*: pointer to new empty struct. */
struct report_t*
new_report(int *errno) {
    struct report_t *report = NULL;
    *errno = E_NOERR;
    report = malloc(sizeof(*report));
    if (report == NULL)
        *errno = E_NULLPT;
    return report;
}

/* convert raw num from argv and (bad sign) fill
 * report.
 * Params:
 *      char *num:                      num as string from argv;
 *      struct report_t *report:        struct report for control 
 *                                          validity.
 * Return:
 *      int: errno for checking. */
int
fetch_num(char *num, struct report_t *report) {
    int sign = 0;
    sign = check_sign(num);
    if (sign == E_INV_SIGN)  
        return sign;
    *num = *skip_sign(num);
    return check_num(num, report, get_ctrl_num(sign), OVF_SIZEOF(sign));
}

/* set parameters for the next operations with a number.
 * Params:
 *      char sys:       bin | oct | hex;
 *      int *symbols:   ptr to int container for symbols limit;
 *      char *pref:     prefix like '0o', '0x', '0b';
 *      num_ul64 *base: base of current system. */
static void
get_num_system_credentials(char sys, int *symbols, char *pref, num_ul64 *base) {
    switch(sys) {
        case 'b':
            *symbols = BIN_SYMBS_CNT();
            *pref = *(char*)"0b";
            *base = (num_ul64)BIN;
            break;
        case 'h':
            *symbols = HEX_SYMBS_CNT();
            *pref = *(char*)"0x";
            *base = (num_ul64)HEX;
            break;
        case 'o':
            *symbols = OCT_SYMBS_CNT();
            *pref = *(char*)"0o";
            *base = (num_ul64)OCT;
            break;
    }
}

int
compile_number(char *num, struct report_t *report, struct command_t *cmd) {
    int symbols = 0, max_pos = 0;
    char *pref = NULL, *result = NULL, *tail = NULL, *merged = NULL;
    num_ul64 base = 0, converted_num = 0;
    double f_part = 0.0;

    get_num_system_credentials(cmd->sys, &symbols, pref, &base);

    if (report->ntype == INT) {
        symbols = ADD_PREF_AND_SUFF(symbols);
    } else {
        symbols = ADD_TAIL(symbols) + 1;
    }
    result = create_string(symbols);

    if (report->ntype == DOUBLE) {
        /* we alloc mem for max precision */
        tail = create_string(ADD_TAIL(MAX_PREC));
        resize_double(&num[report->sppos - 1], tail, MAX_PREC);
        f_part = atof(tail);
        convert_float_part(f_part, tail, (double)base, cmd->prec, cmd->repr_m);
    }
    converted_num = strtoul(num, NULL, DEC);

    set_num_prefix(result, pref);
    convert(converted_num, base, result, &max_pos, cmd->repr_m);
    reverse(result, &max_pos);

    if (report->ntype == DOUBLE) {
        /**
         * We create new string here of
         * size = size(head) + size(tail) + size(pref + '.', '\0', '\n')
         */
        max_pos = ADD_TAIL(symbols) + ADD_TAIL(cmd->prec);
        merged = create_string(max_pos);
        merge_parts(result, tail, merged);
        free(merged);
        free(tail);
    }
    free(result);
    return 0;
}

/* compile all parts of double num representation (head & tail)
 * into a one array (dest_buffer).
 * Params:
 *      */
int
compile_parts_to_double(struct command_t *cmd, num_ul64 num, char *pref, double base,
                char *head, char *tail, char *dest_buffer) {
    int max_pos = 0;
    double f_part = 0.0;
    if (
            (cmd == NULL) || (pref == NULL) || (head == NULL) ||
            (tail == NULL) || (dest_buffer == NULL) 
       )
        return E_NULLPT;
    f_part = atof(tail);
    convert_float_part(f_part, tail, base, cmd->prec, cmd->repr_m);
    set_num_prefix(head, pref);
    convert(num, base, head, &max_pos, cmd->repr_m);
    reverse(head, &max_pos);
    merge_parts(head, tail, dest_buffer);
    return E_NOERR;
}

int
compile_parts_to_int(struct command_t *cmd, num_ul64 num, char *pref, double base,
                    char *strnumber, char *dest_buffer) {
    int max_pos = 0, err = E_NOERR;
    if ((cmd == NULL) || (pref == NULL) || (strnumber == NULL) || (dest_buffer == NULL))
        return E_NULLPT;
    set_num_prefix(strnumber, pref);
    convert(num, base, strnumber, &max_pos, cmd->repr_m);
    reverse(strnumber, &max_pos);
    err = copy_to(strnumber, dest_buffer);
    return err;
}
