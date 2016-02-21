/*
 *  graph5.h - a graph as an incidence list
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "vertex.h"
#include "edge.h"
#include "set.h"
#include "iterator.h"

typedef struct {
    MBset * vertices;
} MBgraph5;

MBgraph5 * MBgraph5_create(void);
void MBgraph5_delete(MBgraph5 *graph);
MBvertex * MBgraph5_add(MBgraph5 *graph, const char *name, void *data);
MBvertex * MBgraph5_get_vertex(const MBgraph5 *graph, const char *name);
void * MBgraph5_remove(MBgraph5 *graph, MBvertex *vertex);
void MBgraph5_add_edge(MBgraph5 *graph, MBvertex *vertex4, MBvertex *vertex2, int weight);
void MBgraph5_remove_edge(MBgraph5 *graph, MBvertex *vertex1, MBvertex *vertex2);
unsigned int MBgraph5_get_adjacent(const MBgraph5 *graph, const MBvertex *vertex1, const MBvertex *vertex2);
MBiterator * MBgraph5_get_neighbours(const MBgraph5 *graph, const MBvertex *vertex);
MBiterator * MBgraph5_get_edges(const MBgraph5 *graph);
MBiterator * MBgraph5_get_vertices(const MBgraph5 *graph);
unsigned int MBgraph5_get_neighbour_count(const MBgraph5 * graph, const MBvertex * vertex);
unsigned int MBgraph5_get_edge_count(const MBgraph5 * graph);
unsigned int MBgraph5_get_vertex_count(const MBgraph5 * graph);

#endif /* GRAPH5_H */