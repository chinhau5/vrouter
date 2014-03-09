#include <stdio.h>
#include <assert.h>
#include "graph.h"
#include <stdlib.h>

void remove_non_steiner_leaves(Graph *g)
{
	
}

void replace_with_shortest_path(Graph *g, Graph *result)
{
	int i, j;
	float **distance;
	int **predecessor;
	Graph *temp_graph;
	GHashTableIter iter;
	gpointer key, value;
	Edge *edge;
	int pred;
	int current;
	GSList *edge_list;
	Edge *temp_edge;
	GSList *item;
	int steiner;  /* just to keep track of one steiner point as a starting point for mst building */

	steiner = -1;
	distance = malloc(sizeof(float *) * g->num_nodes);
	predecessor = malloc(sizeof(int *) * g->num_nodes);
	for (i = 0; i < g->num_nodes; i++) {
		distance[i] = malloc(sizeof(float) * g->num_nodes);
		predecessor[i] = malloc(sizeof(int) * g->num_nodes);
	}
	temp_graph = create_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1), false); /* complete graph */
	add_vertex(temp_graph, g->num_nodes);

	/* building distance graph ONLY for Steiner nodes */
	for (i = 0; i < g->num_nodes; i++) {
		if (get_vertex_property(g, i, VertexProperty_Type) == SteinerVertex) { /* check whether it's Steiner */
			if (steiner == -1) {
				steiner = i;
			}
			build_shortest_path_tree(g, i, distance[i], predecessor[i], NULL);
			for (j = i+1; j < g->num_nodes; j++) {
				if (get_vertex_property(g, j, VertexProperty_Type) == SteinerVertex) { /* check whether it's Steiner */
					add_edge(temp_graph, i, j, distance[i][j]);
				}
			}
		} 	
	}

	printf("distance graph\n");
	print_graph(temp_graph);
	build_minimum_spanning_tree(temp_graph, steiner, NULL);
	printf("min spanning tree of distance graph\n");
	print_graph(temp_graph);

	for (i = 0; i < temp_graph->num_nodes; i++) {
		g_hash_table_iter_init(&iter, temp_graph->nodes[i].edges);	
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			mark_edge(temp_graph, edge->v1, edge->v2, false);
		}
	}

	edge_list = NULL;
	for (i = 0; i < temp_graph->num_nodes; i++) {
		/* if the node has no edges, it's a leaf node because we're iterating through a tree */
		g_hash_table_iter_init(&iter, temp_graph->nodes[i].edges);	
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			/* tracing a path back to source */
			if (!edge->visited) {
				current = edge->v2;
				assert(current >= 0 && current < g->num_nodes);
				pred = predecessor[i][current]; 
				assert(pred >= 0 && pred < g->num_nodes);
				//add_edge(temp_graph, pred, current, distance[i][current]-distance[i][pred]);
				temp_edge = malloc(sizeof(Edge));
				temp_edge->v1 = pred;
				temp_edge->v2 = current;
				temp_edge->weight = distance[i][current]-distance[i][pred];
				edge_list = g_slist_prepend(edge_list, temp_edge);
				while (pred != i) {
					current = pred;
					pred = predecessor[i][current];
					//add_edge(temp_graph, pred, current, distance[i][current]-distance[i][pred]);

					temp_edge = malloc(sizeof(Edge));
					temp_edge->v1 = pred;
					temp_edge->v2 = current;
					temp_edge->weight = distance[i][current]-distance[i][pred];
					edge_list = g_slist_prepend(edge_list, temp_edge);
				}
				mark_edge(temp_graph, edge->v1, edge->v2, true);
			}
		}
	}

	/* remove existing edges */
	for (i = 0; i < temp_graph->num_nodes; i++) {
		g_hash_table_iter_init(&iter, temp_graph->nodes[i].edges);	
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			remove_edge(temp_graph, edge->v1, edge->v2);

			g_hash_table_iter_init(&iter, temp_graph->nodes[i].edges);	
		}
	}
	/* add new edges */
	for (item = edge_list; item != NULL; item = item->next) {
		edge = item->data;
		add_edge(temp_graph, edge->v1, edge->v2, edge->weight);
	}

	printf("mst replaced with shortest paths\n");
	print_graph(temp_graph);
	build_minimum_spanning_tree(temp_graph, steiner, NULL);
	printf("mst of mst replaced with shortest paths\n");
	print_graph(temp_graph);

}

int kmb(Graph *g, Graph *result)
{
	Graph *temp;
	Graph *mst;
	int i;	

	temp = create_graph(g->num_nodes, g->num_nodes-1, false);

	replace_with_shortest_path(g, temp);

	return 0;
}
