#include <stdlib.h>

#include "bsa_analyzer.h"

struct hierarhy {
    const char  *title;
    size_t      limit;
    size_t      h_cnt;
    double      *values;
};

struct _bsa_weight {
    size_t      w_cnt;
    double      *weights;
};

static int compute_bsa_weights(H hierarhies[], size_t h_count, W w);

static int make_rating(W hierarhies_w, W alternatives_w, double rating[]);

static int
make_rating(W hierarhies_w, W alternatives_w, double rating[]) {
    int i, j;

    if ((hierarhies_w == NULL) || (alternatives_w == NULL) || (rating == NULL))
        return -1;

    for (i = 0; i < alternatives_w->w_cnt; i++) {
        for (j = 0; j < hierarhies_w->w_cnt; j++) {
            rating[i] += alternatives_w->weights[i] * hierarhies_w->weights[j];
        }
    }

    return 0;
}

static int
compute_bsa_weights(H hierarhies[], size_t h_count, W w) {
    double col_sum[h_count], tmp;
    int i, j;

    if ((hierarhies == NULL) || (w == NULL))
        return -1;

    for (i = 0; i < h_count; i++) {
        col_sum[i] = 0.0;
    }

    for (i = 0; i < h_count; i++) {
        for (j = 0; j < h_count; j++) {
            if ((int) (hierarhies[i]->values[j]) <= 0) {
                tmp = hierarhies[j]->values[i];

                if ((int) tmp <= 0)
                    return -1;

                tmp = (double) 1.0 / tmp;
                hierarhies[i]->values[j] = tmp;
            }

            col_sum[j] += tmp;
        }
    }

    for (i = 0; i < h_count; i++) {
        for (j = 0; j < h_count; j++) {
            w->weights[j] += (hierarhies[i]->values[j] / col_sum[j]) / h_count;
        }
    }

    return 0;
}
