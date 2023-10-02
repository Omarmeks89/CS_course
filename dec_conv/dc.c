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
    E_NULLPT    = -6,
};

enum sign {
    SIGNED = 0,
    UNSIGNED,
};

enum ntype_t {
    INT = 0,
    DOUBLE,
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
    // printf("%d | %d | %d\n", report->sppos, report->size, report->ntype);
    return E_NOERR;
}

/**
 * Top-level func to detect exact num type 
 * check overflow.
 */
struct report_t *check_num(char *num, char *c_num) {
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
                err = check_overflow(num, c_num, rep->size, 19);
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

int convert_mode(char *mode, char *c) {
    char *f = ++mode, to_lover;
    char diff = 'a' - 'A';
    switch(*f) {
        case 'B':
        case 'H':
        case 'O':
            to_lover = (char)(*f + diff);
            *c = to_lover;
            break;
        case 'b':
        case 'h':
        case 'o':
            *c = *f;
            break;
        default:
            *c = *f;
            return E_INV_SYS;
    }
    return E_NOERR;
}

char *create_string(int size) {
    char *ptr;
    ptr = (char*)calloc((size_t)size, (size_t)sizeof(char));
    return ptr;
}

char decode_symb(unsigned long dig) {
    char c;
    switch(dig) {
        case 0:
            c = '0';
            break;
        case 1:
            c = '1';
            break;
        case 2:
            c = '2';
            break;
        case 3:
            c = '3';
            break;
        case 4:
            c = '4';
            break;
        case 5:
            c = '5';
            break;
        case 6:
            c = '6';
            break;
        case 7:
            c = '7';
            break;
        case 8:
            c = '8';
            break;
        case 9:
            c = '9';
            break;
        case 10:
            c = 'A';
            break;
        case 11:
            c = 'B';
            break;
        case 12:
            c = 'C';
            break;
        case 13:
            c = 'D';
            break;
        case 14:
            c = 'E';
            break;
        case 15:
            c = 'F';
            break;
    }
    return c;
}

void set_num_prefix(num_t num, num_t base, char *dest, char *pref) {
    for(; *pref; ) {
        *dest++ = *pref++;
    }
}

void convert(num_t num, num_t base, char *dest, int *l_pos) {
    num_t symb_no;
    int i;
    for(i = 2; num >= base; i++) {
        symb_no = num % base;
        dest[i] = decode_symb(symb_no);
        num = (num_t)num / base;
    }
    dest[i] = decode_symb(num);
    *l_pos = i;
}

void reverse(char *dest, int *l_pos) {
    int i, j;
    char c;
    for(i = 2, j = *l_pos; i < j ; i++, j--) {
        c = dest[j];
        dest[j] = dest[i];
        dest[i] = c;
    }
    dest[*l_pos + 1] = '\n';
    /**
     * Add tail for '\0\n'
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
        if (prec == 0) {
            break;
        }
        *dest = *fl_part;
        //printf("%c | %c\n", *fl_part, *dest);
    }
}

void convert_float_part(double tail, char *dest, double base, int prec) {
    int tmp_t;
    for(int i = 0; i < prec; i++) {
        tail = tail * base;
        tmp_t = (int)tail;
        dest[i] = decode_symb(tmp_t);
        tail -= (double)tmp_t;
    }
}

void merge_parts(const char *head, const char *tail, char *dest, int size) {
    for(; *head; head++, dest++) {
        if (*head == '\n') {
            continue;
        }
        *dest = *head;
        // printf("%c\n", *dest);
    }
    dest++;
    *dest++ = '.';
    for(; *tail; tail++, dest++) {
        *dest = *tail;
        // printf("%c\n", *dest);
    }
    dest++;
    *dest = '\n';
}

int main(int argc, char **argv) {
    int sign, err, symbols, max_pos = 0;
    double f_part;
    num_t converted_num, base;
    char *num, *result, *tail, *pref, *merged, sys;
    struct report_t *report;

    if (argc == 1) {
        printf("[dc] Converter for decimal numbers to hex, oct, bin systems.\n"
                "Version: 0.1.0;\n"
                "Max positive num: %s (2^64-1);\n"
                "Max negative num: %s (2^63);\n"
                "Supported systems:\n\n"
                "\t- bin, flag -b[B];\n"
                "\t- oct, flag -o[O];\n"
                "\t- hex, flag -h[H];\n\n"
                "Supported signs:\n\n"
                "\t [+] for positive nums;\n"
                "\t [-] for negative nums.\n\n", SYS_UNS_OVF, SYS_SIGN_OVF);
        return 0;
    }
    if ((argc > 1) && (argc < 3)) {
        printf("Not enough args: %d. Need 2 -> [num, [sys]]\n", argc - 1);
        exit(1);
    }
    sign = check_sign(argv[1]);
    if (sign == E_INV_SIGN) {
        printf("Sign %c not supported in %s.\n", argv[1][0], argv[1]);
        exit(1);
    }
    err = convert_mode(argv[2], &sys);
    if (err == E_INV_SYS) {
        printf("Invalid flag: [%c]. Use: -b[B] (bin) | -h[H] (hex) | -o[O] (oct).\n", sys);
        exit(1);
    }
    num = skip_sign(argv[1]);
    report = check_num(num, get_ctrl_num(sign));
    err = report->err;
    if (err != E_NOERR) {
        if (err == E_OVF) {
            printf("Overflow: [%s] | [%c%s]\n", argv[1], SIGN(sign), get_ctrl_num(sign));
        } else if (err == E_UNSUPP) {
            printf("Unsupported symbols in [%s]. Supported: [.0-9]\n", argv[1]);
        }
        exit(1);
    }

    converted_num = strtoul(num, NULL, DEC);

    switch(sys) {
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
    }
    result = create_string(symbols);

    if (report->ntype == DOUBLE) {
        /* ... */
        tail = create_string(PRECISION);
        resize_double(&num[report->sppos - 1], tail, PRECISION);
        f_part = atof(tail);
        convert_float_part(f_part, tail, (double)base, PRECISION);

        if (err != E_NOERR) {
            printf("Can`t convert float part.\n");
            exit(1);
        }
    }

    set_num_prefix(converted_num, base, result, pref);
    convert(converted_num, base, result, &max_pos);
    reverse(result, &max_pos);

    if (report->ntype == DOUBLE) {
        max_pos = (symbols + 1) + ADD_TAIL(PRECISION);
        merged = create_string(max_pos);
        merge_parts(result, tail, merged, max_pos);
        print_result(merged, max_pos, &err);
        free(merged);
        free(tail);
    } else {
        print_result(result, max_pos, &err);
        free(result);
    }

    del_report(report);

    if (err == E_STDOUT) {
        printf("Output error\n");
        exit(1);
    }

    return 0;
}
