/*
 * test.c
 *
 *  Created on: 25 Feb, 2014
 *      Author: chinhau5
 */

#include <stdio.h>
#include "test.h"
#include "graph.h"
#include "kmb.h"
#include "netlist.h"

typedef enum { TILE = 0, CHANX, CHANY } GridGraphVertexType;


int grid_graph_lookup(int x, int y, int nx, int ny, GridGraphVertexType type)
{
	return ((y*nx)+x)*1 + type;
}

void test_parser()
{
	parse_netlist("ex5p.net", NULL, 0, NULL, NULL, NULL);
}
	

Graph *grid_graph_alloc(int nx, int ny)
{
	Graph *g;
	int x, y;
	int chanx, chany, tile, rtile, ttile;

	g = create_graph(nx*ny*3, 100, false);

	add_vertex(g, nx*ny);
	for (x = 0; x < nx; x++) {
		for (y = 0; y < ny; y++) {
			tile = grid_graph_lookup(x, y, nx, ny, TILE);

/*			chanx = grid_graph_lookup(x, y, nx, ny, CHANX);*/
/*			chany = grid_graph_lookup(x, y, nx, ny, CHANY);*/
			if (x < nx-1) {
				rtile = grid_graph_lookup(x+1, y, nx, ny, TILE);
				add_edge(g, tile, rtile, 1);
			}
			if (y < ny-1) {
				ttile = grid_graph_lookup(x, y+1, nx, ny, TILE);
				add_edge(g, tile, ttile, 1);
			}
		}
	}

	return g;
}

void test_mst()
{
	Graph *g;
	Graph *mst;

	g = create_graph(4, 8, false);
	add_vertex(g, 4);
	add_edge(g, 0, 1, 1);
	add_edge(g, 1, 2, 2);
	add_edge(g, 1, 3, 3);
	add_edge(g, 2, 3, 0);

	mst = create_graph(4, 4, false);

	build_minimum_spanning_tree(g, 0, NULL);
	print_graph(mst);
}

void test()
{
	Graph *g;
	Graph *spt;
	const int nx = 10;
	const int ny = 10;
	float *distance;
	int *predecessor;

	g = grid_graph_alloc(nx, ny);

	g = create_graph(3, 3, false);
	add_vertex(g, 4);
	add_edge(g, 0, 1, 1);
	add_edge(g, 1, 2, 1);
	add_edge(g, 0, 2, 3);
	add_edge(g, 0, 3, 10);
	/*get_triples(g);*/

	build_shortest_path_tree(g, 0, &distance, &predecessor, &spt);
	print_graph(spt);
}

void test_kmb()
{
	Graph *g;
	Graph *result;
	const int nx = 10;
	const int ny = 10;
	float distance[1000];
	int predecessor[1000];
	int i;
	MstData *data;

	/*g = grid_graph_alloc(nx, ny);*/

	g = create_graph(3, 3, false);
	/*add_vertex(g, 4);
	add_edge(g, 0, 1, 1);
	add_edge(g, 1, 2, 1);
	add_edge(g, 0, 2, 3);
	add_edge(g, 0, 3, 10);*/
	/*get_triples(g);*/
	add_vertex(g, 9);
	add_edge(g, 0, 8, 1);	
	add_edge(g, 0, 1, 10);	
	add_edge(g, 7, 8, 0.5);	
	add_edge(g, 7, 6, 0.5);	
	add_edge(g, 6, 5, 1);	
	add_edge(g, 4, 5, 1);	
	add_edge(g, 4, 8, 1);
	add_edge(g, 1, 5, 1);
	add_edge(g, 1, 2, 8);	
	add_edge(g, 4, 2, 2);	
	add_edge(g, 3, 4, 2);	
	add_edge(g, 2, 3, 9);	
	for (i = 0; i < 9; i++) {
		data = malloc(sizeof(MstData));
		data->vertex_type = NormalVertex;
		SET_VERTEX_DATA(g, i) = data;
	}
	GET_VERTEX_DATA(g, 0, MstData *)->vertex_type = SteinerVertex;
	GET_VERTEX_DATA(g, 1, MstData *)->vertex_type = SteinerVertex;
	GET_VERTEX_DATA(g, 2, MstData *)->vertex_type = SteinerVertex;
	GET_VERTEX_DATA(g, 3, MstData *)->vertex_type = SteinerVertex;

	result = NULL;
	kmb(g, &result);
	printf("real result\n");
	print_graph(result);
	free_graph(result);
	free_graph(g);

}
