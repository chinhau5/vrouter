/*
 * graph.c
 *
 *  Created on: 25 Feb, 2014
 *      Author: chinhau5
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "heap.h"

Graph *alloc_graph(int num_initial_nodes, int num_initial_edges)
{
	Graph *g;

	g = malloc(sizeof(Graph));
	g->max_nodes = num_initial_nodes;
	g->nodes = malloc(sizeof(Vertex) * g->max_nodes);
	g->num_nodes = 0;

//	g->max_edges = num_initial_edges;
//	g->edges = malloc(sizeof(Edge) * g->max_edges);
//	g->num_edges = 0;

	return g;
}

void graph_reset(Graph *g)
{
	g->num_nodes = 0;
	//g->num_edges = 0;
}

void add_vertex(Graph *g, float weight, VertexType type)
{
	if (g->num_nodes >= g->max_nodes) {
		g->max_nodes += REALLOC_INC;
		g->nodes = realloc(g->nodes, sizeof(Vertex) * g->max_nodes);
	}

	g->nodes[g->num_nodes].edges = g_hash_table_new(g_direct_hash, g_direct_equal);
	g->nodes[g->num_nodes].weight = weight;
	g->nodes[g->num_nodes].type = type;

	g->num_nodes++;
	if (type == SOURCE || type == SINK) {
		g->num_source_sink_nodes++;
	}
}

void set_vertex_type(Graph *g, int v, VertexType type)
{
	assert(v < g->num_nodes);
	if (g->nodes[v].type != type) {
		if (type == STEINER) {
			g->num_source_sink_nodes--;
		} else if (type == SOURCE || type == SINK) {
			g->num_source_sink_nodes++;
		}
	}
}

static inline bool edge_exists(Graph *g, int v1, int v2)
{
//	GSList *l;
//	Edge *edge;
//	int e;
//	bool exists;

//	l = g->nodes[v1].edges;
//	exists = false;
//	while (l && !exists) {
//		e = l->data;
//		edge = &g->edges[e];
//		assert(edge->v1 == v1);
//		if (edge->v2 == v2) {
//			exists = true;
//		}
//		l = l->next;
//	}
	return g_hash_table_contains(g->nodes[v1].edges, GINT_TO_POINTER(v2));
}

bool add_directed_edge(Graph *g, int v1, int v2, float weight)
{
	Edge *edge;
	bool added;

//	if (g->num_edges >= g->max_edges) {
//		g->max_edges += REALLOC_INC;
//		g->edges = realloc(g->edges, sizeof(Edge) * g->max_edges);
//	}

	assert(v1 < g->num_nodes && v2 < g->num_nodes);
	if (!edge_exists(g, v1, v2)) {
		edge = malloc(sizeof(Edge));
		edge->v1 = v1;
		edge->v2 = v2;
		edge->weight = weight;

		g_hash_table_insert(g->nodes[v1].edges, GINT_TO_POINTER(v2), edge);

		added = true;
	} else {
		added = false;
	}

	return added;
}

void add_undirected_edge(Graph *g, int v1, int v2, float weight)
{
	bool added;

	added = add_directed_edge(g, v1, v2, weight);
	if (added) {
		added = add_directed_edge(g, v2, v1, weight);
		assert(added);
	}
}

void remove_directed_edge(Graph *g, int v1, int v2)
{
	if (edge_exists(g, v1, v2)) {
		g_hash_table_remove(g->nodes[v1].edges, GINT_TO_POINTER(v2));
		assert(!edge_exists(g, v1, v2));
	}
}

void remove_undirected_edge(Graph *g, int v1, int v2)
{
	remove_directed_edge(g, v1, v2);
	remove_directed_edge(g, v2, v1);
}

float get_edge_weight(Graph *g, int v1, int v2)
{
	Edge *edge;

	assert(edge_exists(g, v1, v2));
	edge = g_hash_table_lookup(g->nodes[v1].edges, GINT_TO_POINTER(v2));
	return edge->weight;
}

void set_edge_weight(Graph *g, int v1, int v2, float weight)
{
	Edge *edge;

//	l = g->nodes[v1].edges;
//
//	while (l) {
//		edge = &g->edges[(int)l->data];
//		assert(edge->v1 == v1);
//		if (edge->v2 == v2) {
//			edge->weight = weight;
//			break;
//		}
//		l = l->next;
//	}
	assert(edge_exists(g, v1, v2));
	edge = g_hash_table_lookup(g->nodes[v1].edges, GINT_TO_POINTER(v2));
	edge->weight = weight;
}

float calc_total_edge_weights(Graph *g)
{
	int i;
	GHashTableIter iter;
	gpointer key, value;
	float total;
	Edge *edge;

	total = 0;
	for (i = 0; i < g->num_nodes; i++) {
		g_hash_table_iter_init(&iter, g->nodes[i].edges);

		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			total += edge->weight;
		}
	}

	return total;
}

void build_min_spanning_tree(Graph *g, int source, Graph *mst)
{
	Heap heap;
	int num_v;
	GSList *l;
	int i, edge_index;
	Edge *edge;
	int current;
	void *best_v;
	int neighbour;
	GHashTableIter iter;
	gpointer key, value;
	bool visited;
	float cost;

	for (i = 0; i < g->num_nodes; i++) {
		g->nodes[i].visited = false;
	}
	for (i = 0; i < g->num_nodes; i++) {
		add_vertex(mst, 0, SOURCE);
	}
	heap_init(&heap);

	for (i = 0; i < g->num_nodes; i++) {
		g_hash_table_iter_init(&iter, g->nodes[i].edges);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			heap_push(&heap, value, 0);
		}
	}

	num_v = 0;
	while (num_v < g->num_nodes) {
		heap_pop(&heap, &edge, &cost);
		if (g->nodes[edge->v1].visited && !g->nodes[edge->v2].visited) {
			add_directed_edge(mst, 
		}
	}
	/*while (num_v < g->num_nodes) {
		heap_pop(&heap, &best_v, &cost);
		if (num_v > 0) {
			add_directed_edge(mst, current, GPOINTER_TO_INT(best_v), cost);
		}
		current = GPOINTER_TO_INT(best_v);
		if (num_v == 0) {
			current = source;
		} else {
			if (heap_empty(&heap)) {
				assert(0);
			}
			heap_pop(&heap, &edge, &cost);
			printf("v1=%d v2=%d w=%f\n", edge->v1, edge->v2, edge->weight);
			assert(edge->weight == cost);
			add_directed_edge(mst, edge->v1, edge->v2, edge->weight);
			current = edge->v2;
		}

		assert(!g->nodes[current].visited);

		
//		for (l = g->nodes[current].edges; l != NULL; l = l->next) {
//			edge_index = (int)l->data;
//			edge = &g->edges[edge_index];
//			neighbour = edge->v2;
//
//			if (!g->nodes[neighbour].visited) {
//				heap_push(&heap, (void *)edge_index, edge->weight);
//			}
//		}

		g_hash_table_iter_init(&iter, g->nodes[current].edges);

		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			neighbour = GPOINTER_TO_INT(key);

			assert(edge->v1 == current && edge->v2 == neighbour);

			if (!g->nodes[neighbour].visited) {
				heap_push(&heap, GINT_TO_POINTER(neighbour), edge->weight);
			}
		}

		g->nodes[current].visited = true;
		num_v++;
	}*/

	heap_free(&heap);
}

void build_shortest_path_tree(Graph *g, int source, float *distance, int *predecessor, Graph *spt)
{
	int current;
	int neighbour;
	Heap heap;
//	float *distance;
//	int *predecessor;
	//bool *visited;
	GSList *l;
	Edge *edge;
	int i, edge_index;
	gpointer key, value;
	GHashTableIter iter;
	float cost;
	void *best_v;

	//distance = malloc(sizeof(float) * g->num_nodes);
	//predecessor = malloc(sizeof(int) * g->num_nodes);
	//visited = malloc(sizeof(bool) * g->num_nodes);
	for (i = 0; i < g->num_nodes; i++) {
		distance[i] = FLT_MAX;
		predecessor[i] = -1;
		g->nodes[i].visited = false;
		//visited[i] = false;
	}

	/* star graph has g->num_nodes - 1 edges */
	//*shortest_path_tree = alloc_graph(g->num_nodes, g->num_nodes-1);

	heap_init(&heap);

	distance[source] = 0;
	heap_push(&heap, GINT_TO_POINTER(source), distance[source]);

	while (!heap_empty(&heap)) {
		heap_pop(&heap, &best_v, &cost);
		current = GPOINTER_TO_INT(best_v);
		//assert(distance[current] == cost);

		//assert(!g->nodes[current].visited);
		//if (!g->nodes[current].visited) {
			//printf("current: %d\n", current);
//			for (l = g->nodes[current].edges; l != NULL; l = l->next) {
//				edge_index = (int)l->data;
//				edge = &g->edges[edge_index];
//				neighbour = edge->v2;
//
//				if (distance[current] + edge->weight < distance[neighbour]) {
//					distance[neighbour] = distance[current] + edge->weight;
//					predecessor[neighbour] = current;
//					heap_update(&heap, (void *)neighbour, distance[neighbour]);
//				}
//			}
			g_hash_table_iter_init(&iter, g->nodes[current].edges);
			while (g_hash_table_iter_next(&iter, &key, &value)) {
				neighbour = GPOINTER_TO_INT(key);
				edge = value;

				assert(edge->v1 == current && edge->v2 == neighbour);

				/*if (!visited[neighbour]) {
					distance[neighbour] = distance[current] + edge->weight;
					predecessor[neighbour] = current;
					heap_push(&heap, distance[current] + edge->weight, neighbour);
					printf("neighbour push: %d cost = %f + %f\n", neighbour, distance[current], edge->weight);
				} else */
				if (/*!g->nodes[neighbour].visited && */distance[current] + edge->weight < distance[neighbour]) {
					distance[neighbour] = distance[current] + edge->weight;
					predecessor[neighbour] = current;
					heap_update(&heap, GINT_TO_POINTER(neighbour), distance[neighbour]);
					//printf("neighbour update: %d\n", neighbour);
				}
			}
			//g->nodes[current].visited = true;
		//}
	}

	if (spt) {
		for (i = 0; i < g->num_nodes; i++) {
			add_vertex(spt, 0, SOURCE);
		}
		for (i = 0; i < g->num_nodes; i++) {
			if (predecessor[i] != -1) {
				add_directed_edge(spt, predecessor[i], i, distance[i]-distance[predecessor[i]]);
			}
			printf("i: %d distance: %f\n", i, distance[i]);
		}
	}

	heap_free(&heap);

//	fflush(stdout);
//	free(distance);
//	free(predecessor);
//	free(visited);
}

void build_distance_graph(Graph *g, Graph *distance_graph)
{
	int i, j;
	float *distance;
	int *predecessor;

	distance = malloc(sizeof(float) * g->num_nodes);
	predecessor = malloc(sizeof(int) * g->num_nodes);
	//*distance_graph = alloc_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1)); /* complete graph */

	for (i = 0; i < g->num_nodes; i++) {
		add_vertex(distance_graph, 0, SOURCE);
	}

	for (i = 0; i < g->num_nodes; i++) {
		build_shortest_path_tree(g, i, distance, predecessor, NULL);
		for (j = 0; j < g->num_nodes; j++) {
			if (j != i) {
				add_directed_edge(distance_graph, i, j, distance[j]);
			}
		}
	}

	free(distance);
	free(predecessor);
}

void print_graph(Graph *g)
{
	int i;
	GHashTableIter iter;
	gpointer key, value;
	Edge *edge;
	for (i = 0; i < g->num_nodes; i++) {
		g_hash_table_iter_init(&iter, g->nodes[i].edges);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			edge = value;
			assert(edge->v1 == i && edge->v2 == GPOINTER_TO_INT(key));
			printf("v1: %d v2: %d weight: %f\n", edge->v1, edge->v2, edge->weight);
		}
	}
}
