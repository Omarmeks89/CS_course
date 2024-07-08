#include <stdlib.h>

#include "testlike.h"
#include "../../src/bsa_analyzer.c"

void test_new_bsa_hierarhy_handle_null() {
    char *nulltitle = NULL;
    H hierarhy = NULL;

    hierarhy = new_bsa_hierarhy(nulltitle, (size_t) 2);
    ASSERT_EQ_PTR_NULL(hierarhy, "test_new_bsa_hierarhy_handle_null", LINE());
}

int main() {
    test_new_bsa_hierarhy_handle_null();
    return 0;
}
