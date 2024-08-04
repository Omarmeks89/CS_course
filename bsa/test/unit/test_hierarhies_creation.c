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

            ASSERT_EQ_INT32(res, 0, "test_build_hierarhies", LINE());
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
    ASSERT_EQ_INT32(res, 0, "test_compute_bsa_weights", LINE());

    for (i = 0; (size_t) i < members_cnt; i++) {
        free_bsa_hierarhy(hierarhies[i]);
    }

    for (i = 0; (size_t) i < members_cnt; i++) {
        get_weight(w, &valcheck, (size_t) i);
        ASSERT_EQ_DBL(wcheck[i], valcheck, DBL_e_2, "calc_weight_check", LINE());
    }

    free_weight(w);
}

void test_calc_rating() {
    char *titles[3] = {"one", "two", "three"};
    double rating[3] = {0.0, 0.0, 0.0};
    double ctrl_res[3] = {0.425, 0.439, 0.135};

    /* table of grades by each criteria 
     * each table filled as a row */
    int cr_grades[3][3][3] = {
        {{1, 2, 2}, {0, 1, 5}, {0, 0, 1}},
        {{1, 0, 5}, {4, 1, 8}, {0, 0, 1}},
        {{1, 4, 8}, {0, 1, 0}, {0, 5, 1}},
    };

    size_t members_cnt = 3;
    H altern[3][3], h;
    W wghts[3], w;
    double control_weights[3] = {0.63, 0.26, 0.11};
    int i = 0, j = 0, k = 0, res = -1;

    w = (W) malloc(sizeof(struct _bsa_weight));
    w->weights = (double *) malloc(sizeof(double) * 3);
    w->w_cnt = members_cnt;

    for (i = 0; (size_t) i < members_cnt; i++) {
        w->weights[i] = control_weights[i];
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        for (i = 0; (size_t) i < members_cnt; i++) { 
            h = new_bsa_hierarhy(titles[i], members_cnt);
            altern[k][i] = h;

            for (j = 0; (size_t) j < members_cnt; j++) {
                add_new_hierarhy_value(h, cr_grades[k][i][j]);
            }
        }

        wghts[k] = new_bsa_weight(members_cnt);
        res = compute_bsa_weights(altern[k], members_cnt, wghts[k]);
        ASSERT_EQ_INT32(res, 0, "test_compute_altern_weights", LINE());
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        res = make_rating(w->weights[k], wghts[k], rating);
        ASSERT_EQ_INT32(res, 0, "test_compute_rating", LINE());
    }

    for (i = 0; (size_t) i < members_cnt; i++) {
        EXPECT_EQ_DBL(rating[i], ctrl_res[i], DBL_e_3, "calc_control", LINE());
        free_weight(wghts[i]);

        for (j = 0; (size_t) j < members_cnt; j++) {
            free_bsa_hierarhy(altern[i][j]);
        }
    }

    free_weight(w);
}

void test_calc_rating_assymmetric() {
    char *titles[2] = {"one", "two"};
    double rating[2] = {0.0, 0.0};
    double ctrl_res[2] = {0.569, 0.430};

    /* table of grades by each criteria 
     * each table filled as a row */
    int cr_grades[3][2][2] = {
        {{1, 2}, {0, 1}},
        {{1, 0}, {4, 1}},
        {{1, 8}, {0, 1}},
    };

    size_t members_cnt = 3, alt_cnt = 2;
    H altern[3][2], h;
    W wghts[3], w;
    double control_weights[3] = {0.63, 0.26, 0.11};
    int i = 0, j = 0, k = 0, res = -1;

    w = (W) malloc(sizeof(struct _bsa_weight));
    w->weights = (double *) malloc(sizeof(double) * 3);
    w->w_cnt = members_cnt;

    for (i = 0; (size_t) i < members_cnt; i++) {
        w->weights[i] = control_weights[i];
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        for (i = 0; (size_t) i < alt_cnt; i++) { 
            h = new_bsa_hierarhy(titles[i], alt_cnt);
            altern[k][i] = h;

            for (j = 0; (size_t) j < alt_cnt; j++) {
                add_new_hierarhy_value(h, cr_grades[k][i][j]);
            }
        }

        wghts[k] = new_bsa_weight(members_cnt);
        res = compute_bsa_weights(altern[k], alt_cnt, wghts[k]);
        ASSERT_EQ_INT32(res, 0, "test_compute_altern_weights_assym", LINE());
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        res = make_rating(w->weights[k], wghts[k], rating);
        ASSERT_EQ_INT32(res, 0, "test_compute_rating_assym", LINE());
    }

    for (k = 0; (size_t) k < members_cnt; k++) {
        free_weight(wghts[k]);

        for (j = 0; (size_t) j < alt_cnt; j++) {
            free_bsa_hierarhy(altern[k][j]);
        }
    }
    for (i = 0; (size_t) i < alt_cnt; i++) {
        EXPECT_EQ_DBL(rating[i], ctrl_res[i], DBL_e_3, "calc_control_assym", LINE());
    }

    free_weight(w);
}

int main() {
    test_build_hierarhies();
    test_calc_weights_from_hierarhies();
    test_calc_rating();
    test_calc_rating_assymmetric();
    return 0;
}

