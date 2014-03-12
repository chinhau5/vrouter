/*
 * netlist.c
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#include <math.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "xml_helper.h"
#include "helper.h"
#include "list.h"
#include "vpr_types.h"
#include "pb_graph.h"
#include "netlist.h"

struct _block_node_pb_pair {
	xmlNodePtr block_node;
	s_pb *pb;
};

s_pb_type *get_pb_type_from_instance_name(s_pb_type *pb_types, int num_pb_types, const char *instance_name, int *pb_type_index)
{
	int i;

	for (i = 0; i < num_pb_types; i++) {
		if (!strcmp(pb_types[i].name, instance_name)) {
			*pb_type_index = i;
			return &pb_types[i];
		}
	}
	return NULL;
}

s_mode *get_pb_type_mode(const char *mode_name, s_pb_type *pb_type, int *mode_index)
{
	int i;
	for (i = 0; i < pb_type->num_modes; i++) {
		if (!strcmp(pb_type->modes[i].name, mode_name)) {
			*mode_index = i;
			return &pb_type->modes[i];
		}
	}
	return NULL;
}

void dump_netlist(s_block **grid, int nx, int ny)
{
	int x, y;

//	for (x = 0; x < nx; x++) {
//		for (y = 0; y < ny; y++) {
//			if (grid[x][y].pb) {
//				for ()
//			}
//		}
//	}
}

void parse_block_ports(xmlNodePtr block_node, s_pb *pb, GHashTable *external_nets)
{
	xmlNodePtr inputs_node;
	xmlNodePtr outputs_node;
	xmlNodePtr port_node;
	s_list tokens;
	s_list_item *token;
	char *port_name;
	s_port *port;
	s_pb_graph_pin ***pins;
	int num_sets;
	int *num_pins;
	int pin;
	int i, j;
	s_net *net;
	s_pb_graph_pin *top_level_output_pin;
	GSList *next_pins;
	GSList *list_item;
	bool impossible_to_find_top_level_pin;
	int count;

	inputs_node = find_next_element(block_node->children, "inputs");

	if (inputs_node) {
		port_node = find_next_element(inputs_node->children, "port");
		while (port_node) {
			port_name = xmlGetProp(port_node, "name");
			assert(port_name);
			port = find_pb_type_port(pb->type, port_name);
			assert(port);

			//tokenize(port_node->children->content, " ", &tokens);
			assert(tokens.num_items == find_pb_type_port(pb->type, xmlGetProp(port_node, "name"))->num_pins);

			/* for all pins */
			token = tokens.head;
			pin = 0;
			while (token) {
				if (!strcmp(token->data, "open")) {
					//pb->input_pins[port->port_number][pin].next_pin = NULL;
				} else if (!pb->parent) { /*top level input*/
					if (g_hash_table_contains(external_nets, token->data)) {
						net = g_hash_table_lookup(external_nets, token->data);
					} else {
						net = malloc(sizeof(s_net));
						net->name = strdup(token->data);
						net->num_sinks = 0;
						net->source_pin = NULL;
						net->sink_pins = NULL;
						g_hash_table_insert(external_nets, token->data, net);
					}
					net->sink_pins = g_slist_prepend(net->sink_pins, &pb->input_pins[port->port_number][pin]);
					net->num_sinks++;
				} else {
					pins = get_pb_pins(pb->parent, pb->parent->children, strtok(token->data, "->"), &num_sets, &num_pins);
					assert(num_sets == 1 && num_pins[0] == 1);
					pins[0][0]->next_pins = g_slist_prepend(pins[0][0]->next_pins, &pb->input_pins[port->port_number][pin]);
//					printf("INPUT %s %X->%X ", token->data, pins[0][0], &pb->input_pins[port->port_number][pin]);
//					printf("%s %s.%s[%d]->", pins[0][0]->pb->name, pins[0][0]->pb->type->name, pins[0][0]->port->name, pins[0][0]->pin_number);
//					printf("%s %s.%s[%d]\n", pb->input_pins[port->port_number][pin].pb->name, pb->input_pins[port->port_number][pin].pb->type->name, pb->input_pins[port->port_number][pin].port->name, pb->input_pins[port->port_number][pin].pin_number);
				}

				pin++;
				token = token->next;
			}
			port_node = find_next_element(port_node->next, "port");
		}
	}

	outputs_node = find_next_element(block_node->children, "outputs");

	if (outputs_node) {
		port_node = find_next_element(outputs_node->children, "port");
		while (port_node) {
			port_name = xmlGetProp(port_node, "name");
			assert(port_name);
			port = find_pb_type_port(pb->type, port_name);
			assert(port);

			//tokenize(port_node->children->content, " ", &tokens);
			assert(tokens.num_items == find_pb_type_port(pb->type, xmlGetProp(port_node, "name"))->num_pins);

			/* for all pins */
			token = tokens.head;
			pin = 0;
			while (token) {
				if (!strcmp(token->data, "open")) {
					//pb->output_pins[port->port_number][pin].next_pin = NULL;
				} else if (!pb->mode) { /* primitive output */
//					if (g_hash_table_contains(external_nets, token->data)) {
//						net = g_hash_table_lookup(external_nets, token->data);
//					} else {
//						net = malloc(sizeof(s_net));
//						net->source_pin = NULL;
//						net->sink_pins = NULL;
//						g_hash_table_insert(external_nets, token->data, net);
//					}
//					assert(!net->source_pin);
//					printf("\t%X %s %s.%s[%d]\n", &pb->output_pins[port->port_number][pin], pb->output_pins[port->port_number][pin].pb->name, pb->output_pins[port->port_number][pin].pb->type->name, pb->output_pins[port->port_number][pin].port->name, pb->output_pins[port->port_number][pin].pin_number);
//					lol_pin = pb->output_pins[port->port_number][pin].next_pins;
//					while (lol_pin) {
//						prev = lol_pin;
//						printf("\t%X %s %s.%s[%d]\n", lol_pin, lol_pin->pb->name, lol_pin->pb->type->name, lol_pin->port->name, lol_pin->pin_number);
//						lol_pin = lol_pin->next_pins;
//					}
//					net->source_pin = prev;
					next_pins = pb->output_pins[port->port_number][pin].next_pins;
					impossible_to_find_top_level_pin = false;
					while (next_pins && !impossible_to_find_top_level_pin) {
						impossible_to_find_top_level_pin = true;
						count = 0;
						while (next_pins && impossible_to_find_top_level_pin) { /* iterate through all pins of the current level (BFS) */
							top_level_output_pin = next_pins->data;
							if (top_level_output_pin->port->type == OUTPUT_PORT) {
								impossible_to_find_top_level_pin = false;
								count++;
							}
							next_pins = next_pins->next;
						}
						assert(impossible_to_find_top_level_pin || count == 1);
						next_pins = top_level_output_pin->next_pins;
					}

					if (!impossible_to_find_top_level_pin) {
						assert(!top_level_output_pin->pb->parent); /* make sure it's a top level pin */
						if (g_hash_table_contains(external_nets, token->data)) {
							net = g_hash_table_lookup(external_nets, token->data);
						} else {
							net = malloc(sizeof(s_net));
							net->name = strdup(token->data);
							net->source_pin = NULL;
							net->sink_pins = NULL;
							net->num_sinks = 0;
							g_hash_table_insert(external_nets, token->data, net);
						}
						assert(!net->source_pin);
						net->source_pin = top_level_output_pin;
					}

					//pb->output_pins[port->port_number][pin].net_name = strdup(token->data);
				} else {
					pins = get_pb_pins(pb, pb->children, strtok(token->data, "->"), &num_sets, &num_pins);
					assert(num_sets == 1 && num_pins[0] == 1);
					//pb->output_pins[port->port_number][pin].next_pin = pins[0][0];
					pins[0][0]->next_pins = g_slist_prepend(pins[0][0]->next_pins, &pb->output_pins[port->port_number][pin]);
//					printf("%X->%X ", pins[0][0], &pb->output_pins[port->port_number][pin]);
//					printf("%s %s.%s[%d]->", pins[0][0]->pb->name, pins[0][0]->pb->type->name, pins[0][0]->port->name, pins[0][0]->pin_number);
//					printf("%s %s.%s[%d]\n", pb->output_pins[port->port_number][pin].pb->name, pb->output_pins[port->port_number][pin].pb->type->name, pb->output_pins[port->port_number][pin].port->name, pb->output_pins[port->port_number][pin].pin_number);
				}

				pin++;
				token = token->next;
			}
			port_node = find_next_element(port_node->next, "port");
		}
	}

//	if (!pb->parent) {
//		for (i = 0; i < pb->type->num_output_ports; i++) {
//			for (j = 0; j < pb->type->output_ports[i].num_pins; j++) {
//				if (pb->output_pins[i][j].next_pin) {
//					primitive_pin = &pb->output_pins[i][j];
//					while (primitive_pin->next_pin) {
//						primitive_pin = primitive_pin->next_pin;
//					}
//
//					if (g_hash_table_contains(external_nets, primitive_pin->net_name)) {
//						net = g_hash_table_lookup(external_nets, primitive_pin->net_name);
//					} else {
//						net = malloc(sizeof(s_net));
//						net->source_pin = NULL;
//						net->sink_pins = NULL;
//						g_hash_table_insert(external_nets, primitive_pin->net_name, net);
//					}
//					assert(!net->source_pin);
//					net->source_pin = &pb->output_pins[i][j];
//				}
//			}
//		}
//	}
}

s_pb *parse_block(s_pb **pbs, xmlNodePtr block_node, s_pb_type *pb_types, int num_pb_types, s_pb *parent)
{
	char *instance_name_and_index;
	char *instance_name;
	int instance_low;
	int instance_high;
	bool instance_no_index;
	int pb_type_index;
	s_pb_type *pb_type;
	s_pb *pb;
	char *mode_name;
	int mode_index;
	int i;

	check_element_name(block_node, "block");

	instance_name_and_index = xmlGetProp(block_node, "instance");
	assert(instance_name_and_index);
	instance_name = tokenize_name_and_index(instance_name_and_index, &instance_low, &instance_high, &instance_no_index);
	assert(instance_low == instance_high && !instance_no_index); /* debug */

	pb_type = NULL;
	pb_type_index = -1;
	for (i = 0; i < num_pb_types; i++) {
		if (!strcmp(pb_types[i].name, instance_name)) {
			pb_type = &pb_types[i];
			pb_type_index = i;
			break;
		}
	}
	assert(pb_type); /* debug */
	assert(instance_low < pb_type->num_pbs); /* debug */

	pb = &pbs[pb_type_index][instance_low];

	pb->type = pb_type;
	pb->name = xmlGetProp(block_node, "name");
	assert(pb->name);
	mode_name = xmlGetProp(block_node, "mode");
	if (mode_name) {
		pb->mode = get_pb_type_mode(mode_name, pb->type, &mode_index);
		assert(pb->mode);
	} else {
		pb->mode = NULL;
	}
	pb->parent = parent;
	alloc_and_init_pb_pins(pb);

	return pb;
}

typedef struct _port {
	char **connection_specs;
	int num_pins;
} port;

typedef struct _stack_frame {
	xmlNodePtr block_node;
	GHashTable *input_port_specs; /* port name -> port * map */
	GHashTable *output_port_specs; /* port name -> port * map */
	struct _stack_frame **children;
	int num_children;
} stack_frame;

void parse_port(xmlNodePtr port_type_node, GHashTable **port_specs) /* num_pins[port] port_connections[port][pin] */
{
	int pin;	
	xmlNodePtr port_node;
	GSList *tokens;
	GSList *item;
	char *port_name;
	port *p;

	*port_specs = g_hash_table_new(g_str_hash, g_str_equal);

	for (port_node = find_next_element(port_type_node->children, "port"); 
			port_node != NULL;
			port_node = find_next_element(port_node->next, "port")) {
		port_name = xmlGetProp(port_node, "name");
		if (g_hash_table_contains(*port_specs, port_name)) {
			printf("Error: Existing port specifications for %s found\n", port_name);
			exit(-1);
		}
		tokens = tokenize(port_node->children->content, " ");
		p = malloc(sizeof(port));
		p->num_pins = g_slist_length(tokens);
		p->connection_specs = malloc(sizeof(char *) * p->num_pins);
		printf("port name: %s conn: ", port_name);
		for (pin = 0, item = tokens; item != NULL && pin < p->num_pins; item = item->next, pin++) {
			p->connection_specs[pin] = item->data;
			printf("%s ", p->connection_specs[pin]);
		}	
		printf("\n");
		g_hash_table_insert(*port_specs, port_name, p);
	}
}
void parse_all_ports(stack_frame *frame)
{
	xmlNodePtr port_type_node;

	port_type_node = find_next_element(frame->block_node->children, "inputs");
	if (port_type_node) {
		parse_port(port_type_node, &frame->input_port_specs); 
	}

	if (port_type_node) {
		port_type_node = find_next_element(port_type_node->next, "outputs");
		if (port_type_node) {
			parse_port(port_type_node, &frame->output_port_specs); 
		}
	}
}

void search()
{
}

void parse_block_2(xmlNodePtr block_node)
{
	GQueue *frames;
	struct _stack_frame *root_frame, *current_frame, *new_frame;
	xmlNodePtr node;
	int i;

	frames = g_queue_new();

	check_element_name(block_node, "block");

	/*instance_name_and_index = xmlGetProp(block_node, "instance");
	assert(instance_name_and_index);
	instance_name = tokenize_name_and_index(instance_name_and_index, &instance_low, &instance_high, &instance_no_index);
	assert(instance_low == instance_high && !instance_no_index);*/
	new_frame = malloc(sizeof(stack_frame));
	new_frame->block_node = block_node;
	root_frame = new_frame;
	g_queue_push_head(frames, new_frame);

	/* do a depth first search because ports depends on ports on lower level */
	while (!g_queue_is_empty(frames)) {
		current_frame = g_queue_pop_head(frames);	
		printf("name: %s instance: %s mode: %s\n", xmlGetProp(current_frame->block_node, "name"), xmlGetProp(current_frame->block_node, "instance"), xmlGetProp(current_frame->block_node, "mode"));
		parse_all_ports(current_frame);
		current_frame->num_children = get_child_count(current_frame->block_node, "block");
		current_frame->children = malloc(sizeof(stack_frame) * current_frame->num_children);
		for (i = 0, node = find_next_element(current_frame->block_node->children, "block"); i < current_frame->num_children && node != NULL; node = find_next_element(node->next, "block"), i++) {
			new_frame = malloc(sizeof(stack_frame));
			new_frame->block_node = node;
			current_frame->children[i] = new_frame;
			g_queue_push_head(frames, new_frame);
		}
	}
	
	g_queue_push_head(frames, root_frame);
	/* do DFS again but this time look for port connections */
	
/*	while (!g_queue_is_empty(frames)) {*/
/*		current_frame = g_queue_pop_head(frames);	*/
/*		//printf("name: %s instance: %s mode: %s\n", current_frame->);*/
/*		for (port = 0; port < current_frame->num_output_ports; port++) {*/
/*			for (pin = 0; pin < current_frame->num_output_pins[port]; pin++) {*/
/*				current_frame->num_output_pins[port][pin]		*/
/*		parse_all_ports(current_frame);*/
/*		for (i = 0; i < current_frame->num_children; i++) { */
/*			g_queue_push_head(frames, current_frame->children[i]);*/
/*		}*/
/*	}*/
}

void parse_top_level_block(s_pb *pb, GQueue *sub_blocks, xmlNodePtr block_node, s_pb_top_type *pb_top_types, int num_pb_top_types)
{
	int i;
	char *instance_name_and_index;
	char *instance_name;
	int instance_low;
	int instance_high;
	bool instance_no_index;
	s_pb_top_type *pb_top_type;
	char *block_name;
	s_block_position *position;
	s_pb *child_pb;
	char *mode_name;
	int mode_index;
	GQueue *queue;
	xmlNodePtr current_block_node;
	xmlNodePtr child_block_node;
	struct _block_node_pb_pair *pair;

	check_element_name(block_node, "block");

	instance_name_and_index = xmlGetProp(block_node, "instance");
	assert(instance_name_and_index);
	instance_name = tokenize_name_and_index(instance_name_and_index, &instance_low, &instance_high, &instance_no_index);
	assert(instance_low == instance_high && !instance_no_index);

	/* find pb_top_type for validation purposes */
	pb_top_type = NULL;
	for (i = 0; i < num_pb_top_types; i++) {
		if (!strcmp(pb_top_types[i].base.name, instance_name)) {
			pb_top_type = &pb_top_types[i];
			break;
		}
	}
	assert(pb_top_type);
	pb->type = pb_top_type;

	/* find the pb instance pointer */
	block_name = xmlGetProp(block_node, "name");
	assert(block_name);
	pb->name = block_name;

	/* init pb instance struct members */
	mode_name = xmlGetProp(block_node, "mode");
	if (mode_name) {
		pb->mode = get_pb_type_mode(mode_name, pb->type, &mode_index);
		assert(pb->mode);
	} else {
		pb->mode = NULL;
	}

	/* initialized later after the grid is generated (after placement) */
	pb->block = NULL;

	/* parent */
	pb->parent = NULL;

	/* pins */
	alloc_and_init_pb_pins(pb);

	/* parsing children non-recursively */
	queue = g_queue_new();

	pair = malloc(sizeof(struct _block_node_pb_pair));
	pair->pb = pb;
	pair->block_node = block_node;

	g_queue_push_head(queue, pair);
	g_queue_push_head(sub_blocks, pair);

	while (!g_queue_is_empty(queue)) {
		pair = g_queue_pop_tail(queue);

		pb = pair->pb;
		current_block_node = pair->block_node;

		child_block_node = find_next_element(current_block_node->children, "block");
		/* we allocate and parse pb children only when BOTH the architecture and the netlist have children */
		/* case of pb->mode && !child_block_node: LUT route through (tseng.net, name="ngfdn_3") */
		if (pb->mode && child_block_node) {
			pb->children = malloc(sizeof(s_pb *) * pb->mode->num_children);
			for (i = 0; i < pb->mode->num_children; i++) {
				pb->children[i] = calloc(pb->mode->children[i].num_pbs, sizeof(s_pb));
			}

			/* for all children */
			while (child_block_node) {
				child_pb = parse_block(pb->children, child_block_node, pb->mode->children, pb->mode->num_children, pb);
				//parse_block_ports(child_block_node, child_pb, NULL);
				pair = malloc(sizeof(struct _block_node_pb_pair));
				pair->pb = child_pb;
				pair->block_node = child_block_node;
				g_queue_push_head(queue, pair);
				g_queue_push_head(sub_blocks, pair);

				child_block_node = find_next_element(child_block_node->next, "block");
			}
		} else {
			pb->children = NULL;
		}
	}

	g_queue_free(queue);
}

//void init_grid(t_block **grid, int nx, int ny, s_pb_top_type *pb_top_types, int num_pb_top_types)
//{
//
//}

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

	netlist = xmlParseFile(filename);
	root_node = xmlDocGetRootElement(netlist);
	check_element_name(root_node, "block");

	/**num_pbs = get_child_count(root_node, "block");*/
	/**pbs = malloc(sizeof(s_pb) * *num_pbs);*/
	
	for (block = 0, block_node = find_next_element(root_node->children, "block"); 
			block_node != NULL;
			block_node = find_next_element(block_node->next, "block"), block++) {
		parse_block_2(block_node);
	}

	return;

	/* iterate through all the top level blocks */
	count = 0;
	block_queue = g_queue_new();
	*external_nets = g_hash_table_new(g_str_hash, g_str_equal);
	block_node = find_next_element(root_node->children, "block");
	while (block_node) {
		parse_top_level_block(&((*pbs)[count]), block_queue, block_node, pb_top_types, num_pb_top_types);

		while (!g_queue_is_empty(block_queue)) {
			pair = g_queue_pop_tail(block_queue); /* block queue must be traversed in breadth first order */
			parse_block_ports(pair->block_node, pair->pb, *external_nets);
			free(pair);
		}

		block_node = find_next_element(block_node->next, "block");
		count++;
	}
	assert(count == *num_pbs);
}
