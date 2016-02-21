/*
 *  matrix.h - a dynamic 2-d array of unsigned integers
 *  Copyright (C) 2010 Martin Broadhurst 
 *  www.martinbroadhurst.com
 */

#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
	unsigned int rowcount;
	unsigned int columncount;
	unsigned int rowspace;
	unsigned int columnspace;
	unsigned int **rows;
} MBmatrix;

MBmatrix *MBmatrix_create(unsigned int rows, unsigned int columns);
void MBmatrix_delete(MBmatrix *m);
unsigned int MBmatrix_set(MBmatrix *matrix, unsigned int row, unsigned int column, unsigned int value);
unsigned int MBmatrix_get(const MBmatrix *matrix, unsigned int row, unsigned int column);
void MBmatrix_add_row(MBmatrix *matrix);
void MBmatrix_add_column(MBmatrix *matrix);
void MBmatrix_add_rowcolumn(MBmatrix *matrix);
void MBmatrix_insert_row(MBmatrix *matrix, unsigned int pos);
void MBmatrix_insert_column(MBmatrix *matrix, unsigned int pos);
void MBmatrix_insert_rowcolumn(MBmatrix *matrix, unsigned int pos);
void MBmatrix_remove_row(MBmatrix *matrix, unsigned int row);
void MBmatrix_remove_column(MBmatrix *matrix, unsigned int column);
void MBmatrix_remove_rowcolumn(MBmatrix *matrix, unsigned int rc);
unsigned int MBmatrix_get_row_count(const MBmatrix *matrix);
unsigned int MBmatrix_get_column_count(const MBmatrix *matrix);

#endif /* MATRIX_H */
