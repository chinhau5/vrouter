/*
 * parse_arch.c
 *
 *  Created on: Oct 30, 2013
 *      Author: chinhau5
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"
#include "vpr_types.h"
#include "xml_helper.h"

const char *interconnect_type_name[] = { "direct", "complete", "mux" };
const char *port_type_name[] = { "input", "output", "clock" };

void parse_pb_type(xmlNodePtr pb_type_node, s_pb_type *pb_type, s_pb_type *parent);

//void print_tabs(FILE *fp, int num_tabs)
//{
//	int i;
//	for (i = 0; i < num_tabs; i++) {
//		fprintf(fp, "\t");
//	}
//}

void dump_pb(s_pb_type *pb_type, int level)
{
	int i, j;

	print_tabs(stdout, level); printf("name: %s\n", pb_type->name);
	print_tabs(stdout, level); printf("num_pbs: %d\n", pb_type->num_pbs);
	print_tabs(stdout, level); printf("blif_model: %s\n", pb_type->blif_model);

	for (i = 0; i < pb_type->num_input_ports; i++) {
		print_tabs(stdout, level); printf("Input port: %s Num pins: %d\n", pb_type->input_ports[i].name, pb_type->input_ports[i].num_pins);
	}
	for (i = 0; i < pb_type->num_output_ports; i++) {
		print_tabs(stdout, level); printf("Output port: %s Num pins: %d\n", pb_type->output_ports[i].name, pb_type->output_ports[i].num_pins);
	}
	for (i = 0; i < pb_type->num_clock_ports; i++) {
		print_tabs(stdout, level); printf("Clock port: %s Num pins: %d\n", pb_type->clock_ports[i].name, pb_type->clock_ports[i].num_pins);
	}

	if (pb_type->parent) {
		print_tabs(stdout, level); printf("Parent name: %s\n", pb_type->parent->name);
	}

	for (i = 0; i < pb_type->num_modes; i++) {
		print_tabs(stdout, level+1); printf("Mode: %s\n", pb_type->modes[i].name);
		for (j = 0; j < pb_type->modes[i].num_children; j++) {
			dump_pb(&pb_type->modes[i].children[j], level+2);
		}
		for (j = 0; j < pb_type->modes[i].num_interconnects; j++) {
			print_tabs(stdout, level+2); printf("Interconnect: %s Input: %s Output: %s\n", pb_type->modes[i].interconnects[j].name, pb_type->modes[i].interconnects[j].input_string, pb_type->modes[i].interconnects[j].output_string);
		}
	}
}

void dump_pb_top_type(s_pb_top_type *pb_top_type)
{
	dump_pb(&pb_top_type->base, 0);
	printf("capacity: %d\n", pb_top_type->capacity);
	printf("height: %d\n", pb_top_type->height);
}

void dump_pb_top_types(s_pb_top_type *pb_top_types, int num_pb_top_types)
{
	int i;
	for (i = 0; i < num_pb_top_types; i++) {
		dump_pb_top_type(&pb_top_types[i]);
		printf("\n");
	}
}

void parse_port(xmlNodePtr pb_type_node, e_port_type port_type, s_port **ports, int *num_ports, s_pb_type *pb_type)
{
	int i;
	xmlNodePtr port_node;
	*num_ports = get_child_count(pb_type_node, port_type_name[port_type]);
	(*ports) = malloc(sizeof(s_port) * *num_ports);
	port_node = find_next_element(pb_type_node->children, port_type_name[port_type]);
	for (i = 0; i < *num_ports; i++) {
		(*ports)[i].type = port_type;
		(*ports)[i].port_number = i;
		(*ports)[i].name = xmlGetProp(port_node, "name");
		(*ports)[i].num_pins = atoi(xmlGetProp(port_node, "num_pins"));
		(*ports)[i].pb_type = pb_type;
		port_node = find_next_element(port_node->next, port_type_name[port_type]);
	}
}

void parse_pb_type_ports(xmlNodePtr pb_type_node, s_pb_type *pb_type)
{
	parse_port(pb_type_node, INPUT_PORT, &pb_type->input_ports, &pb_type->num_input_ports, pb_type);
	parse_port(pb_type_node, OUTPUT_PORT, &pb_type->output_ports, &pb_type->num_output_ports, pb_type);
	parse_port(pb_type_node, CLOCK_PORT, &pb_type->clock_ports, &pb_type->num_clock_ports, pb_type);
}

void parse_interconnect_type(xmlNodePtr interconnect_node, e_interconnect_type interconnect_type, s_interconnect *interconnect, int *num_interconnects)
{
	xmlNodePtr interconnect_type_node;

	interconnect_type_node = find_next_element(interconnect_node->children, interconnect_type_name[interconnect_type]);
	while (interconnect_type_node) {
		interconnect[*num_interconnects].type = interconnect_type;
		interconnect[*num_interconnects].name = xmlGetProp(interconnect_type_node, "name");
		interconnect[*num_interconnects].input_string = xmlGetProp(interconnect_type_node, "input");
		interconnect[*num_interconnects].output_string = xmlGetProp(interconnect_type_node, "output");

		(*num_interconnects)++;
		interconnect_type_node = find_next_element(interconnect_type_node->next, interconnect_type_name[interconnect_type]);
	}
}

void parse_interconnect(xmlNodePtr interconnect_node, s_mode *mode)
{
	int count;
	int i;

	mode->num_interconnects = 0;
	for (i = 0; i < NUM_INTERCONNECT_TYPE; i++) {
		mode->num_interconnects += get_child_count(interconnect_node, interconnect_type_name[i]);
	}
	mode->interconnects = malloc(sizeof(s_interconnect) * mode->num_interconnects);

	count = 0;
	for (i = 0; i < NUM_INTERCONNECT_TYPE; i++) {
		parse_interconnect_type(interconnect_node, i, mode->interconnects, &count);
	}
	assert(count == mode->num_interconnects);
}

void parse_mode(xmlNodePtr mode_node, s_mode *mode, s_pb_type *mode_parent)
{
	int i;
	xmlNodePtr pb_type_node;
	int interconnect_count;
	xmlNodePtr interconnect_node;

	assert(!strcmp(mode_node->name, "mode") || !strcmp(mode_node->name, "pb_type"));

	mode->name = xmlGetProp(mode_node, "name");
	mode->num_children = get_child_count(mode_node, "pb_type");

	mode->children = malloc(sizeof(s_pb_type) * mode->num_children);
	pb_type_node = find_next_element(mode_node->children, "pb_type");
	for (i = 0; i < mode->num_children; i++) {
		parse_pb_type(pb_type_node, &mode->children[i], mode_parent);
		pb_type_node = find_next_element(pb_type_node->next, "pb_type");
	}

	interconnect_count = get_child_count(mode_node, "interconnect");
	if (interconnect_count != 1) {
		printf("Expected 1 interconnect element for %s but found %d.\n", mode->name, interconnect_count);
	}
	interconnect_node = find_next_element(mode_node->children, "interconnect");
	parse_interconnect(interconnect_node, mode);
}

void parse_primitive(xmlNodePtr pb_type_node, s_pb_type *pb_type)
{
	s_mode *mode;
	s_pb_type *child;

	if (pb_type->class_name) {
		if (!strcmp(pb_type->class_name, "lut")) {
			pb_type->num_modes = 1;
			pb_type->modes = malloc(sizeof(s_mode));
			mode = &pb_type->modes[0];
			mode->name = strdup("lut6");
			mode->children = malloc(sizeof(s_pb_type));
			mode->num_children = 1;
			mode->interconnects = NULL;
			mode->num_interconnects = 0;
			child = &mode->children[0];
			child->blif_model = NULL;
			child->class_name = NULL;
			child->modes = NULL;
			child->num_modes = 0;
			child->num_clock_ports = 0;
			child->clock_ports = NULL;
			child->input_ports = malloc(sizeof(s_port));
			child->input_ports[0].name = strdup("in");
			child->input_ports[0].port_number = 0;
			child->input_ports[0].num_pins = pb_type->input_ports[0].num_pins;
			child->input_ports[0].type = INPUT_PORT;
			child->input_ports[0].pb_type = child;
			child->num_input_ports = 1;
			child->output_ports = malloc(sizeof(s_port));
			child->output_ports[0].name = strdup("out");
			child->output_ports[0].port_number = 0;
			child->output_ports[0].num_pins = 1;
			child->output_ports[0].type = OUTPUT_PORT;
			child->output_ports[0].pb_type = child;
			child->num_output_ports = 1;
			child->num_pbs = 1;
			child->parent = pb_type;
			child->name = strdup("lut");
		} else {
			pb_type->num_modes = 0;
			pb_type->modes = NULL;
		}
	} else {
		pb_type->num_modes = 0;
		pb_type->modes = NULL;
	}

}

void parse_pb_type(xmlNodePtr pb_type_node, s_pb_type *pb_type, s_pb_type *parent)
{
	xmlNodePtr mode_node;
	int count;
	char *num_pbs;

	assert(!strcmp(pb_type_node->name, "pb_type"));

	pb_type->name = xmlGetProp(pb_type_node, "name");
	pb_type->blif_model = xmlGetProp(pb_type_node, "blif_model");
	pb_type->class_name = xmlGetProp(pb_type_node, "class");
	pb_type->parent = parent;
	num_pbs = xmlGetProp(pb_type_node, "num_pb");
	if (num_pbs) {
		pb_type->num_pbs = atoi(num_pbs);
	} else {
		pb_type->num_pbs = 1;
	}

	parse_pb_type_ports(pb_type_node, pb_type);

	/* this is a primitive, no more children */
	if (pb_type->blif_model) {
		parse_primitive(pb_type_node, pb_type);
	} else {
		pb_type->num_modes = get_child_count(pb_type_node, "mode");
		if (pb_type->num_modes > 0) {
			pb_type->modes = malloc(sizeof(s_mode) * pb_type->num_modes);

			mode_node = find_next_element(pb_type_node->children, "mode");
			count = 0;
			while (mode_node) {
				parse_mode(mode_node, &pb_type->modes[count], pb_type);
				count++;
				mode_node = find_next_element(mode_node->next, "mode");
			}
			assert(count == pb_type->num_modes);
		} else {
			pb_type->num_modes = 1;
			pb_type->modes = malloc(sizeof(s_mode) * pb_type->num_modes);

			parse_mode(pb_type_node, &pb_type->modes[0], pb_type);
		}
	}
}

void parse_pb_top_type(xmlNodePtr pb_type_node, s_pb_top_type *pb_top_type)
{
	char *str;
	parse_pb_type(pb_type_node, &pb_top_type->base, NULL);
	str = xmlGetProp(pb_type_node, "capacity");
	if (str) {
		pb_top_type->capacity = atoi(str);
	} else {
		pb_top_type->capacity = 1;
	}
	str = xmlGetProp(pb_type_node, "height");
	if (str) {
		pb_top_type->height = atoi(str);
	} else {
		pb_top_type->height = 1;
	}
}

void parse_complex_block_list(xmlNodePtr complexblocklist_node, s_pb_top_type **pb_top_types, int *num_pb_top_types)
{
	xmlNodePtr current;
	int count;

	check_element_name(complexblocklist_node, "complexblocklist");

	*num_pb_top_types = get_child_count(complexblocklist_node, "pb_type");
	*pb_top_types = malloc(sizeof(s_pb_top_type) * *num_pb_top_types);

	current = find_next_element(complexblocklist_node->children, "pb_type");
	count = 0;
	while (current) {
		parse_pb_top_type(current, &((*pb_top_types)[count]));
		current = find_next_element(current->next, "pb_type");
		count++;
	}
	assert(count == *num_pb_top_types);
}

void parse_arch(const char *filename, s_pb_top_type **pb_top_types, int *num_pb_top_types)
{
	xmlDocPtr doc;
	xmlNodePtr complexblocklist_node;
	xmlNodePtr root_node;

	doc = xmlParseFile(filename);
	root_node = xmlDocGetRootElement(doc);
	check_element_name(root_node, "architecture");

	complexblocklist_node = find_next_element(root_node->children, "complexblocklist");
	parse_complex_block_list(complexblocklist_node, pb_top_types, num_pb_top_types);

	dump_pb_top_types(*pb_top_types, *num_pb_top_types);
}
