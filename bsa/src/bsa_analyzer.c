#include <stdlib.h>
#include <stdio.h>
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

typedef struct _bsa_weight *W;

H new_bsa_hierarhy(char *title, size_t members) {
    H h;

    if ((members > MAX_POSSIBLE_MEMBERS) || (title == NULL))
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

int add_new_hierarhy_value(H h, int value) {
    if (h == NULL)
        return EFAULT;

    if (value < 0)
        return EINVAL;

    if (h->limit == h->pos)
        return -3;

    h->values[h->pos] = (double) value;
    h->pos++;

    return 0;
}

void free_bsa_hierarhy(H h) {
    if (h != NULL) {
        if (h->values != NULL)
            free(h->values);
        free(h);
    }
}

W new_bsa_weight(size_t weights) {
    W w;

    if ((weights > MAX_POSSIBLE_MEMBERS) || (weights == 0))
        /* we guarantee, that w->w_cnt will be bigger as zero */
        return NULL;

    w = (W) malloc(sizeof(*w));
    if (w == NULL)
        return NULL;

    w->weights = (double *) calloc(weights, sizeof(double));
    if (w->weights == NULL)
        return NULL;

    w->w_cnt = weights;

    return w;
}

int get_weight(W w, double *weight, size_t pos) {
    if (pos > (w->w_cnt - 1))
        return EINVAL;

    if ((weight == NULL) || (w == NULL))
        return EFAULT;

    *weight = w->weights[pos];
    return 0;
}

void free_weight(W w) {
    if (w != NULL) {
        if (w->weights != NULL)
            free(w->weights);
        free(w);
    }
}

/* a -> [Da, Ca], rating -> [0.0, 0.0], h -> a
 * looks like: rating[i] += a[i] * a;
 * [Da, Ca] here means weghts alternative D & C
 * by criteria a.
 *
 * So we have N alternatives and rating[N] and
 * M criterias, and we have alternatives weights by
 * each criteria. */
int
make_rating(double hr_weight, W a, double rating[]) {
    int i;

    if ((a == NULL) || (rating == NULL))
        return EFAULT;

    for (i = 0; (size_t) i < a->w_cnt; i++) {
        rating[i] += a->weights[i] * hr_weight;
    }

    return 0;
}


/* will compute weight from hierarhies[] and 
 * set to Weight type. */
int
compute_bsa_weights(H hierarhies[], size_t h_count, W w) {
    double col_sum[h_count], tmp;
    int i, j;

    if ((hierarhies == NULL) || (w == NULL))
        return EFAULT;

    for (i = 0; (size_t) i < h_count; i++) {
        col_sum[i] = 0.0;
    }

    for (i = 0; (size_t) i < h_count; i++) {
        for (j = 0; (size_t) j < h_count; j++) {
            tmp = hierarhies[i]->values[j];

            if ((int) tmp <= 0) {
                tmp = hierarhies[j]->values[i];

                if ((int) tmp < 0)
                    return EINVAL;

                tmp = (double) 1.0 / tmp;
                hierarhies[i]->values[j] = tmp;
            }
            col_sum[j] += tmp;
        }
    }

    for (i = 0; (size_t) i < h_count; i++) {
        for (j = 0; (size_t) j < h_count; j++) {
            w->weights[i] += (hierarhies[i]->values[j] / col_sum[j]) / h_count;
        }
    }
    return 0;
}

