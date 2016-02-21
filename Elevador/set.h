/*
 *  set.h - a set as a sorted dynamic array
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#ifndef SET_H
#define SET_H

#include "dynarray.h"
#include "iterator.h"
#include "mbcommon.h"

typedef struct {
	MBdynarray *array;
	MBcmpfn cmpfn;
} MBset;

MBset * MBset_create(MBcmpfn cmpfn);
void MBset_delete(MBset * set);
void * MBset_add(MBset * set, void * data);
void * MBset_remove(MBset * set, const void * data);
void * MBset_find(const MBset * set, const void * data);
void MBset_for_each(const MBset * set, MBforfn forfn);
unsigned int MBset_get_count(const MBset *set);
void * MBset_get(const MBset *set, unsigned int pos);
void * MBset_remove_at(MBset *set, unsigned int pos);
int MBset_find_index(const MBset *set, const void *data);
MBiterator * MBset_iterator(const MBset * set);

#endif /* SET_H */
