#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ssep
#define ssep '.'
#endif

/**
 * We should define max output size (in symbols)
 * and define overflow value:
 *     max positive: 2^64-1 18 446 744 073 709 551 615
 *     max negative: 2^63 9 223 372 036 854 775 808
 */
#define SYS_UNS_OVF "18446744073709551615"
#define SYS_SIGN_OVF "9223372036854775808"
#define PRECISION 10
#define PREC_NUMS_CNT 2
#define MAX_PREC 32

#define OVF_SIZEOF(s) (s == 1 ? sizeof(SYS_UNS_OVF) : sizeof(SYS_SIGN_OVF))
/* max str size for binary repr */
#define BIN_SYMBS_CNT() ((int)64)
/* same for hex and oct */
#define HEX_SYMBS_CNT() ((int)64 / 4)
#define OCT_SYMBS_CNT() ((int)(64 / 3) + 1)
/* prefix: 0b | 0x | 0o */
#define PREFIX 2
/* this case suffix means '\0' + '\n' */
#define SUFFIX 2

#define SIGN(x) (x < 1 ? '-' : '+')
#define ADD_PREF_AND_SUFF(x) (x + PREFIX + SUFFIX)

/* 1 -> for '\0', 1 -> '\n' */
#define ADD_TAIL(x) (x + 2)

/* macro for convertion into negative repr */
#define INVERSE(x) (x == '0' ? '1' : '0')
#define XOR(a, b) (a == b ? '0' : '1')
#define AND(a, b) ((a == '1') && (b == '1') ? '1' : '0')
#define OR(a, b) ((a == '0') && (b == '0') ? '0' : '1')
#define NAND(a, b) (INVERSE(AND(a, b)))

typedef unsigned long num_t;

enum sys_base {
    BIN = 2,
    OCT = 8,
    DEC = 10,
    HEX = 16,
};

enum io_stream {
    s_stdout = 1,
    s_stderr = 2,
};

enum dc_errors {
    E_NOERR     = 0,
    /* supported: [-] | [+] (or digit) only. */
    E_INV_SIGN  = -1,
    E_OVF       = -2,
    /* means different from: -b[B] | -h[H] | -o[O]. */
    E_INV_SYS   = -3,
    E_UNSUPP    = -4,
    E_STDOUT    = -5,
    /* we got NULL as ptr in func */
    E_NULLPT    = -6,
    E_INVFLG    = -7,
    /* prec digs count > 2 */
    E_PRECOVF   = -8,
    /* unknown preset */
    E_UNKPRES   = -9,
    E_NOMTHP    = -10,
};

static const char *presets[] = {
    "prec",
};

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

struct report_t {
    int ntype;
    /* position of separation symbol */
    int sppos;
    int err;
    /**
     * number of digits after '.'
     * int-numbers will have 0 prec.
     */
    int size;
};

struct command_t {
    /* b[B] | o[O] | h[H] */
    char sys;
    /* l[L] lowercase, uppercase by default */
    char repr_m;
    /* precision for float numbers */
    int prec;
    int err;
    int num_pos;
};

int check_sign(char *num) {
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

char *get_ctrl_num(int sign) {
    if (sign == 1) {
        return SYS_UNS_OVF;
    }
    return SYS_SIGN_OVF;
}

/**
 * If sign [+] | [-] found, skip it - 
 * return string without sign.
 */
char *skip_sign(char *num) {
    char *only_digs = num;
    if ((num[0] == '-') || (num[0] == '+')) {
        only_digs = ++num;
    }
    return only_digs;
}

int check_overflow(char *num, char *c_num, int lim, int size)  {
    for(int i = 0; i < lim; i++) {
        if ((num[i] > c_num[i]) && (lim == size))
            return E_OVF;
    }
    return E_NOERR;
}

int detect_num_type(char *num, struct report_t *report) {
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

/**
 * Top-level func to detect exact num type 
 * check overflow.
 */
struct report_t *check_num(char *num, char *c_num, int ovf_lim) {
    struct report_t *rep;
    int err = E_NOERR;
    rep = malloc(sizeof(*rep));
    if (rep == NULL) {
        return NULL;
    }
    err = detect_num_type(num, rep);
    if (err == E_NOERR) {
        if (rep->ntype == DOUBLE) {
            if (!c_num[rep->sppos - 1]) 
                err = E_OVF;
            else
                err = check_overflow(num, c_num, rep->sppos, rep->size);
        } else {
            if (!c_num[rep->size - 1]) 
                err = E_OVF;
            else
                err = check_overflow(num, c_num, rep->size, ovf_lim);
        }
    }
    rep->err = err;
    return rep;
}

/**
 * Free memory from report
 */
int del_report(struct report_t *r) {
    if (r == NULL) {
        return E_NULLPT;
    }
    free(r);
    return E_NOERR;
}

int parse_float_precision(const char *arg, struct command_t *cmd, int mth_pos) {
    int p_prec = PRECISION;
    char pvalue[] = {'\0', '\0', '\0'};
    for(int i = 0; arg[mth_pos + i]; i++) {
        if ((arg[mth_pos + i] < '0') || (arg[mth_pos + i] > '9')) {
            return E_UNSUPP;
        }
        pvalue[i] = arg[mth_pos + i];
        if (i == PREC_NUMS_CNT)
            break;
    }
    p_prec = atoi(pvalue);
    if (p_prec > MAX_PREC) {
        p_prec = MAX_PREC;
    }
    cmd->prec = p_prec;
    return E_NOERR;
}

int match_p(const char *arg, const char *templ) {
    const char *tmp = arg;
    for(; *templ; templ++, arg++) {
        if (*templ != *arg) {
            return 0;
        }
    }
    return arg - tmp;
}

int parse_preset(const char *arg, struct command_t *cmd, const char *preset[]) {
    int match = 0, err = E_NOERR;
    for(int i = 0; preset[i]; i++) {
        match = match_p(arg, preset[i]);
        if (!match) {
            continue;
        }
        switch(i) {
            case SET_PREC:
                err = parse_float_precision(arg, cmd, match + 1);
                break;
        }
    }
    if (!match) {
        err = E_NOMTHP;
    }
    return err;
}

int convert_mode(const char *mode, struct command_t *cmd, const char *presets[]) {
    const char *f = ++mode;
    char to_lover;
    char diff = 'a' - 'A';
    for(; *f; f++ ) {
        switch(*f) {
            case 'B':
            case 'H':
            case 'O':
                to_lover = (char)(*f + diff);
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

struct command_t *parse_command(int argc, char *argv[], const char *presets[]) {
    struct command_t *cmd;
    char *arg;
    cmd = malloc(sizeof(*cmd));
    if (cmd == NULL) {
        return NULL;
    }
    cmd->repr_m = CUPPER;
    cmd->err = E_NOERR;
    cmd->prec = PRECISION;
    cmd->num_pos = argc - 1;
    for(int i = 1; i < cmd->num_pos; i++) {
        arg = argv[i];
        switch(arg[0]) {
            case '-':
                cmd->err = convert_mode(arg, cmd, presets);
                break;
            default:
                cmd->err = E_INVFLG;
                break;
        }
        if (cmd->err != E_NOERR) {
            break;
        }
    }
    return cmd;
}

int del_command(struct command_t *cmd) {
    if (cmd == NULL) {
        return E_NULLPT;
    }
    free(cmd);
    return E_NOERR;
}

char *create_string(int size) {
    char *ptr;
    ptr = (char*)calloc((size_t)size, (size_t)sizeof(char));
    return ptr;
}

char decode_symb(num_t dig, char repr_m) {
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

/**
 * Add prefix: Ob | 0x | 0o to template
 */
void set_num_prefix(num_t num, num_t base, char *dest, char *pref) {
    for(; *pref; ) {
        *dest++ = *pref++;
    }
}

void convert(num_t num, num_t base, char *dest, int *l_pos, char repr_m) {
    num_t symb_no;
    int i;
    for(i = 2; num >= base; i++) {
        symb_no = num % base;
        dest[i] = decode_symb(symb_no, repr_m);
        num = (num_t)num / base;
    }
    dest[i] = decode_symb(num, repr_m);
    *l_pos = i;
}

void reverse(char *dest, int *l_pos) {
    int i, j;
    char c;
    for(i = 2, j = *l_pos; i < j; i++, j--) {
        c = dest[j];
        dest[j] = dest[i];
        dest[i] = c;
    }
    dest[*l_pos + 1] = '\n';
    /**
     * Add tail for pair '\0\n'
     */
    *l_pos = ADD_TAIL(*l_pos);
}

void print_result(char *src, int max_pos, int *err) {
    ssize_t res;
    res = write(s_stdout, src, (ssize_t)max_pos);
    if ((res < 0) || (res < (ssize_t)max_pos)) {
        *err = E_STDOUT;
    }
    *err = E_NOERR;
}

void resize_double(char *fl_part, char *dest, int prec) {
    *fl_part = '0';
    for(; *fl_part; dest++, fl_part++) {
        prec--;
        if (prec <= 0) {
            break;
        }
        *dest = *fl_part;
    }
}

void convert_float_part(double tail, char *dest, double base, int prec, char repr_m) {
    int tmp_t;
    for(int i = 0; i < MAX_PREC; i++) {
        tail = tail * base;
        tmp_t = (int)tail;
        dest[i] = decode_symb(tmp_t, repr_m);
        tail -= (double)tmp_t;
    }
    dest[prec] = '\0';
}

void merge_parts(const char *head, const char *tail, char *dest, int size) {
    for(; *head; head++, dest++ ) {
        if (*head == '\n') {
            break;
        }
        *dest = *head;
    }
    if (*tail) {
        *dest = '.';
    }
    dest++;
    for(; *tail; dest++, tail++ ) {
        *dest = *tail;
    }
    *dest = '\n';
}

int main(int argc, char **argv) {
    int sign, err, symbols, max_pos = 0;
    double f_part;
    num_t converted_num, base;
    char *num, *result, *tail, *pref, *merged;
    struct report_t *report;
    struct command_t *cmd;

    if (argc == 1) {
        printf("[dcc] Converter for decimal numbers to hex, oct, bin systems.\n"
                "Version: 0.1.0;\n"
                "Max positive num: %s (2^64-1);\n"
                "Max negative num: %s (2^63);\n"
                "Supported operations with int and float numbers, separator for\n"
                "float numbers is '.' .\n"
                "Commands:\n\n"
                "\t -b[B] - convert number to binary system;\n\n"
                "\t -o[O] - convert number to octal system;\n\n"
                "\t -h[H] - convert number to hex system;\n\n"
                "\t -l[L] - convert hex numbers [A-F] to lower. (capitalized by default)\n\n"
                "Presets:\n\n"
                "\t --prec=[number] - set precision for float numbers representation.\n"
                "\t                   Support number from 0 to 32 (numbers bigger as 32\n"
                "\t                   will be converted to 32).\n\n"
                "Signs:\n\n"
                "\t (Not implemented this version)\n"
                "\t [+] for positive nums;\n"
                "\t [-] for negative nums.\n\n", SYS_UNS_OVF, SYS_SIGN_OVF);
        return 0;
    }
    if ((argc > 1) && (argc < 3)) {
        printf("Not enough args: %d. Need 2 -> [num, [sys]]\n", argc - 1);
        exit(1);
    }
    sign = check_sign(argv[argc - 1]);
    if (sign == E_INV_SIGN) {
        printf("Sign %c not supported in %s.\n", argv[argc - 1][0], argv[argc - 1]);
        exit(1);
    }
    cmd = parse_command(argc, argv, presets);
    if (cmd->err != E_NOERR) {
        // printf("Invalid flag: [%c]. Use: -b[B] (bin) | -h[H] (hex) | -o[O] (oct).\n", cmd->sys);
        printf("ERROR NO: [%d]\n", cmd->err);
        exit(1);
    }
    num = skip_sign(argv[cmd->num_pos]);
    report = check_num(num, get_ctrl_num(sign), OVF_SIZEOF(sign));
    err = report->err;
    if (err != E_NOERR) {
        if (err == E_OVF) {
            printf("Overflow: [%s] | [%c%s]\n", argv[cmd->num_pos], SIGN(sign), get_ctrl_num(sign));
        } else if (err == E_UNSUPP) {
            printf("Unsupported symbols in [%s]. Supported: [.0-9]\n", argv[cmd->num_pos]);
        }
        exit(1);
    }

    converted_num = strtoul(num, NULL, DEC);

    switch(cmd->sys) {
        case 'b':
            symbols = BIN_SYMBS_CNT();
            pref = (char*)"0b";
            base = (num_t)BIN;
            break;
        case 'h':
            symbols = HEX_SYMBS_CNT();
            pref = (char*)"0x";
            base = (num_t)HEX;
            break;
        case 'o':
            symbols = OCT_SYMBS_CNT();
            pref = (char*)"0o";
            base = (num_t)OCT;
            break;
    }

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

        if (err != E_NOERR) {
            printf("Can`t convert float part.\n");
            exit(1);
        }
    }

    set_num_prefix(converted_num, base, result, pref);
    convert(converted_num, base, result, &max_pos, cmd->repr_m);
    reverse(result, &max_pos);

    if (report->ntype == DOUBLE) {
        /**
         * We create new string here of
         * size = size(head) + size(tail) + size(pref + '.', '\0', '\n')
         */
        max_pos = ADD_TAIL(symbols) + ADD_TAIL(cmd->prec);
        merged = create_string(max_pos);
        merge_parts(result, tail, merged, max_pos);
        print_result(merged, max_pos, &err);
        free(merged);
        free(tail);
    } else {
        print_result(result, max_pos, &err);
    }

    free(result);
    del_report(report);
    del_command(cmd);

    if (err == E_STDOUT) {
        printf("Output error\n");
        exit(1);
    }

    return 0;
}
