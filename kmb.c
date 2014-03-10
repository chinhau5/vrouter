#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "graph.h"

void remove_non_steiner_leaves(Graph *g, int first_steiner)
{
	GQueue *stack;
	Edge *edge;
	int i;
	int *num_child;
	struct _pair {
		int parent;
		int child;
	} *pair;
	struct _pair **map;
	struct _pair *current;

	stack = g_queue_new();
	map = malloc(sizeof(struct _pair *) * g->num_nodes);

	pair = malloc(sizeof(struct _pair));
	pair->parent = -1;
	pair->child = first_steiner;
	g_queue_push_head(stack, pair); 
	map[pair->child] = pair;

	/*for (i = 0; i < g->num_nodes; i++) {
		FOREACH_EDGE_BEGIN(g, i, edge)
			mark_edge(g, edge->v1, edge->v2, false);
		FOREACH_EDGE_END 
	}*/

	while (!g_queue_is_empty(stack)) {
		current = g_queue_pop_head(stack);

		printf("current: %d\n", current->child);

		if (current->child != first_steiner && g_hash_table_size(g->nodes[current->child].edges) == 0) {
			/* leaf node */
			if (get_vertex_property(g, current->child, VertexProperty_Type) != SteinerVertex) {
				/* if not steiner vertex, we remove it */
				printf("removing edge %d -> %d\n", current->parent, current->child);
				remove_directed_edge(g, current->parent, current->child);
				/* now we need to check if the parent is a leaf */
				/* push it onto the stack */
				if (g_hash_table_size(g->nodes[current->parent].edges) == 0) {
					pair = map[current->parent];
					g_queue_push_head(stack, pair);
				}
			}
			/* need to check whether to pop the parent from the parent stack */
			/*num_child = g_queue_peek_head(parent_stack2);
			(*num_child)--;
			if (*num_child == 0) {
				g_queue_pop_head(parent_stack2);
				free(num_child);
			}*/
		} else {
			/*num_child = malloc(sizeof(int));
			*num_child = g_hash_table_size(g->nodes[current].edges);
			g_queue_push_head(parent_stack2, num_child);
			printf("parent: %d num_child: %d\n", current, *num_child);*/

			FOREACH_EDGE_BEGIN(g, current->child, edge)
				pair = malloc(sizeof(struct _pair));
				pair->parent = edge->v1;
				pair->child = edge->v2;
				g_queue_push_head(stack, pair); 
				map[edge->v2] = pair;

				printf("child: %d\n", edge->v2);

				if (!g->directed) {
					/* make this a directed tree */
					remove_directed_edge(g, edge->v2, edge->v1);
				}
			FOREACH_EDGE_END 
		}
	}

	g_queue_free(stack);
	free(map);
}


void quick_test()
{
	Graph *g;
	int i;
	g = create_graph(9, 8, false);
	add_vertex(g, 9);
	add_edge(g, 0, 8, 0);
	add_edge(g, 8, 4, 0);
	add_edge(g, 4, 3, 0);
	add_edge(g, 4, 2, 0);
	add_edge(g, 4, 5, 0);
	add_edge(g, 5, 1, 0);
	add_edge(g, 8, 7, 0);
	add_edge(g, 7, 6, 0);
	for (i = 0; i < g->num_nodes; i++) {
		set_vertex_property(g, i, VertexProperty_Type, NormalVertex);
	}

	set_vertex_property(g, 0, VertexProperty_Type, SteinerVertex);
	set_vertex_property(g, 1, VertexProperty_Type, SteinerVertex);
	set_vertex_property(g, 2, VertexProperty_Type, SteinerVertex);
	set_vertex_property(g, 3, VertexProperty_Type, SteinerVertex);
	remove_non_steiner_leaves(g, 0);	
}

void replace_with_shortest_path(Graph *g, Graph *result)
{
	int i, j;
	float **distance;
	int **predecessor;
	Graph *temp_graph;
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
	temp_graph = create_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1), false); /* complete graph */
	add_vertex(temp_graph, g->num_nodes);

	/* building distance graph ONLY for Steiner nodes */
	for (i = 0; i < g->num_nodes; i++) {
		if (get_vertex_property(g, i, VertexProperty_Type) == SteinerVertex) { /* check whether it's Steiner */
			if (steiner == -1) {
				steiner = i;
			}
			build_shortest_path_tree(g, i, &distance[i], &predecessor[i], NULL);
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
	printf("mst of distance graph\n");
	print_graph(temp_graph);

	for (i = 0; i < temp_graph->num_nodes; i++) {
		FOREACH_EDGE_BEGIN(temp_graph, i, edge)
			mark_edge(temp_graph, edge->v1, edge->v2, false);
		FOREACH_EDGE_END	
	}

	edge_list = NULL;
	for (i = 0; i < temp_graph->num_nodes; i++) {
		FOREACH_EDGE_BEGIN(temp_graph, i, edge)
			/* tracing a path back to source */
			if (!edge->visited) {
				current = edge->v2;
				assert(current >= 0 && current < g->num_nodes);
				pred = predecessor[i][current]; 
				assert(pred >= 0 && pred < g->num_nodes);
				/*add_edge(temp_graph, pred, current, distance[i][current]-distance[i][pred]);*/
				temp_edge = malloc(sizeof(Edge));
				temp_edge->v1 = pred;
				temp_edge->v2 = current;
				temp_edge->weight = distance[i][current]-distance[i][pred];
				edge_list = g_slist_prepend(edge_list, temp_edge);
				while (pred != i) {
					current = pred;
					pred = predecessor[i][current];
					/*add_edge(temp_graph, pred, current, distance[i][current]-distance[i][pred]);*/

					temp_edge = malloc(sizeof(Edge));
					temp_edge->v1 = pred;
					temp_edge->v2 = current;
					temp_edge->weight = distance[i][current]-distance[i][pred];
					edge_list = g_slist_prepend(edge_list, temp_edge);
				}
				mark_edge(temp_graph, edge->v1, edge->v2, true);
			}
		FOREACH_EDGE_END
	}

	/* remove existing edges */
	for (i = 0; i < temp_graph->num_nodes; i++) {
		FOREACH_EDGE_BEGIN(temp_graph, i, edge)
			remove_edge(temp_graph, edge->v1, edge->v2);

			g_hash_table_iter_init(&_iter, temp_graph->nodes[i].edges);	
		FOREACH_EDGE_END
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

	for (i = 0; i < g->num_nodes; i++) {
		set_vertex_property(temp_graph, i, VertexProperty_Type, get_vertex_property(g, i, VertexProperty_Type));
	}
	remove_non_steiner_leaves(temp_graph, steiner);
	printf("mst after removing non-steiner leaf nodes\n");
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
