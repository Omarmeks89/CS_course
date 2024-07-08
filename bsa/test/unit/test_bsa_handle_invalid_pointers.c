#include <stdlib.h>
#include <errno.h>

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

void test_new_hierarhy_handle_nullptr() {
    H hierarhy = NULL;
    int res = 0;

    res = add_new_hierarhy_value(hierarhy, 1);
    ASSERT_EQ_INT32(EFAULT, res, "test_new_hierarhy_handle_nullptr", LINE());
}

void test_new_hierarhy_handle_invalid_value() {
    H hierarhy = NULL;
    int res = 0;

    hierarhy = new_bsa_hierarhy("test", 2);
    res = add_new_hierarhy_value(hierarhy, -1);
    free_bsa_hierarhy(hierarhy);
    ASSERT_EQ_INT32(EINVAL, res, "test_new_hierarhy_handle_invalid_value", LINE());
}

void test_make_rating_handle_nullptr() {
    int res = 0;
    double rating[1] = {0.123};

    res = make_rating(NULL, NULL, rating);
    ASSERT_EQ_INT32(EFAULT, res, "test_make_rating_handle_nullptr", LINE());
}

int main() {
    test_new_bsa_hierarhy_handle_null();
    test_new_bsa_handle_invalid_members();
    test_new_hierarhy_handle_nullptr();
    test_new_hierarhy_handle_invalid_value();
    test_make_rating_handle_nullptr();
    return 0;
}
