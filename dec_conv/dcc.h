#ifndef DCMOD_ENTRY
#define DCMOD_ENTRY

#ifndef ssep
#define ssep '.'
#endif

 /* We should define max output size (in symbols)
 * and define overflow value:
 *     max positive: 2^64-1 18 446 744 073 709 551 615
 *     max negative: 2^63 9 223 372 036 854 775 808 */
#define SYS_UNS_OVF "18446744073709551615"
#define SYS_SIGN_OVF "9223372036854775808"
#define PRECISION 10
#define PREC_NUMS_CNT 2
#define MAX_PREC 32
#define NO_SYS ' '

/* we make decrement bcs of '\0' at the end */
#define OVF_SIZEOF(s) (s == 1 ? sizeof(SYS_UNS_OVF) - 1 : sizeof(SYS_SIGN_OVF) - 1)

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

typedef unsigned long num_ul64;
typedef struct report_t *conv_report;

/* profiles */
conv_report new_report(int *errno);
int fetch_num(char *num, conv_report report);

#endif
