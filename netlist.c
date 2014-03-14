/*
 * netlist.c
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#include <stdio.h>
#include <math.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "graph.h"
#include "xml_helper.h"
#include "helper.h"
#include "list.h"
#include "vpr_types.h"
#include "pb_graph.h"
#include "netlist.h"

typedef struct _netlist_port {
	char *name;
	struct _netlist_block *block;
	char **connection_specs;
	char **net_name; /* to associate top level output ports with net names */
	int num_pins;
} netlist_port;

typedef struct _netlist_block {
	char *name;
	char *instance;
	char *mode;
	GHashTable *input_ports; /* port name -> port * map */
	GHashTable *output_ports; /* port name -> port * map */
	GHashTable *children;
	struct _netlist_block *parent;
} netlist_block;

void get_full_path(netlist_block *block, char *path, int len)
{
	char *s;
	GSList *list;
	GSList *item;
	netlist_block *current_block;

	current_block = block;
	list = NULL;
	while (current_block) {
		list = g_slist_prepend(list, current_block->instance);
		current_block = current_block->parent;
	}

	path[0] = '\0';
	for (item = list; item != NULL; item = item->next) {
		s = item->data;	
		strcat(path, s);
	}

	g_slist_free(list);
}

char *resolve_reference(const char *ref, int *low, int *high)
{
	char *p;
	bool matched_bracket;
	enum { NameState, HighIndexState, LowIndexState, CompleteState };
	int state;
	bool error;
	char *name;
	char buf[16]; /* 16 characters for the index should be more than enough */
	char *index_ptr;
	char *name_ptr;

	matched_bracket = true;
	state = NameState;
	name = calloc(strlen(ref)+1, sizeof(char));
	name_ptr = name;
	*low = -1;
	*high = -1;
	error = false;
	for (p = ref; *p != '\0' && !error && state != CompleteState; p++) {
		if (*p == '[') {
			matched_bracket = false;
			state = HighIndexState;
			index_ptr = buf;
			p++; /* skip the [ character */
		} else if (*p == ']') {
			if (matched_bracket) {
				matched_bracket = false;
				error = true;	
			} else { 
				matched_bracket = true;
				assert(state == HighIndexState || state == LowIndexState);
				*index_ptr = '\0';
				if (*high == -1) { /* there's only one index */
					*high = atoi(buf);
					*low = *high;
				} else {
					*low = atoi(buf);
				}
				state = CompleteState;
			}
		} else if (*p == ':') {
			if (state == HighIndexState) {
				state = LowIndexState;
				*index_ptr = '\0';
				*high = atoi(buf);
				index_ptr = buf;
				p++; /* skip the : character */
			} else {
				error = true;	
			}
		} else if (*p == '\0') {
			if (state == NameState) {
				//*low = *high = 0; /* we dont have any instance index */
			}
		}

		if (state == NameState) {
			*name_ptr = *p;
			name_ptr++;
		} else if (state == HighIndexState || state == LowIndexState) {
			*index_ptr = *p;
			index_ptr++;
		}
	}

	return error ? NULL : name;

}	

void trim_invalid_characters(char *str)
{
	int len;
	char *p;
	int i;

	len = strlen(str);
	p = str;
	for (i = 0; i < len+1; i++) {
		if (str[i] == '\n' || str[i] == '\t') {
		} else {
			*p = str[i];
			p++; 	
		}
	}
}

void parse_netlist_port(xmlNodePtr port_type_node, netlist_block *block, GHashTable *port_specs) /* num_pins[port] port_connections[port][pin] */
{
	int pin;	
	xmlNodePtr port_node;
	GSList *tokens;
	GSList *item;
	char *port_name;
	netlist_port *port;

	for (port_node = find_next_element(port_type_node->children, "port"); 
			port_node != NULL;
			port_node = find_next_element(port_node->next, "port")) {
		port_name = xmlGetProp(port_node, "name");
		if (g_hash_table_contains(port_specs, port_name)) {
			printf("Error: Existing port specifications for %s found\n", port_name);
			exit(-1);
		}
		tokens = tokenize(port_node->children->content, " ");
		port = malloc(sizeof(netlist_port));
		port->name = port_name;
		port->block = block;
		port->num_pins = g_slist_length(tokens);
		port->connection_specs = malloc(sizeof(char *) * port->num_pins);
		port->net_name = calloc(port->num_pins, sizeof(char *));
		//printf("port name: %s conn: ", port_name);
		for (pin = 0, item = tokens; item != NULL && pin < port->num_pins; item = item->next, pin++) {
			port->connection_specs[pin] = item->data;
			//printf("%s ", port->connection_specs[pin]);
		}	
		//printf("\n");
		g_hash_table_insert(port_specs, port_name, port);
	}
}

void parse_all_ports(xmlNodePtr block_node, netlist_block *block)
{
	xmlNodePtr port_type_node;

	port_type_node = find_next_element(block_node->children, "inputs");
	block->input_ports = g_hash_table_new(g_str_hash, g_str_equal);
	block->output_ports = g_hash_table_new(g_str_hash, g_str_equal);

	if (port_type_node) {
		parse_netlist_port(port_type_node, block, block->input_ports); 
	}

	if (port_type_node) {
		port_type_node = find_next_element(port_type_node->next, "outputs");
		if (port_type_node) {
			parse_netlist_port(port_type_node, block, block->output_ports); 
		}
	}
}


netlist_port *parse_top_level_port(xmlNodePtr port_type_node)
{
	netlist_port *port;
	GSList *tokens;
	GSList *item;
	int pin;
	char *s;

	tokens = tokenize(port_type_node->children->content, " ");
	port = malloc(sizeof(netlist_port));
	port->name = strdup(port_type_node->name);
	port->num_pins = g_slist_length(tokens);
	port->net_name = calloc(port->num_pins, sizeof(char *));
	port->connection_specs = malloc(sizeof(char *) * port->num_pins); 
	for (pin = 0, item = tokens; item != NULL && pin < port->num_pins; item = item->next) {
		s = item->data;
		trim_invalid_characters(s);
		if (s[0] != '\0') {
			port->connection_specs[pin] = s;
			//printf("%s ", port->connection_specs[pin]);
			pin++;
		} else {
			port->num_pins--;
		}
	}	
	//printf("\n");

	return port;
}

void parse_top_level_ports(xmlNodePtr block_node, netlist_block *block)
{
	netlist_port *p;
	xmlNodePtr port_type_node;

	port_type_node = find_next_element(block_node->children, "inputs");
	assert(port_type_node);
	p = parse_top_level_port(port_type_node);
	block->input_ports = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(block->input_ports, p->name, p);

	port_type_node = find_next_element(block_node->children, "outputs");
	assert(port_type_node);
	p = parse_top_level_port(port_type_node);
	block->output_ports = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(block->output_ports, p->name, p);
}


netlist_block *parse_netlist_blocks(xmlNodePtr block_node)
{
	GQueue *frames;
	struct _stack_frame {
		xmlNodePtr block_node;
		struct _stack_frame *parent;
		netlist_block *block;
	} *root_frame, *current_frame, *new_frame;
	xmlNodePtr node;
	int i;
	gpointer key, value;
	GHashTableIter iter;
	netlist_port *root_input_port;
	netlist_port *current_port;
	GSList *tokens;
	int instance_low, instance_high;
	int pin_low, pin_high;
	char *port_name, *instance_name;
	bool is_input;
	char *current_specs;
	char buf[256];
	bool found;

	frames = g_queue_new();

	check_element_name(block_node, "block");

	/*instance_name_and_index = xmlGetProp(block_node, "instance");
	assert(instance_name_and_index);
	instance_name = tokenize_name_and_index(instance_name_and_index, &instance_low, &instance_high, &instance_no_index);
	assert(instance_low == instance_high && !instance_no_index);*/
	new_frame = malloc(sizeof(struct _stack_frame));
	new_frame->block_node = block_node;
	new_frame->parent = NULL;
	new_frame->block = malloc(sizeof(netlist_block));
	new_frame->block->parent = NULL;
	root_frame = new_frame;
	g_queue_push_head(frames, new_frame);

	/* do a depth first search because ports depends on ports on lower level */
	while (!g_queue_is_empty(frames)) {
		current_frame = g_queue_pop_head(frames);	

		if (current_frame->parent) {
			parse_all_ports(current_frame->block_node, current_frame->block);
		} else {
			parse_top_level_ports(current_frame->block_node, current_frame->block);
		}

		current_frame->block->name = xmlGetProp(current_frame->block_node, "name");
		if (current_frame->block->parent) {
			current_frame->block->instance = xmlGetProp(current_frame->block_node, "instance");
		} else {
			current_frame->block->instance = strdup("");
		}
		current_frame->block->mode = xmlGetProp(current_frame->block_node, "mode");
		//printf("name: %s instance: %s mode: %s\n", current_frame->block->name, current_frame->block->instance, current_frame->block->mode);

		current_frame->block->children = g_hash_table_new(g_str_hash, g_str_equal);
		for (node = find_next_element(current_frame->block_node->children, "block"); node != NULL; node = find_next_element(node->next, "block")) {
			new_frame = malloc(sizeof(struct _stack_frame)); 
			new_frame->block_node = node;
			new_frame->parent = current_frame;
			new_frame->block = malloc(sizeof(netlist_block));
			new_frame->block->parent = current_frame->block;

			instance_name = xmlGetProp(node, "instance");
			assert(!g_hash_table_contains(current_frame->block->children, instance_name));
			g_hash_table_insert(current_frame->block->children, instance_name, new_frame->block); 

			g_queue_push_head(frames, new_frame);
		}
	}
	
	return root_frame->block;
}

void lol(netlist_block *block, bool is_input, Graph *g, GHashTable *lookup)
{
	netlist_port *from_port;
	GHashTableIter iter;
	gpointer key, value;
	char tmp[256], from[256], to[256];
	GSList *tokens;
	int num_tokens;
	char *instance_name, *port_name;
	int i;
	int instance_low, instance_high;
	int pin_low, pin_high;
	netlist_block *child_block;
	int v;
	int from_node, to_node;

	if (is_input) {
		g_hash_table_iter_init(&iter, block->input_ports);
	} else {
		g_hash_table_iter_init(&iter, block->output_ports);
	}

	while (g_hash_table_iter_next(&iter, &key, &value)) {
		from_port = value;
		for (i = 0; i < from_port->num_pins; i++) {
			if (strcmp(from_port->connection_specs[i], "open")) {
				tokens = tokenize(from_port->connection_specs[i], ".");
				num_tokens = g_slist_length(tokens);
				get_full_path(block, tmp, sizeof(tmp));

				if (num_tokens == 2) {
					snprintf(to, sizeof(to), "%s%s[%d]", tmp, from_port->name, i);
					instance_name = resolve_reference(tokens->data, &instance_low, &instance_high);
					port_name = resolve_reference(tokens->next->data, &pin_low, &pin_high);
					assert(instance_name && port_name && instance_low == instance_high && pin_low == pin_high);

					if (is_input) {
						/* check for parent level input port and same level output port */
						if (block->parent && !strncmp(block->parent->instance, instance_name, strlen(instance_name)) && g_hash_table_contains(block->parent->input_ports, port_name)) {
							/* no need instance number in this case because we only have one parent */
							get_full_path(block->parent, tmp, sizeof(tmp));
							snprintf(from, sizeof(from), "%s%s[%d]", tmp, port_name, pin_low);
						} else {
							/* need instance number in this case because we have multiple children */
							if (block->parent && g_hash_table_contains(block->parent->children, tokens->data)) {
								child_block = g_hash_table_lookup(block->parent->children, tokens->data);
								if (g_hash_table_contains(child_block->output_ports, port_name)) {
									get_full_path(child_block, tmp, sizeof(tmp));
									snprintf(from, sizeof(from), "%s%s[%d]", tmp, port_name, pin_low);
								}
							}
						}
					} else {
						/* check for current level input port and child level output port */
						if (!strncmp(block->instance, instance_name, strlen(instance_name)) && g_hash_table_contains(block->input_ports, port_name)) {
							get_full_path(block, tmp, sizeof(tmp));
							snprintf(from, sizeof(from), "%s%s[%d]", tmp, port_name, pin_low);
						} else {
							if (g_hash_table_contains(block->children, tokens->data)) {
								child_block = g_hash_table_lookup(block->children, tokens->data);
								if (g_hash_table_contains(child_block->output_ports, port_name)) {
									get_full_path(child_block, tmp, sizeof(tmp));
									snprintf(from, sizeof(from), "%s%s[%d]", tmp, port_name, pin_low);
								}
							}
						}

					}	

				} else {
					/* net names for top level block input or primitive block output  */
					assert(num_tokens == 1);	
					if ((!block->parent && !is_input) || (block->parent && is_input)) {
						strcpy(from, tokens->data);
					 	get_full_path(block, tmp, sizeof(tmp));
						snprintf(to, sizeof(to), "%s%s[%d]", tmp, from_port->name, i);
					} else {
					 	get_full_path(block, tmp, sizeof(tmp));
						snprintf(from, sizeof(from), "%s%s[%d]", tmp, from_port->name, i);
						strcpy(to, tokens->data);
					}
/*					assert(block->parent == NULL || block->parent->parent == NULL || g_hash_table_size(block->children) == 0);*/
/*					if (block->parent == NULL) {*/
/*						if (is_input) {*/
/*							g_hash_table_lookup(block->*/
/*						} else {*/
/*						}*/
/*					} else if (block->parent->parent == NULL) {*/
/*						assert(is_input);*/
/*					} else if (g_hash_table_size(block->children) == 0) {*/
/*						assert(!is_input);*/
/*					} else {*/
/*						assert(0);*/
/*					}*/
				}
				/* the key pointer passed to g_hash_table_insert must be valid always, so strdup is used */
				if (!g_hash_table_contains(lookup, from)) {
					from_node = add_one_vertex(g);
					g_hash_table_insert(lookup, strdup(from), GINT_TO_POINTER(from_node));
				} else {
					from_node = GPOINTER_TO_INT(g_hash_table_lookup(lookup, from));
				}

				if (!g_hash_table_contains(lookup, to)) {
					to_node = add_one_vertex(g);
					g_hash_table_insert(lookup, strdup(to), GINT_TO_POINTER(to_node));
				} else {
					to_node = GPOINTER_TO_INT(g_hash_table_lookup(lookup, to));
				}

				add_edge(g, from_node, to_node, 0);

				printf("%s [%d] -> %s [%d]\n", from, from_node, to, to_node);
			} /* end if (strcmp) */
		}
	}
}
void build_netlist_graph(netlist_block *root_block)
{
	int i;
	gpointer key, value;
	GHashTableIter iter;
	netlist_port *root_input_port;
	netlist_port *current_port;
	GSList *tokens;
	int instance_low, instance_high;
	int pin_low, pin_high;
	char *port_name, *instance_name;
	bool is_input;
	char *current_specs;
	char buf[256];
	bool found;
	netlist_block *current_block, *child_block;
	struct _stack_frame {
		char *path;
		netlist_block *block;
	} *new_frame, *current_frame;
	GQueue *stack;
	GHashTable *graph_lookup;
	Graph *graph;

	stack = g_queue_new();
	graph_lookup = g_hash_table_new(g_str_hash, g_str_equal); 
	graph = create_graph(1000, 1000, true);


	new_frame = malloc(sizeof(struct _stack_frame));
	new_frame->path = malloc(sizeof(char) * 256);
	new_frame->path[0] = '\0';
	new_frame->block = root_block;
	g_queue_push_head(stack, new_frame);

	while (!g_queue_is_empty(stack)) {
		current_frame = g_queue_pop_head(stack);

		current_block = current_frame->block;
		lol(current_block, true, graph, graph_lookup);
		lol(current_block, false, graph, graph_lookup);

		/*		current_block->*/

		g_hash_table_iter_init(&iter, current_block->children);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			child_block = value;
			new_frame = malloc(sizeof(struct _stack_frame));
			new_frame->path = malloc(sizeof(char) * 256);
			snprintf(new_frame->path, 256, "%s%s", current_frame->path, child_block->instance);
			new_frame->block = child_block;
			g_queue_push_head(stack, new_frame);
		}
	}
/*	 for all pins of a port */
/*		for (i = 0; i < root_input_port->num_pins; i++) {*/
/*			current_frame = root_frame;*/
/*				current_specs = root_input_port->connection_specs[i];*/
/*				is_input = false;*/
/*				found = false;*/
/*				while (current_specs && !found) {*/
/*					if (!strcmp(current_specs, "open")) {*/
/*						break;*/
/*					}*/
/*					if (g_hash_table_size(current_frame->children) == 0) {*/
/*						assert(!root_input_port->net_name[i]);*/
/*						root_input_port->net_name[i] = current_specs;*/
/*						found = true;*/
/*					} else {*/
/*						tokens = tokenize(current_specs, ".");*/
/*						assert(g_slist_length(tokens) == 2);*/
/*						instance_name = resolve_reference(tokens->data, &instance_low, &instance_high);*/
/*						port_name = resolve_reference(tokens->next->data, &pin_low, &pin_high);*/
/*						assert(instance_low == instance_high && pin_low == pin_high);*/
/*		*/
/*						current_specs = NULL;*/
/*						if (!is_input) {*/
							/*check for current level input port and child level output port */
/*							if (g_hash_table_contains(current_frame->input_ports, port_name)) {*/
/*								current_port = g_hash_table_lookup(current_frame->input_ports, port_name);*/
/*								current_specs = current_port->connection_specs[pin_low];*/
/*								is_input = true;*/
/*							} else {*/
/*								snprintf(buf, sizeof(buf), "%s[%d]", instance_name, instance_low);*/
/*								if (g_hash_table_contains(current_frame->children, buf)) {*/
/*									current_frame = g_hash_table_lookup(current_frame->children, buf);*/
/*									if (g_hash_table_contains(current_frame->output_ports, port_name)) {*/
/*										current_port = g_hash_table_lookup(current_frame->output_ports, port_name);*/
/*										current_specs = current_port->connection_specs[pin_low];*/
/*										is_input = false;*/
/*									}*/
/*								}*/
/*							}*/
/*						} else {*/
							/* check for parent level input port and same level output port */
/*							if (current_frame->parent && g_hash_table_contains(current_frame->parent->input_ports, port_name)) {*/
/*								current_port = g_hash_table_lookup(current_frame->parent->input_ports, port_name);*/
/*								current_specs = current_port->connection_specs[pin_low];*/
/*								current_frame = current_frame->parent;*/
/*								is_input = true;*/
/*								assert(0);*/
/*							} else {*/
/*								snprintf(buf, sizeof(buf), "%s[%d]", instance_name, instance_low);*/
/*								if (current_frame->parent && g_hash_table_contains(current_frame->parent->children, buf)) {*/
/*									current_frame = g_hash_table_lookup(current_frame->parent->children, buf);*/
/*									if (g_hash_table_contains(current_frame->output_ports, port_name)) {*/
/*										current_port = g_hash_table_lookup(current_frame->output_ports, port_name);*/
/*										current_specs = current_port->connection_specs[pin_low];*/
/*										is_input = false;*/
/*									}*/
/*								}*/
/*							}*/
/*						}	*/
/*					}*/
				/*}*/ /* end while(current_specs) */
/*		*/
/*				}*/
}

void parse_netlist(
		const char *filename, s_pb_top_type *pb_top_types, int num_pb_top_types, /* inputs */
		s_pb **pbs, int *num_pbs,
		GHashTable **external_nets)
{
	xmlDocPtr netlist;
	xmlNodePtr root_node;
	xmlNodePtr block_node;
	int count;
	int block;
	GQueue *block_queue;
	struct _block_node_pb_pair *pair;
	int low, high;
	gpointer key, value;
	GHashTableIter iter;
	netlist_port *p;
	int i;
	netlist_block *root_block;

	netlist = xmlParseFile(filename);
	root_node = xmlDocGetRootElement(netlist);
	check_element_name(root_node, "block");

	root_block = parse_netlist_blocks(root_node);
	build_netlist_graph(root_block);


	/**num_pbs = get_child_count(root_node, "block");*/
	/**pbs = malloc(sizeof(s_pb) * *num_pbs);*/
/*	for (block = 0, block_node = find_next_element(root_node->children, "block"); */
/*			block_node != NULL;*/
/*			block_node = find_next_element(block_node->next, "block"), block++) {*/
/*		frame = parse_netlist_blocks(block_node);*/
/*		g_hash_table_iter_init(&iter, frame->output_ports);*/
/*		while (g_hash_table_iter_next(&iter, &key, &value)) {*/
/*			printf("output port: %s\n", key);	*/
/*			p = value;*/
/*			for (i = 0; i < p->num_pins; i++) {*/
/*				if (p->net_name[i]) {*/
/*					printf("[%d] %s ", i, p->net_name[i]);*/
/*				}*/
/*			}*/
/*			printf("\n");*/
/*		}*/
/*	}*/
}
