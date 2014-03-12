/*
 * list.c
 *
 *  Created on: Oct 12, 2013
 *      Author: chinhau5
 */

#include <stdlib.h>
#include "list.h"

void init_list(s_list *list)
{
	list->current = NULL;
	list->head = NULL;
	list->num_items = 0;
}

s_list_item *alloc_list_item()
{
	s_list_item *item = malloc(sizeof(s_list_item));
	item->data = NULL;
	item->next = NULL;
	item->prev = NULL;
	return item;
}

void insert_into_list(s_list *list, void *data)
{
	s_list_item *item;

	item = alloc_list_item();
	item->data = data;

	if (!list->head) {
		list->head = item;
		list->current = item;
	} else {
		item->prev = list->current;
		list->current->next = item;
		list->current = item;
	}

	list->num_items++;
}
