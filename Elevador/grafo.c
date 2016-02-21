/*
 *  graph5.c - a graph as an incidence list
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "edge.h"

#include "grafo.h"

typedef struct {
    MBset *edges;
} vertex_body;

static vertex_body * vertex_body_create(void) {
    vertex_body * body = malloc(sizeof (vertex_body));
    if (body) {
        body->edges = MBset_create((MBcmpfn) MBedge_compare);
    }
    return body;
}

static void vertex_body_delete(vertex_body * body) {
    if (body) {
        MBset_for_each(body->edges, (MBforfn) MBedge_delete);
        MBset_delete(body->edges);
        free(body);
    }
}

static MBset * vertex_get_edges(const MBvertex * vertex) {
    return ((vertex_body*) vertex->body)->edges;
}

MBgraph5 *MBgraph5_create(void) {
    MBgraph5 *graph = malloc(sizeof (MBgraph5));
    if (graph) {
        graph->vertices = MBset_create((MBcmpfn) MBvertex_compare);
    }
    return graph;
}

void MBgraph5_delete(MBgraph5 *graph) {
    if (graph) {
        MBset_for_each(graph->vertices, (MBforfn) MBvertex_delete);
        MBset_delete(graph->vertices);
        free(graph);
    }
}

MBvertex *MBgraph5_add(MBgraph5 *graph, const char *name, void *data) {
    MBvertex *vertex = MBvertex_create(name, data, vertex_body_create(), (MBdeletefn) vertex_body_delete);
    MBvertex *existing = MBset_add(graph->vertices, vertex);
    MBvertex_delete(existing);
    return vertex;
}

MBvertex *MBgraph5_get_vertex(const MBgraph5 *graph, const char *name) {
    return MBset_find(graph->vertices, &name);
}

void *MBgraph5_remove(MBgraph5 *graph, MBvertex *vertex) {
    void *data;
    unsigned int v;
    MBvertex * removed;

    /* Remove any edges pointing to this vertex */
    for (v = 0; v < MBset_get_count(graph->vertices); v++) {
        MBvertex *current = MBset_get(graph->vertices, v);
        if (current != vertex) {
            unsigned int e;
            unsigned int removed = 0;
            MBset * edges = vertex_get_edges(current);
            for (e = 0; e < MBset_get_count(edges) && !removed; e++) {
                MBedge *edge = MBset_get(edges, e);
                if (edge->to == vertex) {
                    MBset_remove_at(edges, e);
                    MBedge_delete(edge);
                    removed = 1;
                }
            }
        }
    }
    /* Remove this vertex */
    removed = MBset_remove(graph->vertices, vertex);
    if (removed) {
        data = vertex->data;
        MBvertex_delete(removed);
    }

    return data;
}

void MBgraph5_add_edge(MBgraph5 *graph, MBvertex *vertex1, MBvertex *vertex2, int weight) {
    MBedge *edge = MBedge_create(vertex1, vertex2, weight);
    MBedge *existing = MBset_add(vertex_get_edges(vertex1), edge);
    MBedge_delete(existing);
}

void MBgraph5_remove_edge(MBgraph5 *graph, MBvertex *vertex1, MBvertex *vertex2) {
    unsigned int e;
    unsigned int removed = 0;
    MBset *edges = vertex_get_edges(vertex1);

    for (e = 0; e < MBset_get_count(edges) && !removed; e++) {
        MBedge *edge = MBset_get(edges, e);
        if (edge->to == vertex2) {
            MBset_remove_at(edges, e);
            MBedge_delete(edge);
            removed = 1;
        }
    }
    if (!removed) {
        printf("%s and %s are not connected\n", (const char*) vertex1->data, (const char *) vertex2->data);
    }
}

unsigned int MBgraph5_get_adjacent(const MBgraph5 *graph, const MBvertex *vertex1, const MBvertex *vertex2) {
    unsigned int adjacent = 0;
    unsigned int e;
    const MBset *edges = vertex_get_edges(vertex1);

    for (e = 0; e < MBset_get_count(edges)&& !adjacent; e++) {
        const MBedge *edge = MBset_get(edges, e);
        adjacent = edge->to == vertex2;
    }

    return adjacent;
}

typedef struct {
    const MBset *edges;
    unsigned int e;
} neighbour_iterator;

static neighbour_iterator *neighbour_iterator_create(const MBset *edges) {
    neighbour_iterator *it = malloc(sizeof (neighbour_iterator));
    if (it) {
        it->edges = edges;
        it->e = 0;
    }
    return it;
}

static void neighbour_iterator_delete(neighbour_iterator *it) {
    free(it);
}

static void *neighbour_iterator_get(neighbour_iterator *it) {
    MBvertex *neighbour = NULL;
    if (it->e < MBset_get_count(it->edges)) {
        const MBedge *edge = MBset_get(it->edges, it->e);
        neighbour = edge->to;
        it->e++;
    }
    return neighbour;
}

MBiterator *MBgraph5_get_neighbours(const MBgraph5 *graph, const MBvertex *vertex) {
    return MBiterator_create(neighbour_iterator_create(vertex_get_edges(vertex)), (MBgetfn) neighbour_iterator_get,
            (MBdeletefn) neighbour_iterator_delete);
}

typedef struct {
    const MBgraph5 *graph;
    unsigned int v; /* Index of the current outgoing vertex */
    unsigned int e; /* Index of the current edge */
} edge_iterator;

static edge_iterator *edge_iterator_create(const MBgraph5 *graph) {
    edge_iterator *it = malloc(sizeof (edge_iterator));
    if (it) {
        it->graph = graph;
        it->v = 0;
        it->e = 0;
    }
    return it;
}

static void edge_iterator_delete(edge_iterator *it) {
    if (it) {
        free(it);
    }
}

static void *edge_iterator_get(edge_iterator *it) {
    MBedge *edge = NULL;
    MBvertex *vertex = MBset_get(it->graph->vertices, it->v);
    while (edge == NULL && vertex != NULL) {
        edge = MBset_get(vertex_get_edges(vertex), it->e);
        if (edge != NULL) {
            it->e++;
        } else {
            /* Move to the next vertex */
            it->v++;
            vertex = MBset_get(it->graph->vertices, it->v);
            it->e = 0;
        }
    }
    return edge;
}

MBiterator *MBgraph5_get_edges(const MBgraph5 *graph) {
    return MBiterator_create(edge_iterator_create(graph), (MBgetfn) edge_iterator_get,
            (MBdeletefn) edge_iterator_delete);
}

MBiterator *MBgraph5_get_vertices(const MBgraph5 *graph) {
    return MBset_iterator(graph->vertices);
}

unsigned int MBgraph5_get_neighbour_count(const MBgraph5 * graph, const MBvertex * vertex) {
    return MBset_get_count(vertex_get_edges(vertex));
}

unsigned int MBgraph5_get_edge_count(const MBgraph5 * graph) {
    /* The edge count is the sum of all of the vertices' edge counts */
    unsigned int v;
    unsigned int edges = 0;

    for (v = 0; v < MBset_get_count(graph->vertices); v++) {
        const MBvertex *vertex = MBset_get(graph->vertices, v);
        edges += MBset_get_count(vertex_get_edges(vertex));
    }

    return edges;
}

unsigned int MBgraph5_get_vertex_count(const MBgraph5 * graph) {
    return MBset_get_count(graph->vertices);
}

