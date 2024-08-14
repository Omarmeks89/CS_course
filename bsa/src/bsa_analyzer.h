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

/** \def MAX_POSSIBLE_MEMBERS is a limit for
 * alternatives and hierarhies count
 */
#define MAX_POSSIBLE_MEMBERS 1024

/** \typedef type for pointer on hierarhy struct
 */
typedef struct hierarhy *H;

H new_bsa_hierarhy(char *title, size_t members);

int add_new_hierarhy_value(H h, int value);

void free_bsa_hierarhy(H h);

int compute_rating(H alts[], H crts[], size_t crt_cnt, size_t alts_cnt, double rating[]);

#ifdef __cplusplus
}
#endif                                                  /* __cplusplus */

#endif                                                  /* BSA_ANALYZER_H */
