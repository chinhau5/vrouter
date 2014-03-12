/*
 * xml_helper.h
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#ifndef XML_HELPER_H_
#define XML_HELPER_H_

#include <libxml/tree.h>

xmlNodePtr find_next_element(xmlNodePtr node, const char *element);
int get_child_count(xmlNodePtr parent, const char *element);
void check_element_name(xmlNodePtr element, const char *name);

#endif /* XML_HELPER_H_ */
