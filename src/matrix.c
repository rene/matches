/*
 * Copyright (C) 2014 Renê de Souza Pinto. All rights reserved.
 *
 * Author: Renê S. Pinto
 * This file is part of matches.
 *
 * Matches is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Matches is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Matches.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmatches.h"

/**
 * \brief Create a square matrix
 * \param size Matrix size
 * \param return cmat_t
 */
cmat_t *create_matrix(unsigned long size)
{
	cmat_t *mat;
	unsigned long i, j;

	mat = (cmat_t*)malloc(sizeof(cmat_t));
	if (mat == NULL) {
		return NULL;
	}

	mat->col_names = (char**)malloc(sizeof(char*) * size);
	if (mat->col_names == NULL) {
		free(mat);
		return NULL;
	}

	/* alloc lines */
	mat->matrix = (double**)malloc(sizeof(double*) * size);
	if (mat->matrix == NULL) {
		free(mat->col_names);
		free(mat);
		return NULL;
	}
	/* alloc columns */
	for (i = 0; i < size; i++) {
		mat->matrix[i] = (double*)malloc(sizeof(double) * size);
		if (mat->matrix[i] == NULL) {
			for (j = 0; j < i; j++) {
				free(mat->matrix[j]);
			}
		}
	}

	mat->size = size;

	return(mat);
}


/**
 * \brief Destroy matrix
 * \param mat Matrix
 */ 
void destroy_matrix(cmat_t *mat)
{
	unsigned long i, size;

	if (mat == NULL) {
		return;
	} else {
		size = mat->size;
	}

	free(mat->col_names);
	for (i = 0; i < size; i++) {
		free(mat->matrix[i]);
	}

	free(mat);
	mat = NULL;
}


/**
 * \brief Zero matrix (Fill the matrix with zeros)
 * \param mat Matrix
 */
void zero_matrix(cmat_t *mat)
{
	unsigned long i, j;

	if (mat == NULL) {
		return;
	}

	for (i = 0; i < mat->size; i++) {
		for (j = 0; j < mat->size; j++) {
			mat->matrix[i][j] = 0;
		}
	}
}


/**
 * \param Print the matrix
 * \param [in] mat Matrix
 * \param [out] stream File stream
 */
void print_matrix(cmat_t *mat, FILE *stream)
{
	unsigned long i, j;

	if (mat == NULL) {
		return;
	}

	for (i = 0; i < mat->size; i++) {
		fprintf(stream, "%s ", mat->col_names[i]);
		for (j = 0; j < mat->size; j++) {
			fprintf(stream, "%f ", mat->matrix[i][j]);
		}
		fprintf(stream, "\n");
	}
}

