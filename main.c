/*
 * main.c
 *
 *  Created on: 8 Feb, 2014
 *      Author: chinhau5
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include "heap.h"

typedef enum { SOURCE, SINK, STEINER } VertexType;

typedef struct _Vertex {
	VertexType type;
	bool visited;
	int weight;
	GHashTable *edges; /* v2 to edge array index map */
} Vertex;

typedef struct _Edge {
	int v1;
	int v2;
	float weight;
} Edge;

typedef struct _Graph {
	Vertex *nodes;
	int num_nodes;
	int num_source_sink_nodes;
	int max_nodes;

	Edge *edges;
	int num_edges;
	int max_edges;
} Graph;

#define REALLOC_INC 100

Graph *alloc_graph(int num_initial_nodes, int num_initial_edges)
{
	Graph *g;

	g = malloc(sizeof(Graph));
	g->max_nodes = num_initial_nodes;
	g->nodes = malloc(sizeof(Vertex) * g->max_nodes);
	g->num_nodes = 0;

	g->max_edges = num_initial_edges;
	g->edges = malloc(sizeof(Edge) * g->max_edges);
	g->num_edges = 0;

	return g;
}

void graph_reset(Graph *g)
{
	g->num_nodes = 0;
	g->num_edges = 0;
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

static bool edge_exists(Graph *g, int v1, int v2)
{
	GSList *l;
	Edge *edge;
	int e;
	bool exists;

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
	return g_hash_table_contains(g->nodes[v1].edges, v2);
}

void set_edge_weight(Graph *g, int v1, int v2, float weight)
{
	GSList *l;
	Edge *edge;
	int e;

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
	e = (int)g_hash_table_lookup(g->nodes[v1].edges, v2);
	edge = &g->edges[e];
	edge->weight = weight;
}

float get_edge_weight(Graph *g, int v1, int v2)
{
	Edge *edge;
	int e;

	assert(edge_exists(g, v1, v2));
	e = (int)g_hash_table_lookup(g->nodes[v1].edges, v2);
	edge = &g->edges[e];
	return edge->weight;
}

void add_edge(Graph *g, int v1, int v2, float weight)
{
	Edge *edge;

	if (g->num_edges >= g->max_edges) {
		g->max_edges += REALLOC_INC;
		g->edges = realloc(g->edges, sizeof(Edge) * g->max_edges);
	}

	assert(v1 < g->num_nodes && v2 < g->num_nodes);
	if (!edge_exists(g, v1, v2)) {
		edge = &g->edges[g->num_edges];
		edge->v1 = v1;
		edge->v2 = v2;
		edge->weight = weight;

		g_hash_table_insert(g->nodes[v1].edges, v2, g->num_edges);
		g->num_edges++;
	}
}

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

void build_min_spanning_tree(Graph *g, int source, Graph *mst)
{
	Heap heap;
	int num_v;
	GHashTableIter iter;
	gpointer key, value;
	int i, e;
	int current;
	int neighbour;
	bool visited;

	for (i = 0; i < g->num_nodes; i++) {
		g->nodes[i].visited = false;
	}
	heap_init(&heap);

	num_v = 0;
	while (num_v < g->num_nodes) {
		if (num_v == 0) {
			current = source;
		} else {
			if (heap_is_empty(&heap)) {
				assert(0);
			}
			e = heap_pop(&heap);
			printf("v1=%d v2=%d w=%f\n", g->edges[e].v1, g->edges[e].v2, g->edges[e].weight);
			current = g->edges[e].v2;
		}

		assert(!g->nodes[current].visited);

		g->nodes[current].visited = true;
		num_v++;

		g_hash_table_iter_init(&iter, g->nodes[current].edges);

		while (g_hash_table_iter_next(&iter, &key, &value)) {
			e = (int)value;
			neighbour = (int)key;
			assert(g->edges[e].v1 == current && g->edges[e].v2 == neighbour);
			if (!g->nodes[neighbour].visited) {
				heap_push(&heap, e, g->edges[e].weight);
			}
		}
	}
}

void build_shortest_path_tree(Graph *g, int source, float *distance, int *predecessor)
{
	int current;
	int neighbour;
	Heap heap;
//	float *distance;
//	int *predecessor;
	//bool *visited;
	GSList *l;
	Edge *edge;
	int i, e;
	GHashTableIter iter;
	gpointer key, value;

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
	heap_push(&heap, source, 0);
	distance[source] = 0;

	while (!heap_is_empty(&heap)) {
		current = heap_pop(&heap);

		//assert(!visited[current]);
		if (!g->nodes[current].visited) {
			printf("current: %d\n", current);
			g_hash_table_iter_init(&iter, g->nodes[current].edges);
			while (g_hash_table_iter_next(&iter, &key, &value)) {
				edge = &g->edges[(int)value];
				assert(edge->v1 == current && edge->v2 == (int)key);
				neighbour = edge->v2;

				/*if (!visited[neighbour]) {
					distance[neighbour] = distance[current] + edge->weight;
					predecessor[neighbour] = current;
					heap_push(&heap, distance[current] + edge->weight, neighbour);
					printf("neighbour push: %d cost = %f + %f\n", neighbour, distance[current], edge->weight);
				} else */if (distance[current] + edge->weight < distance[neighbour]) {
					distance[neighbour] = distance[current] + edge->weight;
					predecessor[neighbour] = current;
					heap_update(&heap, neighbour, distance[neighbour]);
					printf("neighbour update: %d\n", neighbour);
				}
			}
			g->nodes[current].visited = true;
		}
	}

//	for (i = 0; i < g->num_nodes; i++) {
//		assert(visited[i]);
//
//		add_vertex(*shortest_path_tree, 0, SOURCE);
//		if (predecessor[i] < 0) {
//			assert(i == source);
//		} else {
//			add_edge(*shortest_path_tree, source, i, distance[i]);
//		}
//		printf("i: %d distance: %f\n", i, distance[i]);
//	}

//	fflush(stdout);
//	free(distance);
//	free(predecessor);
//	free(visited);
}

void build_distance_graph(Graph *g, Graph **distance_graph)
{
	int i, j;
	float *distance;
	int *predecessor;

	distance = malloc(sizeof(float) * g->num_nodes);
	predecessor = malloc(sizeof(int) * g->num_nodes);
	*distance_graph = alloc_graph(g->num_nodes, g->num_nodes*(g->num_nodes-1)); /* complete graph */

	for (i = 0; i < g->num_nodes; i++) {
		add_vertex(*distance_graph, 0, SOURCE);
	}

	for (i = 0; i < g->num_nodes; i++) {
		build_shortest_path_tree(g, i, distance, predecessor);
		for (j = 0; j < g->num_nodes; j++) {
			if (j != i) {
				add_edge(*distance_graph, i, j, distance[j]);
			}
		}
	}

	free(distance);
	free(predecessor);
}

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
}

typedef enum { TILE = 0, CHANX, CHANY } GridGraphVertexType;


int grid_graph_lookup(int x, int y, int nx, int ny, GridGraphVertexType type)
{
	return ((y*nx)+x)*1 + type;
}

Graph *grid_graph_alloc(int nx, int ny)
{
	Graph *g;
	int x, y;
	int chanx, chany, tile, rtile, ttile;

	g = alloc_graph(nx*ny*3, 100);

	for (x = 0; x < nx; x++) {
		for (y = 0; y < ny; y++) {
			add_vertex(g, 0, SOURCE);
		}
	}
	for (x = 0; x < nx; x++) {
		for (y = 0; y < ny; y++) {
			tile = grid_graph_lookup(x, y, nx, ny, TILE);

//			chanx = grid_graph_lookup(x, y, nx, ny, CHANX);
//			chany = grid_graph_lookup(x, y, nx, ny, CHANY);
			if (x < nx-1) {
				rtile = grid_graph_lookup(x+1, y, nx, ny, TILE);
				add_edge(g, tile, rtile, 1);
				add_edge(g, rtile, tile, 1);
			}
			if (y < ny-1) {
				ttile = grid_graph_lookup(x, y+1, nx, ny, TILE);
				add_edge(g, tile, ttile, 1);
				add_edge(g, ttile, tile, 1);
			}
		}
	}

	return g;
}

void test_mst()
{
	Graph *g;
	Graph *mst;

	g = alloc_graph(4, 4);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_edge(g, 0, 1, 1);
	add_edge(g, 1, 2, 2);
	add_edge(g, 1, 3, 3);
	add_edge(g, 2, 3, 0);

	mst = alloc_graph(4, 4);

	build_min_spanning_tree(g, 0, mst);
}

void test()
{
	Graph *g;
	Graph *spt;
	const int nx = 10;
	const int ny = 10;

	g = grid_graph_alloc(nx, ny);

	g = alloc_graph(3, 3);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_edge(g, 0, 1, 1);
	add_edge(g, 1, 2, 1);
	add_edge(g, 0, 2, 5);
	add_edge(g, 0, 3, 10);
	//get_triples(g);

	//build_shortest_path_tree(g, 0, &spt);
}

//t_vertex *alloc_vertex()
//{
//	t_vertex *v;
//	v = calloc(1, sizeof(t_vertex));
//	return
//}
//
//void add_edge(t_vertex *from, t_vertex *to)
//{
//	from->edges[from->num_edges++] = to;
//}
//
//void alloc_and_load_channel(t_vertex *block)
//{
//	const int num_channels = 2; /* X and Y */
//	int i;
//
//	block->edges = malloc(sizeof(t_vertex *) * num_channels);
//	block->num_edges = 0;
//
//	for (i = 0; i < num_channels; i++) {
//		add_edge(block,
//	}
//	block->e
//}

//t_vertex **alloc_graph(int nx, int ny)
//{
//	t_vertex *nodes;
//	const int num_channels = 2;
//
//	nodes = malloc(sizeof(t_vertex *) * nx * ny * (num_channels+1));
//
//	return grid;
//}


int main()
{
	test_mst();
	return 0;
}
