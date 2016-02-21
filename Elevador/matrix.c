/*
 *  matrix.c - a dynamic 2-d array of unsigned integers
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#include <stdlib.h>
#include <string.h> /* For memmove */

#include "matrix.h"

#define INCREMENT_SIZE 4

MBmatrix *MBmatrix_create(unsigned int rows, unsigned int columns)
{
	MBmatrix *matrix = malloc(sizeof(MBmatrix));
	unsigned int succeeded = 1;
	if (matrix) {
		if (rows) {
			matrix->rowspace = rows;
		}
		else {
			matrix->rowspace = INCREMENT_SIZE;
		}
		matrix->rowcount = 0;
		matrix->columncount = 0;
		matrix->columnspace = columns;
		matrix->rows = malloc(matrix->rowspace * sizeof(unsigned int*));
		succeeded = matrix->rows != NULL;
		if (succeeded) {
			unsigned int r;
			if (matrix->columnspace) {
				for (r = 0; r < matrix->rowspace && succeeded; r++) {
					matrix->rows[r] = calloc(matrix->columnspace, sizeof(unsigned int));
					succeeded = matrix->rows[r] != NULL;
				}
				if (!succeeded) {
					unsigned int r2;
					for (r2 = 0; r2 <= r; r2++) {
						free(matrix->rows[r2]);
						free(matrix->rows);
					}
				}
			}
			else {
				for (r = 0; r < matrix->rowspace; r++) {
					matrix->rows[r] = NULL;
				}
			}
		}
		if (!succeeded) {
			free(matrix);
			matrix = NULL;
		}
	}
	return matrix;
}

void MBmatrix_delete(MBmatrix *matrix)
{
	if (matrix) {
		unsigned int r;
		for (r = 0; r < matrix->rowspace; r++) {
			free(matrix->rows[r]);
		}
		free(matrix->rows);
		free(matrix);
	}
}

unsigned int MBmatrix_set(MBmatrix *matrix, unsigned int row, unsigned int column, unsigned int value)
{
	unsigned int existing = 0;
    if (row < matrix->rowcount && column < matrix->columncount) {
        existing = matrix->rows[row][column];
        matrix->rows[row][column] = value;
    }
    return existing;
}

unsigned int MBmatrix_get(const MBmatrix *matrix, unsigned int row, unsigned int column)
{
	unsigned int value = 0;
	if (row < matrix->rowcount && column < matrix->columncount) {
		value = matrix->rows[row][column];
	}
	return value;
}

static unsigned int MBmatrix_allocate_rows(MBmatrix *matrix)
{
	unsigned int succeeded = 1;
	unsigned int r;
	matrix->rowspace += INCREMENT_SIZE;
	matrix->rows = realloc(matrix->rows, matrix->rowspace * sizeof(unsigned int*));
	for (r = matrix->rowcount; r < matrix->rowspace && succeeded; r++) {
		matrix->rows[r] = calloc(matrix->columnspace, sizeof(unsigned int));
		succeeded = matrix->rows[r] != NULL;
	}
	return succeeded;
}

static unsigned int MBmatrix_allocate_columns(MBmatrix *matrix)
{
	unsigned int succeeded = 1;
	unsigned int r;
	matrix->columnspace += INCREMENT_SIZE;
	for (r = 0; r < matrix->rowspace && succeeded; r++) {
		matrix->rows[r] = realloc(matrix->rows[r], matrix->columnspace * sizeof(unsigned int));
		succeeded = matrix->rows[r] != NULL;
	}
	return succeeded;
}

void MBmatrix_add_row(MBmatrix *matrix)
{
	unsigned int succeeded = 1;
	if (matrix->rowcount == matrix->rowspace) {
		/* Allocate some more rows */
		succeeded = MBmatrix_allocate_rows(matrix);
	}
	if (succeeded) {
		matrix->rowcount++;
	}
}

void MBmatrix_add_column(MBmatrix *matrix)
{
	unsigned int r;
	unsigned int succeeded = 1;
	if (matrix->columncount == matrix->columnspace) {
		/* Allocate some more columns */
		succeeded = MBmatrix_allocate_columns(matrix);
	}
	if (succeeded) {
		for (r = 0; r < matrix->rowspace; r++) {
			matrix->rows[r][matrix->columncount] = 0;
		}
		matrix->columncount++;
	}
}

void MBmatrix_add_rowcolumn(MBmatrix *matrix)
{
	MBmatrix_add_row(matrix);
	MBmatrix_add_column(matrix);
}

void MBmatrix_insert_row(MBmatrix *matrix, unsigned int pos)
{
	if (pos == matrix->rowcount) {
		MBmatrix_add_row(matrix);
	}
	else if (pos < matrix->rowcount) {
		unsigned int succeeded = 1;
		if (matrix->rowcount == matrix->rowspace) {
			/* Allocate some more rows */
			succeeded = MBmatrix_allocate_rows(matrix);
		}
		if (succeeded) {
			unsigned int *temp = matrix->rows[matrix->rowcount];
			memset(temp, 0, matrix->columncount * sizeof(unsigned int));
			memmove(matrix->rows + pos + 1, matrix->rows + pos, (matrix->rowcount - pos) * sizeof(unsigned int*));
			matrix->rows[pos] = temp;
			matrix->rowcount++;
		}
	}
}

void MBmatrix_insert_column(MBmatrix *matrix, unsigned int pos)
{
	if (pos == matrix->columncount) {
		MBmatrix_add_column(matrix);
	}
	else if (pos < matrix->columncount) {
		unsigned int succeeded = 1;
		if (matrix->columncount == matrix->columnspace) {
			/* Allocate some more columns */
			succeeded = MBmatrix_allocate_columns(matrix);
		}
		if (succeeded) {
			unsigned int r;
			for (r = 0; r < matrix->rowcount; r++) {
				memmove(&(matrix->rows[r][pos + 1]), &(matrix->rows[r][pos]), 
					(matrix->columncount - pos) * sizeof(unsigned int));
				matrix->rows[r][pos] = 0;
			}
			matrix->columncount++;			
		}
	}
}

void MBmatrix_insert_rowcolumn(MBmatrix *matrix, unsigned int pos)
{
	MBmatrix_insert_row(matrix, pos);
	MBmatrix_insert_column(matrix, pos);
}

void MBmatrix_remove_row(MBmatrix *matrix, unsigned int row)
{
	if (row < matrix->rowcount) {
		/* Remember the row */
		void *temp = matrix->rows[row];
		memset(temp, 0, matrix->columncount * sizeof(unsigned int));
		if (row < matrix->rowcount - 1) {
			/* Move the succeeding rows forwards */
			memmove(matrix->rows + row, matrix->rows + row + 1, (matrix->rowcount - row - 1) * sizeof(unsigned int*));
			/* Put the removed row at the end */
			matrix->rows[matrix->rowcount - 1] = temp;
		}
		matrix->rowcount--;
		if (matrix->rowcount == 0) {
			matrix->columncount = 0;
		}
	}
}

void MBmatrix_remove_column(MBmatrix *matrix, unsigned int column)
{
	if (column < matrix->columncount) {
		if (column < matrix->columncount - 1) {
			unsigned int r;
			for (r = 0; r < matrix->rowcount; r++) {
				memmove(&(matrix->rows[r][column]), &(matrix->rows[r][column + 1]), (matrix->columncount - column - 1) * sizeof(unsigned int));
				matrix->rows[r][matrix->columncount - 1] = 0;
			}
		}
		matrix->columncount--;
		if (matrix->columncount == 0) {
			matrix->rowcount = 0;
		}
	}
}

void MBmatrix_remove_rowcolumn(MBmatrix *matrix, unsigned int rc)
{
    MBmatrix_remove_row(matrix, rc);
    if (MBmatrix_get_column_count(matrix)) {
        MBmatrix_remove_column(matrix, rc);
    }
}

unsigned int MBmatrix_get_row_count(const MBmatrix *matrix)
{
	return matrix->rowcount;
}

unsigned int MBmatrix_get_column_count(const MBmatrix *matrix)
{
	return matrix->columncount;
}

