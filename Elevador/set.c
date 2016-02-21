/*
 *  set.c - a set as a sorted dynamic array
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#include <stdlib.h>

#include "set.h"

MBset * MBset_create(MBcmpfn cmpfn) {
    MBset *set = malloc(sizeof (MBset));
    if (set) {
        set->array = MBdynarray_create(0);
        set->cmpfn = cmpfn;
    }
    return set;
}

void MBset_delete(MBset * set) {
    if (set) {
        MBdynarray_delete(set->array);
        free(set);
    }
}

typedef struct {
    void * data;
    unsigned int pos;
} search_result;

static search_result MBset_search(const MBset *set, const void *data) {
    search_result sresult;
    unsigned int elements = set->array->count;
    unsigned int offset = 0;
    unsigned int middle = 0;

    sresult.data = NULL;
    while (elements > 0 && !sresult.data) {
        int result;
        middle = elements / 2;
        result = set->cmpfn(data, MBdynarray_get(set->array, offset + middle));
        if (result > 0) {
            offset = offset + middle + 1;
            elements = elements - (middle + 1);
        } else if (result < 0) {
            elements = middle;
        } else {
            sresult.data = MBdynarray_get(set->array, offset + middle);
            offset += middle;
        }
    }
    sresult.pos = offset;
    if (sresult.pos > MBset_get_count(set)) {
        sresult.pos--;
    }

    return sresult;
}

void * MBset_add(MBset * set, void * data) {
    search_result result = MBset_search(set, data);
    void *existing = result.data;
    if (existing) {
        /* Replace */
        MBdynarray_set(set->array, result.pos, data);
    } else {
        /* Add new */
        MBdynarray_insert(set->array, result.pos, data);
    }
    return existing;
}

void * MBset_remove(MBset * set, const void * data) {
    search_result result = MBset_search(set, data);
    if (result.data) {
        MBdynarray_remove(set->array, result.pos);
    }
    return result.data;
}

void * MBset_find(const MBset * set, const void * data) {
    search_result result = MBset_search(set, data);
    return result.data;
}

void MBset_for_each(const MBset * set, MBforfn forfn) {
    MBdynarray_for_each(set->array, forfn);
}

unsigned int MBset_get_count(const MBset *set) {
    return set->array->count;
}

void * MBset_get(const MBset *set, unsigned int pos) {
    return MBdynarray_get(set->array, pos);
}

void * MBset_remove_at(MBset * set, unsigned int pos) {
    return MBdynarray_remove(set->array, pos);
}

int MBset_find_index(const MBset *set, const void *data) {
    int index = -1;
    search_result result = MBset_search(set, data);
    if (result.data) {
        index = result.pos;
    }
    return index;
}

MBiterator * MBset_iterator(const MBset * set) {
    return MBdynarray_iterator(set->array);
}
