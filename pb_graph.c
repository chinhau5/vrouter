/*
 * pb_graph.c
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#include "pb_graph.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "helper.h"
#include "list.h"
#include "vpr_types.h"

s_pb_type * find_pb_type(s_pb_type *pb_types, int num_pb_types, const char *child_pb_type_name, int *index)
{
	int i;

	for (i = 0; i < num_pb_types; i++) {
		if (!strcmp(child_pb_type_name, pb_types[i].name)) {
			*index = i;
			return &pb_types[i];
		}
	}
	return NULL;
}

s_pb_graph_pin *find_pb_pin(s_pb *pb, s_port *port, int pin_number)
{
	switch (port->type) {
	case INPUT_PORT:
		return &pb->input_pins[port->port_number][pin_number];
	case OUTPUT_PORT:
		return &pb->output_pins[port->port_number][pin_number];
	case CLOCK_PORT:
		return &pb->clock_pins[port->port_number][pin_number];
	default:
		break;
	}

	return NULL;
}

s_port *find_pb_type_port(s_pb_type *pb_type, const char *port_name)
{
	int i;

	for (i = 0; i < pb_type->num_input_ports; i++) {
		if (!strcmp(port_name, pb_type->input_ports[i].name)) {
			return &pb_type->input_ports[i];
		}
	}
	for (i = 0; i < pb_type->num_output_ports; i++) {
		if (!strcmp(port_name, pb_type->output_ports[i].name)) {
			return &pb_type->output_ports[i];
		}
	}
	for (i = 0; i < pb_type->num_clock_ports; i++) {
		if (!strcmp(port_name, pb_type->clock_ports[i].name)) {
			return &pb_type->clock_ports[i];
		}
	}
	return NULL;
}

/* s_pb_graph_pin ***pins pins[set][pin_index]-> */
s_pb_graph_pin ***get_pb_pins(s_pb *parent_pb, s_pb **child_pbs, const char *port_string, int *num_sets, int **num_pins)
{
	s_list tokens;
	s_list_item *item;
	s_list instance_name_and_port_name;
	char *pb_type_name;
	char *pb_type_name_and_index;
	char *port_name_and_index;
	char *port_name;
	int instance_low;
	int instance_high;
	int pin_low;
	int pin_high;
	s_pb_type *child_pb_type;
	s_port *port_ptr;
	int pb_type_index;
	int pin;
	int instance;
	s_pb_graph_pin ***pins;
	int set;
	bool pb_type_no_index;
	bool port_no_index;
	int i;
	s_pb *pb;

	tokenize(port_string, " ", &tokens);

	*num_sets = tokens.num_items;
	pins = malloc(sizeof(s_pb_graph_pin **) * *num_sets);
	*num_pins = malloc(sizeof(int) * *num_sets);

	item = tokens.head;
	set = 0;
	while (item) {
		tokenize(item->data, ".", &instance_name_and_port_name);

		assert(instance_name_and_port_name.num_items == 2);

		pb_type_name_and_index = instance_name_and_port_name.head->data;
		pb_type_name = tokenize_name_and_index(pb_type_name_and_index, &instance_low, &instance_high, &pb_type_no_index);
		port_name_and_index = instance_name_and_port_name.head->next->data;
		port_name = tokenize_name_and_index(port_name_and_index, &pin_low, &pin_high, &port_no_index);

		/* interconnect input can only come from 2 possible sources: parent input port OR child output port */
		/* interconnect output can only go to 2 possible sinks: parent output port OR child input port */
		if (!strcmp(pb_type_name, parent_pb->type->name)) { /* parent */
			port_ptr = find_pb_type_port(parent_pb->type, port_name);
			assert(port_ptr);

			if (port_no_index) {
				pin_low = 0;
				pin_high = port_ptr->num_pins-1;
			}

			(*num_pins)[set] = pin_high-pin_low+1;
			pins[set] = malloc(sizeof(s_pb_graph_pin *) * (*num_pins)[set]);

			for (pin = pin_low; pin <= pin_high; pin++) {
				pins[set][pin-pin_low] = find_pb_pin(parent_pb, port_ptr, pin);
				assert(pins[set][pin-pin_low]);
			}
		} else { /* child */
			pb = NULL;
			for (i = 0; i < child_pbs[0][0].parent->mode->num_children; i++) {
				if (!strcmp(child_pbs[i][0].type->name, pb_type_name)) {
					pb = &child_pbs[i][0];
					pb_type_index = i;
					break;
				}
			}
			assert(pb);

			port_ptr = find_pb_type_port(pb->type, port_name);
			assert(port_ptr);

			if (pb_type_no_index) {
				instance_low = 0;
				instance_high = pb->type->num_pbs-1;
			}

			if (port_no_index) {
				pin_low = 0;
				pin_high = port_ptr->num_pins-1;
			}

			(*num_pins)[set] = (pin_high-pin_low+1) * (instance_high-instance_low+1);
			pins[set] = malloc(sizeof(s_pb_graph_pin *) * (*num_pins)[set]);

			for (instance = instance_low; instance <= instance_high; instance++) {
				for (pin = pin_low; pin <= pin_high; pin++) {
					pins[set][(instance-instance_low)*(pin_high-pin_low+1) + pin-pin_low] =
							find_pb_pin(&child_pbs[pb_type_index][instance], port_ptr, pin);
					assert(pins[set][(instance-instance_low)*(pin_high-pin_low+1) + pin-pin_low]);
				}
			}
		}

		item = item->next;
		set++;
	}

	return pins;
}

void init_pb_pin_connections(s_pb *pb)
{
	int i;
	int interconnect;
	int *num_input_pins;
	int num_input_sets;
	int *num_output_pins;
	int num_output_sets;
	s_pb_graph_pin ***input_pins;
	s_pb_graph_pin ***output_pins;

	if (pb->mode) {
		/* connect all pb_type->modes to pb_type->input_ports and pb_type->output_ports */
		for (interconnect = 0; interconnect < pb->mode->num_interconnects; interconnect++) {
			input_pins = get_pb_pins(pb, pb->mode->children, pb->mode->interconnects[interconnect].input_string, &num_input_sets, &num_input_pins);
			output_pins = get_pb_pins(pb, pb->mode->children, pb->mode->interconnects[interconnect].output_string, &num_output_sets, &num_output_pins);

			switch (pb->mode->interconnects[interconnect].type) {
			case DIRECT:
				assert(num_input_sets == 1 && num_output_sets == 1);
				break;
			case COMPLETE:
				break;
			case MUX:
				assert(num_output_sets == 1);
				for (i = 0; i < num_input_sets; i++) {
					assert(num_output_pins[0] == num_input_pins[i]);
				}
				break;
			default:
				break;
			}
		}

//		for (i = 0; i < pb_type->num_input_ports; i++) {
//			//pb->input_pins[i][j].edges
//		}
	}
}

/* assume that pb->type and pb->mode has already been initialized by parse_block/parse_top_level_block */
void alloc_and_init_pb_pins(s_pb *pb)
{
	int i, j;
	s_pb_type *pb_type;

	/* we check for pb->children instead of pb->mode because it is possible for the architecture to have children but there's no children in netlist */
//	if (pb->children) {
//		for (i = 0; i < pb->mode->num_children; i++) {
//			for (j = 0; j < pb->mode->children[i].num_pbs; j++) {
//				init_pb_pins(&pb->children[i][j]);
//			}
//		}
//	}

	/* requires children to be loaded first before pins can be connected */
	pb_type = pb->type;

	pb->input_pins = malloc(sizeof(s_pb_graph_pin *) * pb_type->num_input_ports);
	for (i = 0; i < pb_type->num_input_ports; i++) {
		pb->input_pins[i] = calloc(pb_type->input_ports[i].num_pins, sizeof(s_pb_graph_pin));
	}

	for (i = 0; i < pb_type->num_input_ports; i++) {
		for (j = 0; j < pb_type->input_ports[i].num_pins; j++) {
			pb->input_pins[pb_type->input_ports[i].port_number][j].base.type = INPUT_PIN;
			if (pb->block) {
				pb->input_pins[pb_type->input_ports[i].port_number][j].base.x = pb->block->x;
				pb->input_pins[pb_type->input_ports[i].port_number][j].base.y = pb->block->y;
			}
			pb->input_pins[pb_type->input_ports[i].port_number][j].port = &pb_type->input_ports[i];
			pb->input_pins[pb_type->input_ports[i].port_number][j].pin_number = j;
			pb->input_pins[pb_type->input_ports[i].port_number][j].pb = pb;
		}
	}

	pb->output_pins = malloc(sizeof(s_pb_graph_pin *) * pb_type->num_output_ports);
	for (i = 0; i < pb_type->num_output_ports; i++) {
		pb->output_pins[i] = calloc(pb_type->output_ports[i].num_pins, sizeof(s_pb_graph_pin));
	}

	for (i = 0; i < pb_type->num_output_ports; i++) {
		for (j = 0; j < pb_type->output_ports[i].num_pins; j++) {
			pb->output_pins[pb_type->output_ports[i].port_number][j].base.type = OUTPUT_PIN;
			if (pb->block) {
				pb->output_pins[pb_type->output_ports[i].port_number][j].base.x = pb->block->x;
				pb->output_pins[pb_type->output_ports[i].port_number][j].base.y = pb->block->y;
			}
			pb->output_pins[pb_type->output_ports[i].port_number][j].port = &pb_type->output_ports[i];
			pb->output_pins[pb_type->output_ports[i].port_number][j].pin_number = j;
			pb->output_pins[pb_type->output_ports[i].port_number][j].pb = pb;
		}
	}

	pb->clock_pins = malloc(sizeof(s_pb_graph_pin *) * pb_type->num_clock_ports);
	for (i = 0; i < pb_type->num_clock_ports; i++) {
		pb->clock_pins[i] = calloc(pb_type->clock_ports[i].num_pins, sizeof(s_pb_graph_pin));
	}

	for (i = 0; i < pb_type->num_clock_ports; i++) {
		for (j = 0; j < pb_type->clock_ports[i].num_pins; j++) {
			pb->clock_pins[pb_type->clock_ports[i].port_number][j].base.type = CLK_PIN;
			if (pb->block) {
				pb->clock_pins[pb_type->clock_ports[i].port_number][j].base.x = pb->block->x;
				pb->clock_pins[pb_type->clock_ports[i].port_number][j].base.y = pb->block->y;
			}
			pb->clock_pins[pb_type->clock_ports[i].port_number][j].port = &pb_type->clock_ports[i];
			pb->clock_pins[pb_type->clock_ports[i].port_number][j].pin_number = j;
			pb->clock_pins[pb_type->clock_ports[i].port_number][j].pb = pb;
		}
	}
}
