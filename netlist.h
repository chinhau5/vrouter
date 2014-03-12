/*
 * netlist.h
 *
 *  Created on: Nov 1, 2013
 *      Author: chinhau5
 */

#ifndef NETLIST_H_
#define NETLIST_H_

void parse_netlist(
		const char *filename, s_pb_top_type *pb_top_types, int num_pb_top_types, /* inputs */
		s_pb **pbs, int *num_pbs,
		GHashTable **external_nets);

#endif /* NETLIST_H_ */
