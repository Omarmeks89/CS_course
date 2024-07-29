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

struct assessment {
    const char *title;
    double value;
};

typedef struct assessment *assessment_t;

static assessment_t
make_bsa_assessment(const char *title, double value) {
    assessment_t a;

    if (title == NULL)
        return NULL;

    a = (assessment_t) malloc(sizeof(assessment_t));
    if (a == NULL)
        return NULL;

    a->title = title;
    a->value = value;
    return a;
}

static void
free_bsa_assessment(assessment_t a) {
    if (a != NULL)
        free(a);
}

struct bsa_rating {
    size_t size;
    assessment_t *rating;
};

bsa_rating_t
make_bsa_raiting(size_t obj_cnt) {
    bsa_rating_t r;

    if (obj_cnt > MAX_POSSIBLE_MEMBERS)
        return NULL;

    r = (bsa_rating_t) malloc(sizeof(bsa_rating_t));
    if (r == NULL)
        return NULL;

    r->rating = (assessment_t *) calloc(obj_cnt, sizeof(assessment_t));
    if (r->rating == NULL)
        return NULL;

    r->size = obj_cnt;
    return r;
}

void free_bsa_rating(bsa_rating_t r) {
    int i = 0;

    if (r != NULL) {
        if (r->rating != NULL)
            for (i = 0; (size_t) i < r->size; i++) {
                free_bsa_assessment(r->rating[i]);
            }
            free(r->rating);
        free(r);
    }
}

static int
add_new_assessment(bsa_rating_t r, const char *title, double value, size_t pos) {
    assessment_t a;

    if (r == NULL)
        return EFAULT;

    if (pos > r->size)
        return EINVAL;

    a = make_bsa_assessment(title, value);
    if (a == NULL)
        return EFAULT;

    r->rating[pos] = a;
    return 0;
}

int
make_rating(W h, W a, double *rating) {
    int i;

    if ((h == NULL) || (a == NULL) || (rating == NULL))
        return EFAULT;

    for (i = 0; (size_t) i < a->w_cnt; i++) {
        *rating += a->weights[i] * h->weights[i];
    }

    return 0;
}

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

int bsa(bsa_rating_t r, H hierarhies[], ...);
