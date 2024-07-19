#include <stdlib.h>
#include <errno.h>

#include "testlike.h"
#include "../../src/bsa_analyzer.c"

void test_build_hierarhies() {
    char *titles[3] = {"one", "two", "three"};
    int grades[3][3] = {{1, 3, 5}, {0, 1, 3}, {0, 0, 1}};
    int *grade;
    size_t members_cnt = 3;
    H hierarhies[3];
    H h;
    int i = 0, j = 0, res = -1;

    for (i = 0; (size_t) i < members_cnt; i++) { 
        h = new_bsa_hierarhy(titles[i], members_cnt);

        ASSERT_NE_PTR_NULL(h, "test_build_hierarhies", LINE());
        hierarhies[i] = h;
        grade = grades[i];

        for (j = 0; (size_t) j < members_cnt; j++) {
            res = add_new_hierarhy_value(h, grade[j]);

            ASSERT_EQ_INT32(0, res, "test_build_hierarhies", LINE());
        }
    }

    for (i = 0; (size_t) i < members_cnt; i++) {
        free_bsa_hierarhy(hierarhies[i]);
    }
}

void test_calc_weights_from_hierarhies() {
    char *titles[3] = {"one", "two", "three"};
    int grades[3][3] = {{1, 3, 5}, {0, 1, 3}, {0, 0, 1}};
    double wcheck[3] = {0.63, 0.26, 0.11};
    double valcheck;
    int *grade;
    size_t members_cnt = 3;
    H hierarhies[3];
    H h;
    W w;
    int i = 0, j = 0, res = -1;

    /* EINVAL */
    w = new_bsa_weight(members_cnt);
    ASSERT_NE_PTR_NULL(w, "new_bsa_weight_creation", LINE());

    for (i = 0; (size_t) i < members_cnt; i++) { 
        h = new_bsa_hierarhy(titles[i], members_cnt);
        hierarhies[i] = h;
        grade = grades[i];

        for (j = 0; (size_t) j < members_cnt; j++) {
            add_new_hierarhy_value(h, grade[j]);
        }
    }

    res = compute_bsa_weights(hierarhies, members_cnt, w);
    ASSERT_EQ_INT32(0, res, "test_compute_bsa_weights", LINE());

    for (i = 0; (size_t) i < members_cnt; i++) {
        free_bsa_hierarhy(hierarhies[i]);
    }

    for (i = 0; (size_t) i < members_cnt; i++) {
        get_weight(w, &valcheck, (size_t) i);
        ASSERT_EQ_DBL(wcheck[i], valcheck, DBL_e_2, "calc_weight_check", LINE());
    }

    free(w);
}

void test_calc_rating() {
    char *titles[3] = {"one", "two", "three"};
    int grades[3][3] = {{1, 3, 5}, {0, 1, 3}, {0, 0, 1}};
    double rating[3] = {0.0, 0.0, 0.0};

    /* table of grades by each criteria */
    int cr_grades[3][3][3] = {
        {{1, 2, 0}, {0, 1, 3}, {5, 0, 1}},
        {{1, 1, 3}, {1, 1, 0}, {0, 5, 1}},
        {{1, 2, 4}, {0, 1, 6}, {0, 0, 1}},
    };

    int *grade;
    size_t members_cnt = 3;
    H hierarhies[3], altern[3][3];
    H h;
    W w;
    W wghts[3];
    int i = 0, j = 0, k = 0, res = -1;

    /* EINVAL */
    w = new_bsa_weight(members_cnt);

    /* set hierarhies */
    for (i = 0; (size_t) i < members_cnt; i++) { 
        h = new_bsa_hierarhy(titles[i], members_cnt);
        hierarhies[i] = h;
        grade = grades[i];

        for (j = 0; (size_t) j < members_cnt; j++) {
            add_new_hierarhy_value(h, grade[j]);
        }
    }

    res = compute_bsa_weights(hierarhies, members_cnt, w);
    ASSERT_EQ_INT32(0, res, "test_compute_hierarhies_weight", LINE());

    for (k = 0; (size_t) k < members_cnt; k++) {

    /* set alternatives */
        for (i = 0; (size_t) i < members_cnt; i++) { 
            h = new_bsa_hierarhy(titles[i], members_cnt);

            /* where k -> criteria_no, i -> altern_no */
            altern[k][i] = h;

            for (j = 0; (size_t) j < members_cnt; j++) {
                add_new_hierarhy_value(h, cr_grades[k][i][j]);
            }
        }

        /* we will have 3 weights with [a, b, c] array inside
         * next we will join with criterias */
        wghts[k] = new_bsa_weight(members_cnt);
        res = compute_bsa_weights(altern[k], members_cnt, wghts[k]);
        ASSERT_EQ_INT32(0, res, "test_compute_altern_weights", LINE());
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        /* iterate by wghts[k] */
        res = make_rating(w, wghts[k], rating);
        ASSERT_EQ_INT32(0, res, "test_compute_rating", LINE());
    }

    for (i = 0; (size_t) i < members_cnt; i++) {
        free_bsa_hierarhy(hierarhies[i]);
        free_weight(wghts[i]);

        for (j = 0; (size_t) j < members_cnt; j++) {
            free_bsa_hierarhy(altern[i][j]);
        }
    }

    free_weight(w);
}

int main() {
    test_build_hierarhies();
    test_calc_weights_from_hierarhies();
    test_calc_rating();
    return 0;
}

