/* Simpliest form of system-analysis utils (for educational purposes).
* Copyright (C) 2024  Egor Markov
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef BSA_ANALYZER_H
#define BSA_ANALYZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define MAX_POSSIBLE_MEMBERS 1024

typedef struct hierarhy *H;

typedef struct assessment *assessment_t;

typedef struct bsa_rating *bsa_rating_t;

H new_bsa_hierarhy(char *title, size_t members);

int add_new_hierarhy_value(H h, int value);

void free_bsa_hierarhy(H h);

bsa_rating_t make_bsa_raiting(size_t obj_cnt);

int bsa(bsa_rating_t r, H criterias[], ...);

#ifdef __cplusplus
}
#endif                                                  /* __cplusplus */

#endif                                                  /* BSA_ANALYZER_H */
