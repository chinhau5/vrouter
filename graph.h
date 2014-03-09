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

#define VERTEX_NUM_PROPERTIES 16
typedef enum { VertexProperty_Type } VertexProperty;
typedef enum { NormalVertex, SteinerVertex } VertexType;

typedef struct _Vertex {
	GHashTable *edges;
	int weight;
	bool visited;
	bool active;
	int *forest;
	int properties[VERTEX_NUM_PROPERTIES];
} Vertex;

typedef struct _Edge {
	int v1;
	int v2;
	bool visited;
	float weight;
} Edge;

typedef struct _Graph {
	Vertex *nodes;
	int num_nodes;
	int num_source_sink_nodes;
	int max_nodes;
	bool directed;
//	Edge *edges;
//	int num_edges;
//	int max_edges;
} Graph;

Graph *create_graph(int num_initial_nodes, int num_initial_edges, bool directed);
void graph_reset(Graph *g);

void add_vertex(Graph *g, int n);
void set_vertex_type(Graph *g, int v, VertexType type);

void add_edge(Graph *g, int v1, int v2, float weight);
void remove_edge(Graph *g, int v1, int v2);
Edge *get_edge(Graph *g, int v1, int v2);
float get_edge_weight(Graph *g, int v1, int v2);
void mark_edge(Graph *g, int v1, int v2, bool visited);
void set_edge_weight(Graph *g, int v1, int v2, float weight);
void set_vertex_property(Graph *g, int v, int prop, int value);
int get_vertex_property(Graph *g, int v, int prop);
float calculate_total_edge_weights(Graph *g);
void deactivate_vertex(Graph *g, int v);
void activate_vertex(Graph *g, int v);

void build_distance_graph(Graph *g, Graph *distance_graph);
void build_shortest_path_tree(Graph *g, int source, float *distance, int *predecessor, Graph **spt);
void build_minimum_spanning_tree(Graph *g, int source, Graph **mst);

void print_graph(Graph *g);
#define REALLOC_INC 100

#endif /* GRAPH_H_ */
