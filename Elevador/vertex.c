/*
 *  vertex.c - a graph vertex
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#include <stdlib.h>
#include <string.h>

#include "vertex.h"

MBvertex * MBvertex_create(const char *name, void *data, void *body, MBdeletefn deletefn) {
    MBvertex *vertex = malloc(sizeof (MBvertex));
    if (vertex) {
        vertex->name = strdup(name);
        vertex->data = data;
        vertex->body = body;
        vertex->deletefn = deletefn;
    }
    return vertex;
}

void MBvertex_delete(MBvertex *vertex) {
    if (vertex) {
        free(vertex->name);
        if (vertex->deletefn) {
            vertex->deletefn(vertex->body);
        }
        free(vertex);
    }
}

int MBvertex_compare(const MBvertex *vertex1, const MBvertex *vertex2) {
    return strcmp(vertex1->name, vertex2->name);
}

const char * MBvertex_get_name(const MBvertex *vertex) {
    return vertex->name;
}

void * MBvertex_get_data(const MBvertex *vertex) {
    return vertex->data;
}