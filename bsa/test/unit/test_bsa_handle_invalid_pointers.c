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

    /* will be overflow, because (size_t) is unsigned long int */
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

    res = make_rating(0.0, NULL, rating);
    ASSERT_EQ_INT32(EFAULT, res, "test_make_rating_handle_nullptr", LINE());
}

void test_new_weight_handle_null() {
    size_t cnt = 40000000;
    W w;

    w = new_bsa_weight(cnt);
    ASSERT_EQ_PTR_NULL(w, "test_new_weight_handle_null", LINE());
}

void test_weight_handle_invalid_pos() {
    W w;
    size_t weights_cnt = 3, invalid_pos = 10;
    double v = -1.0;
    int res = -1;

    w = new_bsa_weight(weights_cnt);
    res = get_weight(w, &v, invalid_pos);
    ASSERT_EQ_INT32(res, EINVAL, "raise EINVAL", LINE());
}

int main() {
    test_new_bsa_hierarhy_handle_null();
    test_new_bsa_handle_invalid_members();
    test_new_hierarhy_handle_nullptr();
    test_new_hierarhy_handle_invalid_value();
    test_make_rating_handle_nullptr();
    test_new_weight_handle_null();
    test_weight_handle_invalid_pos();
    return 0;
}
