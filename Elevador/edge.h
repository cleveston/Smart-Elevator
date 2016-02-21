/*
 *  edge.h - a graph edge
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#ifndef EDGE_H
#define EDGE_H

#include "vertex.h"

typedef struct {
    MBvertex *from;
    MBvertex *to;
    int weight;
} MBedge;

MBedge * MBedge_create(MBvertex* from, MBvertex *to, int weight);
void MBedge_delete(MBedge *edge);
int MBedge_compare(const MBedge *edge1, const MBedge *edge2);
const MBvertex * MBedge_get_from(const MBedge * edge);
const MBvertex * MBedge_get_to(const MBedge * edge);
const int MBedge_get_weight(const MBedge * edge);

#endif /* EDGE_H */