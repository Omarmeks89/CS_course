#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include <stdlib.h>

#define DESCRIPTION             "|%8c|%8d|%12s|\n"
#define BORDER                  "================================\n"
#define SEPARATOR               "+--------+--------+------------+\n"

/* contains symbol description constants */
const char * identifiers[5] = {"letter", "digit", "control", "blank", "graph"};

const char * prog_help = "print line you want to be explained:\n";

/* describe position in array */
enum dsc_position {
    LETTER = 0,
    DIGIT,
    CONTROL,
    BLANK,
    GRAPH,
} d_pos;

/* \fn get_ascii_description return description
 * string about symbol. Naive implementation.
 * @param c ascii position 
 * @return pointer on description in description table */
char *
get_ascii_description(int c)
{
    int pos = 0;
    if (isalpha(c) != 0)
        pos = LETTER;
    else if (isdigit(c) != 0)
        pos = DIGIT;
    else if (iscntrl(c) != 0)
        pos = CONTROL;
    else if (isspace(c) != 0)
        pos = BLANK;
    else if (isgraph(c) != 0)
        pos = GRAPH;
    else
        return NULL;

    return (char *) identifiers[pos];
}

int main(void)
{
    char *lc = NULL;
    const char *descr;
    int cli_symb = 0;

    lc = setlocale(LC_ALL, "");
    if (lc == NULL)
        exit(errno);

    printf("%s", prog_help);

    while ((cli_symb = getchar()) != EOF)
    {
        if (cli_symb == '\n')
            break;

        printf(SEPARATOR);

        descr = get_ascii_description(cli_symb);
        if (descr == NULL)
            exit(1);

        printf(DESCRIPTION, cli_symb, cli_symb, descr);
    }

    printf(BORDER);
    return 0;
}
