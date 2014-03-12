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
#include "helper.h"

void tokenize(const char *str, const char *delim, s_list *tokens)
{
#define TOKENIZE_BUFFER_SIZE 256
	char *token;
	char *copy;
//	static int current_buffer_size;
//
//	if (copy && strlen(str)+1 > current_buffer_size) {
//		current_buffer_size += TOKENIZE_BUFFER_SIZE;
//		free(copy);
//		copy = malloc(current_buffer_size);
//	} else {
//		current_buffer_size = TOKENIZE_BUFFER_SIZE;
//		copy = malloc(current_buffer_size);
//	}

	copy = strdup(str);

	init_list(tokens);
	token = strtok(copy, delim);
	while (token) {
		insert_into_list(tokens, token);
		token = strtok(NULL, delim);
	}
}

char *tokenize_name_and_index(const char *name_and_index, int *low, int *high, bool *no_index)
{
	s_list name_and_index_tokens;
	s_list index_tokens;
	char *temp;

	tokenize(name_and_index, "[]", &name_and_index_tokens);

	if (name_and_index_tokens.num_items == 1) {
		*low = -1;
		*high = *low;
		*no_index = true;
	} else {
		assert(name_and_index_tokens.num_items == 2);

		tokenize(name_and_index_tokens.head->next->data, ":", &index_tokens);

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
