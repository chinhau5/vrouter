/*
 * xml_helper.c
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#include <string.h>
#include "xml_helper.h"

xmlNodePtr find_next_element(xmlNodePtr node, const char *element)
{
	xmlNodePtr current;
	for (current = node; current && strcmp(current->name, element); current = current->next) {
	}
	return current;	
}

int get_child_count(xmlNodePtr parent, const char *element)
{
	xmlNodePtr current;
	int count;

	current = find_next_element(parent->children, element);
	count = 0;
	while (current) {
		count++;
		current = find_next_element(current->next, element);
	}

	return count;
}

void check_element_name(xmlNodePtr element, const char *name)
{
	if (strcmp(element->name, name)) {
		printf("[%s:%d] Expected '%s'. Found '%s' instead.\n", element->doc->URL, element->line, name, element->name);
		exit(1);
	}
}
