#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "bsa_analyzer.h"

/** \struct hierarhy is a container type
 * for collect values, that have hierarhy 
 * relationships between each other.
 */
struct hierarhy {
    const char  *title;
    size_t      limit;              /**< limit (max possible count) of hierarhies */
    size_t      pos;                /**< position for put new hierarhy value */
    double      *values;
};

struct _bsa_weight {
    size_t      w_cnt;              /**< possible weights count */
    double      *weights;           /**< array of weights, size = w_cnt */
};

/**
 * \struct _bsa_weight is used as a container for
 * calculated weights from hierarhies 
 */
typedef struct _bsa_weight *W;

/** \fn new_bsa_hierarhy create new hierarhy
 * struct and return pointer on a struct.
 * @param title a name of current hierarhy
 * @param members possible count of hierarhy elements
 * @return pointer on hierarhy struct
 */
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

/** \fn add_hierarhy_value add new value into 
 * current hierarhy. If hierarhy pointer is NULL,
 * will be returned EFAULT, if value is lower as zero,
 * will be returned EINVAL. If hierarhy haven`t free space will be
 * returned -3 ??
 * @param h pointer on current hierarhy
 * @param value hierarhy value (from 0 to 9)
 * @return status code
 */
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

/** \fn free_bsa_hierarhy release memory
 * allocated for hierarhy struct and nested
 * values array
 * @param h pointer on hierarhy struct
 * @return no return
 */
void free_bsa_hierarhy(H h) {
    if (h != NULL) {
        if (h->values != NULL)
            free(h->values);
        free(h);
    }
}

/** \fn new_bsa_weight create new _bsa_weight struct
 * @param weights calculated weights count
 * @return pointer on _bsa_weights struct
 */
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

/** \fn get_weight is used for receive weight value
 * by position.
 * @param w pointer on _bsa_weight struct
 * @param weight pointer on a variable (double) to store weight
 * @param pos wished weight position
 * @return status code
 */
int get_weight(W w, double *weight, size_t pos) {
    if (pos > (w->w_cnt - 1))
        return EINVAL;

    if ((weight == NULL) || (w == NULL))
        return EFAULT;

    *weight = w->weights[pos];
    return 0;
}

/** \fn free_weight release memory allocated
 * for _bsa_weight struct and nested array
 * @param w pointer on _bsa_weight struct
 * @return no return
 */
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

static H get_hierarhy(H hiers[], size_t alts_cnt, size_t crt_idx, size_t altr_idx) {
    return hiers[crt_idx * alts_cnt + altr_idx];
}

int compute_rating(H alts[], H crts[], size_t crt_cnt, size_t alts_cnt, double rating[]) {
    W crt_weights = NULL;
    W alts_weights[alts_cnt];
    H alterns[alts_cnt], a;

    int res = 0, i = 0, j = 0;

    if ((alts == NULL) || (crts == NULL) || (rating == NULL))
        return EFAULT;

    if ((crt_cnt > MAX_POSSIBLE_MEMBERS) || (alts_cnt > MAX_POSSIBLE_MEMBERS))
        return EINVAL;

    crt_weights = new_bsa_weight(crt_cnt);
    if (crt_weights == NULL)
        return EFAULT;

    res = compute_bsa_weights(crts, crt_cnt, crt_weights);
    if (res != 0) {
        free_weight(crt_weights);
        return res;
    }

    for (i = 0; (size_t) i < crt_cnt; i++) {

        /* alloc weight for each criteria */
        alts_weights[i] = new_bsa_weight(alts_cnt);

        if (alts_weights[i] == NULL) {

            for (j = 0; j < i; j++) {
                free_weight(alts_weights[j]);
            }

            free_weight(crt_weights);
            return EFAULT;
        }

        for (j = 0; (size_t) j < alts_cnt; j++) {
            a = get_hierarhy(alts, alts_cnt, (size_t) i, (size_t) j);
            if (a == NULL)
                break;
            alterns[j] = a;
        }

        res = compute_bsa_weights(alterns, alts_cnt, alts_weights[i]);
        if (res != 0)
            break;
    }

    for (i = 0; (size_t) i < crt_cnt; i++) {
        res = make_rating(crt_weights->weights[i], alts_weights[i], rating);
        if (res != 0)
            break;
    }

    for (i = 0; (size_t) i < crt_cnt; i++) {
        free_weight(alts_weights[i]);
    }

    free_weight(crt_weights);
    return res;
}
