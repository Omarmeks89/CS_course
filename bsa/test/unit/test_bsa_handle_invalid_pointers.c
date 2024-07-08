#include <stdlib.h>

#include "testlike.h"
#include "../../src/bsa_analyzer.c"

void test_new_bsa_hierarhy_handle_null() {
    char *nulltitle = NULL;
    H hierarhy = NULL;

    hierarhy = new_bsa_hierarhy(nulltitle, (size_t) 2);
    ASSERT_EQ_PTR_NULL(hierarhy, "test_new_bsa_hierarhy_handle_null", LINE());
}

void test_new_bsa_handle_invalid_members() {
    char *title = "title";
    size_t members = -1;
    H hierarhy = NULL;

    hierarhy = new_bsa_hierarhy(title, members);
    ASSERT_EQ_PTR_NULL(hierarhy, "test_new_bsa_handle_invalid_members", LINE());
}

int main() {
    test_new_bsa_hierarhy_handle_null();
    test_new_bsa_handle_invalid_members();
    return 0;
}
