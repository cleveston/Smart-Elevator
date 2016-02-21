/*
 *  edge.c - a graph edge
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#include <stdlib.h>

#include "edge.h"

MBedge *MBedge_create(MBvertex* from, MBvertex *to, int weight) {
    MBedge *edge = malloc(sizeof (MBedge));
    if (edge) {
        edge->from = from;
        edge->to = to;
        edge->weight = weight;
    }

    return edge;
}

void MBedge_delete(MBedge *edge) {
    free(edge);
}

int MBedge_compare(const MBedge *edge1, const MBedge *edge2) {
    int result = MBvertex_compare(edge1->from, edge2->from);
    if (result == 0) {
        result = MBvertex_compare(edge1->to, edge2->to);
    }
    return result;
}

const MBvertex * MBedge_get_from(const MBedge * edge) {
    return edge->from;
}

const MBvertex * MBedge_get_to(const MBedge * edge) {
    return edge->to;
}

const int MBedge_get_weight(const MBedge * edge) {
    return edge->weight;
}