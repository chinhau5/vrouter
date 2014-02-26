/*
 * zel.c
 *
 *  Created on: 25 Feb, 2014
 *      Author: chinhau5
 */

#include <assert.h>
#include "graph.h"

typedef struct _Triple {
	int v[3];
	int min;
	float distance;
} Triple;

Triple *get_triples(Graph *g, int *num_triples)
{
	Triple *triples;
	int n;
	int i, j, k, v;
	int check;

	n = g->num_source_sink_nodes;
	*num_triples = n*(n-1)*(n-2) / (3*2);

	triples = malloc(sizeof(Triple) * *num_triples);

	n = 0;
	for (i = 0; i < g->num_nodes; i++) {
		if (g->nodes[i].type == STEINER) continue;
		for (j = i + 1; j < g->num_nodes; j++) {
			if (g->nodes[j].type == STEINER) continue;
			for (k = j + 1; k < g->num_nodes; k++) {
				if (g->nodes[k].type == STEINER) continue;
				for (v = 0; v < 3; v++) {
					triples[n].v[v] = i;
				}

				n++;
			}
		}
	}
	assert(n == g->num_source_sink_nodes);

	return triples;
}

//void contract(Graph *g, Triple *t)
//{
//	set_edge_weightt->
//}

void zel(Graph *g)
{
	Triple *triples;
	int num_triples;
	int i, j, v;
	Graph *distance_graph;
	float total_weight, max_weight;
	int min_triple;

	build_distance_graph(g, &distance_graph);

	triples = get_triples(g, &num_triples);

	/* finding the best steiner point for every triple */
	for (i = 0; i < num_triples; i++) {
		max_weight = FLT_MAX;
		for (j = 0; j < g->num_nodes; j++) {
			total_weight = 0;
			for (v = 0; v < 3; v++) {
				total_weight += get_edge_weight(distance_graph, triples[i].v[v], j);
			}
			if (total_weight < max_weight) {
				max_weight = total_weight;
				triples[i].min = j;
				triples[i].distance = total_weight;
			}
		}
	}

//	do {
//		for (i = 0; i < num_triples; i++) {
//			distance_graph
//		}
//	} while (win > 0);
}
