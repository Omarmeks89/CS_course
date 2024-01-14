#ifndef IO_UTILS_ENTRY
#define IO_UTILS_ENTRY

#ifdef __cplusplus
extern "C"
#endif

enum io_stream {
    s_stdout = 1,
    s_stderr = 2,
};

void print_result(char *src, int max_pos, int *err);

#endif
