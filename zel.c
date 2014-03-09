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
		if (get_vertex_property(g, i, VertexProperty_Type) == SteinerVertex) continue;
		for (j = i + 1; j < g->num_nodes; j++) {
			if (get_vertex_property(g, j, VertexProperty_Type) == SteinerVertex) continue;
			for (k = j + 1; k < g->num_nodes; k++) {
				if (get_vertex_property(g, k, VertexProperty_Type) == SteinerVertex) continue;
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
	Graph *mst;
	float total_weight, max_weight;
	int min_triple;
	float uv_weight, vw_weight;
	float f_weight, f_contracted_weight;
	float win, max_win;
	int best_triple;
	int *steiner_nodes;
	int num_steiner_nodes;

	build_distance_graph(g, distance_graph);
	mst = create_graph(g->num_nodes, g->num_nodes-1, false);
	steiner_nodes = malloc(sizeof(int) * g->num_nodes);
	num_steiner_nodes = 0;

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

	do {
		best_triple = -1;
		max_win = 0;

		for (i = 0; i < num_triples; i++) {
			build_minimum_spanning_tree(distance_graph, 0, mst);
			f_weight = calculate_total_edge_weights(mst);

			/* backup weights before contracting */
			uv_weight = get_edge_weight(distance_graph, triples[i].v[0], triples[i].v[1]);
			vw_weight = get_edge_weight(distance_graph, triples[i].v[1], triples[i].v[2]);

			/* contract */
			set_edge_weight(distance_graph, triples[i].v[0], triples[i].v[1], 0);
			set_edge_weight(distance_graph, triples[i].v[1], triples[i].v[2], 0);

			build_minimum_spanning_tree(distance_graph, 0, mst);
			f_contracted_weight = calculate_total_edge_weights(mst);

			/* restore */
			set_edge_weight(distance_graph, triples[i].v[0], triples[i].v[1], uv_weight);
			set_edge_weight(distance_graph, triples[i].v[1], triples[i].v[2], vw_weight);

			/* win = mst(F)-mst(F(z))-d(z) */
			win = f_weight - f_contracted_weight - triples[i].distance;
			if (win > max_win) {
				max_win = win;
				best_triple = i;
			}
		}

		if (best_triple != -1) {
			/* contract permanently */
			set_edge_weight(distance_graph, triples[best_triple].v[0], triples[best_triple].v[1], 0);
			set_edge_weight(distance_graph, triples[best_triple].v[1], triples[best_triple].v[2], 0);

			steiner_nodes[num_steiner_nodes++] = triples[best_triple].min;
		}
	} while (best_triple != -1);
}
