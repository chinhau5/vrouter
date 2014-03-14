/*
 * helper.c
 *
 *  Created on: Nov 2, 2013
 *      Author: chinhau5
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <glib.h>
#include "list.h"
#include "helper.h"

GSList* tokenize(const char *str, const char *delim)
{
#define TOKENIZE_BUFFER_SIZE 256
	char *token;
	char *copy;
	GSList *result;
	GSList *current;
/*	static int current_buffer_size;*/
/**/
/*	if (copy && strlen(str)+1 > current_buffer_size) {*/
/*		current_buffer_size += TOKENIZE_BUFFER_SIZE;*/
/*		free(copy);*/
/*		copy = malloc(current_buffer_size);*/
/*	} else {*/
/*		current_buffer_size = TOKENIZE_BUFFER_SIZE;*/
/*		copy = malloc(current_buffer_size);*/
/*	}*/

	copy = strdup(str);
	result = NULL;
	current = NULL;
	token = strtok(copy, delim);
	while (token) {
		current = g_slist_append(current, token);
		if (result != NULL) {
			current = current->next;
		} else {
			result = current;
		}
		token = strtok(NULL, delim);
	}
	return result;
}

char *tokenize_name_and_index(const char *name_and_index, int *low, int *high, bool *no_index)
{
	s_list name_and_index_tokens;
	s_list index_tokens;
	char *temp;

	//tokenize(name_and_index, "[]", &name_and_index_tokens);

	if (name_and_index_tokens.num_items == 1) {
		*low = -1;
		*high = *low;
		*no_index = true;
	} else {
		assert(name_and_index_tokens.num_items == 2);

		//tokenize(name_and_index_tokens.head->next->data, ":", &index_tokens);

		if (index_tokens.num_items == 1) { /* name[lsb] */
			*low = atoi(index_tokens.head->data);
			*high = *low;
		} else { /* name[msb:lsb] */
			assert(index_tokens.num_items == 2);

			*low = atoi(index_tokens.head->next->data);
			*high = atoi(index_tokens.head->data);

			assert(*low <= *high);
		}

		*no_index = false;
	}

	temp = name_and_index_tokens.head->data;

	return temp;
}
