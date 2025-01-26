#include <check.h>

/* first work with check */
START_TEST(test_first) 
{
    ck_assert_int_eq(1, 1);
}
END_TEST

/* init Runner to start tests */
SRunner * create_runner()
{
    SRunner *sr = NULL;
    Suite *s = NULL;
    TCase *tc = NULL;

    s = suite_create("FEATURE");

    tc = tcase_create("test_one_eq_one");
    tcase_add_test(tc, test_first);

    suite_add_tcase(s, tc);
    sr = srunner_create(s);

    return sr;
}

int main(void)
{
    int fq = 0;

    SRunner *sr = create_runner();
    srunner_run_all(sr, CK_VERBOSE);

    fq = srunner_ntests_failed(sr);
    return (fq == 0) ? 0 : 1;
}

