/*
 * vpr_types.h
 *
 *  Created on: Oct 12, 2013
 *      Author: chinhau5
 */

#ifndef VPR_TYPES_H_
#define VPR_TYPES_H_

#include <stdbool.h>
#include <glib.h>
#include "list.h"
#include "bounding_box.h"

typedef enum e_block_pin_type { BLOCK_INPUT, BLOCK_OUTPUT } e_block_pin_type;

//typedef enum e_side { TOP, RIGHT, BOTTOM, LEFT, SIDE_END } e_side;

typedef enum e_block_type { CLB, X_CHANNEL, Y_CHANNEL, SWITCH_BOX } e_block_type;

//typedef enum e_rr_type { CHANX, CHANY, RR_TYPE_END } e_rr_type;

typedef enum _e_routing_node_type {	CLK_PIN, OUTPUT_PIN, INPUT_PIN, WIRE } e_routing_node_type;

typedef enum _e_wire_direction {
	WIRE_E, WIRE_W, WIRE_N, WIRE_S,
	WIRE_EN, WIRE_ES, WIRE_WN, WIRE_WS,
	WIRE_NE, WIRE_NW, WIRE_SE, WIRE_SW,
	NUM_WIRE_DIRECTIONS
} e_wire_direction;

//enum { INC_DIRECTION, DEC_DIRECTION, NUM_DIRECTIONS };

typedef struct _s_value_index_pair {
	int value;
	int index;
} s_value_index_pair;

typedef struct _s_wire_type {
	char *name;
	int freq;
	bool is_horizontal;
	int relative_x;
	int relative_y;

	/* determined at runtime */
	int num_wires;
	e_wire_direction direction;
	int shape; /* each direction with num_types has type ranging from 0 to num_types-1 */
} s_wire_type;

typedef struct _s_track {
	int start;
	int length;
	bool is_increasing;
} s_track;

//typedef struct _s_rr_node {
//	int index;
//	e_rr_type type;
//	bool is_increasing;
//	int xlow;
//	int xhigh;
//	int ylow;
//	int yhigh;
//	int ptc_number;
//	s_list children;
//} s_rr_node;

typedef struct _s_routing_node {
	e_routing_node_type type;
	int id;
	int x;
	int y;
	GSList *children;
} s_routing_node;

typedef struct _s_wire {
	s_routing_node base;
	struct _s_wire_type *type;
	int track;
} s_wire;

typedef struct _s_pb_graph_pin {
	s_routing_node base;
	struct _s_pb *pb;
	struct _s_port *port;
	int pin_number;
	GSList *next_pins; /* net */
} s_pb_graph_pin;

typedef struct _s_switch_box {
	struct _s_wire *starting_wires;
	int num_starting_wires;
	GHashTable *starting_direction_to_index;
	GHashTable **starting_shape_to_index;
	GList *starting_wire_directions;
	GList **starting_wire_shapes;
	int **num_starting_wires_by_type; /* [direction][shape] */

	struct _s_wire **ending_wires;
	int num_ending_wires;
	GList *ending_wire_directions;
	GList **ending_wire_shapes;
	GHashTable *ending_direction_to_index;
	GHashTable **ending_shape_to_index;
	int **num_ending_wires_by_type;
} s_switch_box;

typedef enum _e_interconnect_type { DIRECT, COMPLETE, MUX, NUM_INTERCONNECT_TYPE  } e_interconnect_type;
typedef enum _e_port_type { INPUT_PORT, OUTPUT_PORT, CLOCK_PORT, NUM_PORT_TYPE  } e_port_type;

typedef struct _s_interconnect {
	e_interconnect_type type;
	char *name;
	char *input_string;
	char *output_string;
} s_interconnect;

typedef struct _s_port {
	e_port_type type;
	int port_number;
	char *name;
	int num_pins;

	struct _s_pb_type *pb_type;
} s_port;

typedef struct _s_mode {
	char *name;
	struct _s_pb_type *children;
	int num_children;

	struct _s_interconnect *interconnects;
	int num_interconnects;
} s_mode;

typedef struct _s_pb_type {
	char *name;
	char *blif_model;
	char *class_name;
	int num_pbs;

	struct _s_port *input_ports; /* [port_index][pin_index] */
	int num_input_ports;

	struct _s_port *output_ports; /* [port_index][pin_index] */
	int num_output_ports;

	struct _s_port *clock_ports;
	int num_clock_ports;

	struct _s_pb_type *parent;
	struct _s_mode *modes;
	int num_modes;
} s_pb_type;

typedef struct _s_pb_top_type {
	struct _s_pb_type base;
	int height;
	int capacity;
} s_pb_top_type;

typedef struct _s_pb {
	char *name;
	struct _s_pb_type *type;
	struct _s_mode *mode;

	struct _s_block *block; /* only valid for top level pbs */
	struct _s_pb *parent;
	struct _s_pb **children; /* [pb_type][pb_type_instance] */

	struct _s_pb_graph_pin **input_pins; /* [0..num_input_ports-1] [0..num_port_pins-1]*/
	struct _s_pb_graph_pin **output_pins; /* [0..num_output_ports-1] [0..num_port_pins-1]*/
	struct _s_pb_graph_pin **clock_pins; /* [0..num_clock_ports-1] [0..num_port_pins-1]*/
} s_pb;

typedef struct _s_block {
	int x;
	int y;
	char *name;

	struct _s_pb *pb;
	int capacity; /* specific for IO blocks */
	struct _s_switch_box *switch_box;
} s_block;

//typedef struct _s_physical_block_instance {
//	int x;
//	int y;
//	struct _s_pin *input_pins;
//	int num_input_pins;
//	struct _s_pin *output_pins;
//	int num_output_pins;
//	struct _s_switch_box *switch_box;
//} s_block;

typedef struct _s_block_position {
	int x;
	int y;
	int z;
} s_block_position;

typedef struct _s_net {
	char *name;
	struct _s_pb_graph_pin *source_pin;
	GSList *sink_pins;
	int num_sinks;
	struct _s_bounding_box bounding_box;
} s_net;

typedef struct _s_node_requester {
	s_net *net;
	s_routing_node *node;
} s_node_requester;

#endif /* VPR_TYPES_H_ */
