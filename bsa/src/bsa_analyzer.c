#include <stdlib.h>
#include <errno.h>

#include "bsa_analyzer.h"

struct hierarhy {
    const char  *title;
    size_t      limit;
    size_t      pos;
    double      *values;
};

struct _bsa_weight {
    size_t      w_cnt;
    double      *weights;
};

H new_bsa_hierarhy(char *title, size_t members) {
    H h;

    if (members <= 0)
        return NULL;

    h = (H) malloc(sizeof(*h));
    if (h == NULL)
        return NULL;

    h->values = (double *) calloc(members, sizeof(double));
    if (h->values == NULL)
        return NULL;

    h->pos= 0;
    h->limit = members;

    return h;
}

void free_bsa_hierarhy(H h) {
    if (h != NULL) {
        free(h->values);
        free(h);
    }
}

int add_new_hierarhy_value(H h, int value) {
    if (h == NULL)
        return EFAULT;

    if (value < 0)
        return EINVAL;

    if (h->limit == h->pos)
        return -3;

    h->values[h->pos] = value;

    return 0;
}

static int
make_rating(W h, W a, double rating[]) {
    int i, j;

    if ((h == NULL) || (a == NULL) || (rating == NULL))
        return EFAULT;

    for (i = 0; i < a->w_cnt; i++) {
        for (j = 0; j < h->w_cnt; j++) {
            rating[i] += a->weights[i] * h->weights[j];
        }
    }

    return 0;
}

static int
compute_bsa_weights(H hierarhies[], size_t h_count, W w) {
    double col_sum[h_count], tmp;
    int i, j;

    if ((hierarhies == NULL) || (w == NULL))
        return EFAULT;

    for (i = 0; i < h_count; i++) {
        col_sum[i] = 0.0;
    }

    for (i = 0; i < h_count; i++) {
        for (j = 0; j < h_count; j++) {
            if ((int) (hierarhies[i]->values[j]) <= 0) {
                tmp = hierarhies[j]->values[i];

                if ((int) tmp <= 0)
                    return EINVAL;

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
