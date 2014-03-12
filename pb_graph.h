/*
 * pb_graph.h
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#ifndef PB_GRAPH_H_
#define PB_GRAPH_H_

#include "vpr_types.h"

s_port *find_pb_type_port(s_pb_type *pb_type, const char *port_name);
void alloc_pb_children(s_pb *pb);
void alloc_and_init_pb_pins(s_pb *pb);

s_pb_graph_pin ***get_pb_pins(s_pb *parent_pb, s_pb **child_pbs, const char *port_string, int *num_sets, int **num_pins);


#endif /* PB_GRAPH_H_ */
