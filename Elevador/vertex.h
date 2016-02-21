/*
 *  vertex.h - a graph vertex
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#ifndef VERTEX_H
#define VERTEX_H

#include "mbcommon.h"

typedef struct {
    char * name;
    void * data;
    void * body;
    MBdeletefn deletefn;
} MBvertex;

MBvertex * MBvertex_create(const char *name, void *data, void *body, MBdeletefn deletefn);
void MBvertex_delete(MBvertex *vertex);
int MBvertex_compare(const MBvertex *vertex1, const MBvertex *vertex2);
const char * MBvertex_get_name(const MBvertex *vertex);
void * MBvertex_get_data(const MBvertex *vertex);

#endif /* VERTEX_H */