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

typedef enum { SOURCE, SINK, STEINER } VertexType;

typedef struct _Vertex {
	VertexType type;
	bool visited;
	int weight;
	GHashTable *edges;
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

//	Edge *edges;
//	int num_edges;
//	int max_edges;
} Graph;

Graph *alloc_graph(int num_initial_nodes, int num_initial_edges);
void graph_reset(Graph *g);

void add_vertex(Graph *g, float weight, VertexType type);
void set_vertex_type(Graph *g, int v, VertexType type);

void add_undirected_edge(Graph *g, int v1, int v2, float weight);
bool add_directed_edge(Graph *g, int v1, int v2, float weight);
float get_edge_weight(Graph *g, int v1, int v2);
void set_edge_weight(Graph *g, int v1, int v2, float weight);
float calc_total_edge_weights(Graph *g);

void build_distance_graph(Graph *g, Graph *distance_graph);
void build_shortest_path_tree(Graph *g, int source, float *distance, int *predecessor, Graph *spt);
void build_min_spanning_tree(Graph *g, int source, Graph *mst);

void print_graph(Graph *g);
#define REALLOC_INC 100

#endif /* GRAPH_H_ */
