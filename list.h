/*
 * list.h
 *
 *  Created on: Oct 12, 2013
 *      Author: chinhau5
 */

#ifndef LIST_H_
#define LIST_H_

typedef struct _s_list_item {
	void *data;
	struct _s_list_item *next;
	struct _s_list_item *prev;
} s_list_item;

typedef struct _s_list {
	int num_items;
	struct _s_list_item *head;
	struct _s_list_item *current;
} s_list;

void init_list(s_list *list);
void insert_into_list(s_list *list, void *data);

#endif /* LIST_H_ */
