#include <stdio.h>
#include "graph.h"
#include <stdlib.h>
int replace_with_shortest_path(Graph *g, Graph *output)
{
	int i, j;
	float **distance;
	int **predecessor;
	Graph *distance_graph;
	Graph *mst;
	GHashTableIter iter;
	gpointer key, value;
	Edge *edge;
	int pred;
	int current_v;

	distance = malloc(sizeof(float *) * g->num_nodes);
	predecessor = malloc(sizeof(int *) * g->num_nodes);
	for (i = 0; i < g->num_nodes; i++) {
		distance[i] = malloc(sizeof(float) * g->num_nodes);
		predecessor[i] = malloc(sizeof(int) * g->num_nodes);
	}
	distance_graph = alloc_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1)); /* complete graph */
	mst = alloc_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1)); /* complete graph */
	
	for (i = 0; i < g->num_nodes; i++) {
		add_vertex(distance_graph, 0, SOURCE);
		add_vertex(mst, 0, SOURCE);
		add_vertex(output, 0, SOURCE);
	}

	/* building distance graph */
	for (i = 0; i < g->num_nodes; i++) {
		build_shortest_path_tree(g, i, distance[i], predecessor[i], NULL);
		for (j = 0; j < g->num_nodes; j++) {
			if (j != i) {
				add_directed_edge(distance_graph, i, j, distance[i][j]);
			}
		}
	}

	printf("distance graph\n");
	print_graph(distance_graph);
	build_min_spanning_tree(distance_graph, 0, mst);
	printf("min spanning tree of distance graph\n");
	print_graph(mst);

	for (i = 0; i < mst->num_nodes; i++) {
		/* if the node has no edges, it's a leaf node because we're iterating through a tree */
		g_hash_table_iter_init(&iter, mst->nodes[i].edges);	
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			/* tracing a path back to source */
			current_v = edge->v2;
			pred = predecessor[i][current_v]; 
			while (pred != i) {
				add_undirected_edge(output, pred, current_v, distance[i][current_v]-distance[i][pred]);
				current_v = pred;
				pred = predecessor[i][current_v];
			}
		}
	}
}
int kmb(Graph *g, Graph *output)
{
	Graph *temp;
	Graph *mst;
	int i;	
	temp = alloc_graph(g->num_nodes, g->num_nodes-1);
//	build_min_spanning_tree(distance_graph, 0, mst);

	replace_with_shortest_path(g, temp);
	

	return 0;
}
