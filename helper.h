/*
 * helper.h
 *
 *  Created on: Nov 2, 2013
 *      Author: chinhau5
 */

#ifndef HELPER_H_
#define HELPER_H_

#include <stdbool.h>
#include <glib.h> 

GSList *tokenize(const char *str, const char *delim);
char *tokenize_name_and_index(const char *name_and_index, int *low, int *high, bool *no_index);

#endif /* HELPER_H_ */
