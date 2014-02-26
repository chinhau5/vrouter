/*
 * test.c
 *
 *  Created on: 25 Feb, 2014
 *      Author: chinhau5
 */

#include "graph.h"

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
				add_undirected_edge(g, tile, rtile, 1);
			}
			if (y < ny-1) {
				ttile = grid_graph_lookup(x, y+1, nx, ny, TILE);
				add_undirected_edge(g, tile, ttile, 1);
			}
		}
	}

	return g;
}

void test_mst()
{
	Graph *g;
	Graph *mst;

	g = alloc_graph(4, 8);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_undirected_edge(g, 0, 1, 1);
	add_undirected_edge(g, 1, 2, 2);
	add_undirected_edge(g, 1, 3, 3);
	add_undirected_edge(g, 2, 3, 0);

	mst = alloc_graph(4, 4);

	build_min_spanning_tree(g, 0, mst);
}

void test()
{
	Graph *g;
	Graph *spt;
	const int nx = 10;
	const int ny = 10;
	float distance[1000];
	int predecessor[1000];

	g = grid_graph_alloc(nx, ny);

	g = alloc_graph(3, 3);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_vertex(g, 0, SOURCE);
	add_undirected_edge(g, 0, 1, 1);
	add_undirected_edge(g, 1, 2, 1);
	add_undirected_edge(g, 0, 2, 3);
	add_undirected_edge(g, 0, 3, 10);
	//get_triples(g);

	build_shortest_path_tree(g, 0, distance, predecessor);
}

