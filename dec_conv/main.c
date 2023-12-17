/* entry point for app.
 * link all modules together into an app. */

#include <stdio.h>

#include "dcc.h"             /* convert digits from 10sys to oct | bin | hex system. */
#include "ioutils.h"         /* read / write data from / to CLI (terminal). */
#include "sysutils.h"       /* some system utils that will use all of modules (not implemented at 09/12/2023). */
#include "drepr.h"          /* represent nondecimal digits like: hex -> bin, bin -> hex, bin -> oct, oct -> hex (etc). */
#include "parser.h"         /* parse incoming commands using concrete handlers from app modules. */

int main(int argc, char **argv) {
   int sign, err, symbols, max_pos = 0;
    double f_part;
    num_ul64 converted_num, base;
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

    /* move to func ==> void switch_num_system(); */
    switch(cmd->sys) {
        case 'b':
            symbols = BIN_SYMBS_CNT();
            pref = (char*)"0b";
            base = (num_ul64)BIN;
            break;
        case 'h':
            symbols = HEX_SYMBS_CNT();
            pref = (char*)"0x";
            base = (num_ul64)HEX;
            break;
        case 'o':
            symbols = OCT_SYMBS_CNT();
            pref = (char*)"0o";
            base = (num_ul64)OCT;
            break;
    }
    /*-----------------------------------------------*/

    /* ========================================================== 
     * top level func for compiling result (int & float)
     * use <switch_num_system> here. */

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
        print_result(merged, max_pos, &err);
        free(merged);
        free(tail);
    } else {
        print_result(result, max_pos, &err);
    }

    free(result);
    del_report(report);
    del_command(cmd);

    /* ========================== end =========================== */

    if (err == E_STDOUT) {
        printf("Output error\n");
        exit(1);
    }

    return 0;
}
