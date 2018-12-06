/*
 * Copyright (C) 2014 Renê de Souza Pinto. All rights reserved.
 *
 * Author: Renê S. Pinto
 *
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
#ifndef CMATCHES_H

	#define CMATCHES_H

	#include <stdio.h>
	#include <gmp.h>

	/** Verbose information */
	#define print_info(...) if(verbose) fprintf(fpout, __VA_ARGS__)

	/** Verbose information (for gmp data) */
	#define gmp_print_info(...) if(verbose) gmp_fprintf(fpout, __VA_ARGS__)

	/** pair-to-pair index */
	#define INDEX_P2P  0x01
	/** complete congruency index */
	#define INDEX_COMP 0x02


	/** Output file descriptor */
	extern FILE *fpout;

	/** Verbose parameter */
	extern char verbose;


	/** 
	 * Congruency matrix:
	 * col_name: First column with file names 
	 * matrix: The total congruency matrix itself
	 */
	typedef struct _cmatrix {
		/** columns names */
		char **col_names;
		/** the matrix */
		double **matrix;
		/** matrix size */
		unsigned long size;
		/** total congruency */
		double total;
		/** standard deviation */
		double sd;
	} cmat_t;

	/**
	 * Element structure:
	 * name Element name
	 * cluster Number of the cluster which element belongs to
	 */
	typedef struct _element {
		char *name;
		unsigned long cluster;
	} elem_t;

	/* Prototypes */
	cmat_t *create_matrix(unsigned long size);
	void destroy_matrix(cmat_t *mat);
	void zero_matrix(cmat_t *mat);
	void print_matrix(cmat_t *mat, FILE *stream);
	int elem_cmp(const void *e1, const void *e2);
	int cmpstringp(const void *p1, const void *p2);
	elem_t *read_clusterset(char *filename, unsigned long *vsize);
	elem_t *dup_clusterset(elem_t *cset, unsigned long size);
	void print_cluterset(char *filename, FILE *stream);
	void show_clustersets(const char *dirname, FILE *stream);
	void free_clusterset(elem_t *cset, unsigned long size);
	char **gen_elements_list(const char *dirname, const char *filename, unsigned long *size);
	mpz_t *factorial (unsigned long int n);
	mpz_t *single_combination (unsigned int n, unsigned int r);

#endif
