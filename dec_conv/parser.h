#ifndef PARSER_ENTRY_H
#define PARSER_ENTRY_H

#ifdef __cplusplus
extern "C"
#endif

/* Array of supported presets, order of presets
 * is important, bcs we use it`s index to choose 
 * operation. May be that behaviour will be change later. */
const char *presets[] = {
    "prec",
};

int parse_command(int argc, char *argv[], void *cmd, const char *presets[]);

#endif
