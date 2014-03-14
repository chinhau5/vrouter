/*
 * graph.h
 *
 *  Created on: 25 Feb, 2014
 *      Author: chinhau5
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <glib.h>
#include <stdbool.h>

typedef struct _MstData {
	int vertex_type;
} MstData;

enum { NormalVertex, SteinerVertex };

typedef struct _Vertex {
	GHashTable *edges;
	int weight;
	bool visited;
	int *forest;
	void *data;
} Vertex;

#define GET_VERTEX_DATA(g, v, type) ((type)(g)->nodes[(v)].data)
#define SET_VERTEX_DATA(g, v) (g)->nodes[(v)].data

#define FOREACH_EDGE_BEGIN(g, v, e) { \
		GHashTableIter _iter; \
		gpointer _key, _value; \
		g_hash_table_iter_init(&_iter, (g)->nodes[(v)].edges); \
		while (g_hash_table_iter_next(&_iter, &_key, &_value)) { \
			(e) = _value; \
			if ((e)->valid) {
			
#define FOREACH_EDGE_END } } }

typedef struct _Edge {
	int v1; /* not using pointer to Vertex because realloc invalidates the addresses */
	int v2;
	float weight;
	bool visited;
	bool valid; /* for lazy deletion */
} Edge;

typedef struct _Graph {
	Vertex *nodes;
	int num_nodes;
	int num_source_sink_nodes;
	int max_nodes;
	bool directed;
/*	Edge *edges;
	int num_edges;
	int max_edges;*/
} Graph;

Graph *create_graph(int num_initial_nodes, int num_initial_edges, bool directed);
void free_graph(Graph *g);
void reset_graph(Graph *g);

void add_vertex(Graph *g, int n);
int add_one_vertex(Graph *g);
void add_edge(Graph *g, int v1, int v2, float weight);
bool add_directed_edge(Graph *g, int v1, int v2, float weight);
void remove_edge(Graph *g, int v1, int v2);
void remove_directed_edge(Graph *g, int v1, int v2);

Edge *get_edge(Graph *g, int v1, int v2);
float get_edge_weight(Graph *g, int v1, int v2);
void set_edge_weight(Graph *g, int v1, int v2, float weight);
void mark_edge(Graph *g, int v1, int v2, bool visited);
/*void set_vertex_property(Graph *g, int v, int prop, int value);*/
/*int get_vertex_property(Graph *g, int v, int prop);*/
float calculate_total_edge_weights(Graph *g);
/*void deactivate_vertex(Graph *g, int v);*/
/*void activate_vertex(Graph *g, int v);*/

void build_distance_graph(Graph *g, Graph *distance_graph);
void build_shortest_path_tree(Graph *g, int source, float **distance, int **predecessor, Graph **spt);
void build_minimum_spanning_tree(Graph *g, int source, Graph **mst);

void print_graph(Graph *g);
#define REALLOC_INC 100

#endif /* GRAPH_H_ */
