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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "cmatches.h"


/**
 * \brief Compare two cluster elements
 * \param [in] e1 Element 1
 * \param [in] e2 Element 2
 * \return int
 * \note This function should be used with qsort
 */
int elem_cmp(const void *e1, const void *e2)
{
	elem_t *elem1 = (elem_t*)e1;
	elem_t *elem2 = (elem_t*)e2;

	return (elem1->cluster - elem2->cluster);
}


/**
 * \brief compare two strings, function to be used with qsort
 * \param [in] p1 String 1
 * \param [in] p2 String 2
 * \return int
 */
int cmpstringp(const void *p1, const void *p2)
{
	return strcmp(* (char * const *) p1, * (char * const *) p2);
}


/**
 * \brief Read cluster set file
 * \param [in] filename Cluster set file name
 * \param [out] vsize The number of elements
 * \return elem_t* Vector of elements (name and cluster number) and vector size
 */
elem_t *read_clusterset(char *filename, unsigned long *vsize)
{
	FILE *fp;
	size_t i, k, cnt, fsize;
	elem_t *elements;
	char *buffer, *str;

	if ((fp = fopen(filename, "r")) == NULL) {
		perror("read_clusterset()");
		return NULL;
	} else {
		/* Read file contents */
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = malloc(sizeof(char) * fsize);
		if (buffer == NULL) {
			fclose(fp);
			perror("read_clusterset()");
			return NULL;
		} else {
			if(fread(buffer, sizeof(char), fsize, fp) < fsize) {
				fclose(fp);
				free(buffer);
				perror("read_clusterset()");
				return NULL;
			} else {
				fclose(fp);
			}
		}
	}

	/* Count the number of elements into the file */
	cnt = 0;
	for (i = 0; i < fsize; i++) {
		if (buffer[i] == ',') {
			cnt++;
		}
	}
	if (cnt <= 0) {
		fprintf(stderr, "Cluster set file is empty.\n");
		return NULL;
	}
	
	/* Read clusters */
	elements = malloc(sizeof(elem_t) * cnt);
	if (elements == NULL) {
		perror("read_clusterset()");
		free(buffer);
		return NULL;
	}

	i = k = 0;
	while(i < fsize) {
		while((buffer[i] == '\n' || buffer[i] == ' ') && i < fsize) i++;

		if (i >= fsize || k >= cnt) break;

		/* get name */
		str = &buffer[i];
		while(buffer[i] != ' ' && i < fsize) i++;

		buffer[i] = '\0';
		elements[k].name = strdup(str);
		
		str = &buffer[++i];

		/* get cluster number */
		while(buffer[i] != ',' && i < fsize) i++; 

		buffer[i] = '\0';
		elements[k].cluster = atoi(str);

		k++;
		i++;
	}

	/* Sort clusters */
	qsort(elements, k, sizeof(elem_t), elem_cmp);

	/* Return */
	free(buffer);
	*vsize = k;
	return elements;
}


/**
 * \brief Duplicate a clusterset
 * \param [in] cset Clusterset to be duplicated
 * \param [in] size Clusterset size
 * \return elem_t New duplicated clusterset
 */
elem_t *dup_clusterset(elem_t *cset, unsigned long size)
{
	elem_t *newcs;
	unsigned long i;

	if (cset == NULL || size <= 0) return NULL;

	newcs = malloc(sizeof(elem_t) * size);
	if (newcs == NULL) {
		return NULL;
	}

	for (i = 0; i < size; i++) {
		newcs[i].name = strdup(cset[i].name);
		newcs[i].cluster = cset[i].cluster;
	}

	return newcs;
}


/**
 * \brief Destroy clusterset (release allocated memory)
 * \param [in] [out] cset Clusterset
 * \param [in] size Clusterset size
 */
void free_clusterset(elem_t *cset, unsigned long size)
{
	unsigned long i;

	if (cset == NULL) return;

	for (i = 0; i < size; i++) {
		if (cset[i].name != NULL) {
			free(cset[i].name);
		}
	}
	free(cset);
	cset = NULL;
	return;
}


/**
 * \brief Print clusterset
 * \param [in] filename Clusterset file name
 * \param [out] stream Output file descriptor
 */
void print_cluterset(char *filename, FILE *stream)
{
	elem_t *cset;
	unsigned long i, j, k, nc, csize;

	/* Read clusterset */
	cset = read_clusterset(filename, &csize);

	/* Count clusters */
	i = j = nc = 0;
	while(i <= csize) {
		while(cset[j].cluster == cset[i].cluster) i++;

		nc++;

		j = i;
		i++;
	}

	fprintf(stream, "%s (%ld): ", filename, nc);

	i = j = 0;
	while(i <= csize) {
		while(cset[j].cluster == cset[i].cluster) i++;

		fprintf(stream, "{");
		for (k = j; k < i; k++) {
			if (k < (i-1)) {
				fprintf(stream, "%s, ", cset[k].name);
			} else {
				fprintf(stream, "%s}", cset[k].name);
			}
		}
		if (i < csize) {
			fprintf(stream, ", ");
		}

		j = i;
		i++;
	}
	fprintf(stream, "\n");

	free_clusterset(cset, csize);
	return;
}


/**
 * \brief Read and show clusters from each clusterset
 * \param [in] dirname Directory containing clustersets
 * \param [out] stream Output file descriptor
 */
void show_clustersets(const char *dirname, FILE *stream)
{
	DIR *dp;
	struct dirent *ep;
	unsigned long nfiles, i;
	char **filenames;
	char *cfile;

	/* Count the number of interested files into directory */
	nfiles = 0;
	dp = opendir(dirname);
	if (dp == NULL) {
		perror("show_clustersets");
		return;
	}
	while ((ep = readdir(dp))) {
		if (ep->d_type == DT_REG || ep->d_type == DT_LNK) {
			nfiles++;
		}
	}
	closedir(dp);

	/* Fill columns with file names */
	filenames = malloc(sizeof(char*) * nfiles);
	if (filenames == NULL) {
		perror("show_clustersets");
		return;
	}

	i = 0;
	dp = opendir(dirname);
	while ((ep = readdir(dp)) && i < nfiles) {
		if (ep->d_type == DT_REG || ep->d_type == DT_LNK) {
			filenames[i] = strdup(ep->d_name);
			i++;
		}
	}
	closedir(dp);
	qsort(filenames, i, sizeof(char*), cmpstringp);
	
	/* Now, read and show each clusterset */
	for (i = 0; i < nfiles; i++) {
		asprintf(&cfile, "%s/%s", dirname, filenames[i]);
		print_cluterset(cfile, stream);
		free(cfile);
		free(filenames[i]);
	}

	free(filenames);
	return;
}


/**
 * \brief Read clustersets and create the list of elements
 * \param [in] dirname Directory with clustersets
 * \param [out] filename List file name (if NULL, list file will not be generated)
 * \param [out] size Vector size (returned by the function)
 * \return char** vector with the names and the vector size
 */
char **gen_elements_list(const char *dirname, const char *filename, unsigned long *size)
{
	DIR *dp;
	struct dirent *ep;
	unsigned long nfiles, i, j, k;
	char **filenames, **elements, **enames;
	char *cfile;
	elem_t **clustersets;
	unsigned long *csizes, vsize, esize;
	FILE *fp = NULL;

	/* Create/Open output list file */
	if (filename != NULL) {
		if ((fp = fopen(filename, "w+")) == NULL) {
			perror("gen_elements_list");
			return NULL;
		}
	}

	/* Count the number of interested files into directory */
	nfiles = 0;
	dp = opendir(dirname);
	if (dp == NULL) {
		perror("gen_elements_list");
		if (fp != NULL)
			fclose(fp);
		return NULL;
	}
	while ((ep = readdir(dp))) {
		if (ep->d_type == DT_REG || ep->d_type == DT_LNK) {
			nfiles++;
		}
	}
	closedir(dp);

	/* Fill columns with file names */
	filenames = malloc(sizeof(char*) * nfiles);
	if (filenames == NULL) {
		perror("gen_elements_list");
		if (fp != NULL)
			fclose(fp);
		return NULL;
	}

	i = 0;
	dp = opendir(dirname);
	while ((ep = readdir(dp)) && i < nfiles) {
		if (ep->d_type == DT_REG || ep->d_type == DT_LNK) {
			filenames[i] = strdup(ep->d_name);
			i++;
		}
	}
	closedir(dp);

	clustersets = malloc(sizeof(elem_t*) * nfiles);
	csizes      = malloc(sizeof(unsigned long) * nfiles);
	if (clustersets == NULL || csizes == NULL) {
		perror("gen_elements_list");
		for (i = 0; i < nfiles; i++) free(filenames[i]);
		free(filenames);
		if (fp != NULL)
			fclose(fp);
		return NULL;
	}

	/* Now, read each clusterset */
	vsize = 0;
	for (i = 0; i < nfiles; i++) {
		asprintf(&cfile, "%s/%s", dirname, filenames[i]);
		
		clustersets[i] = read_clusterset(cfile, &csizes[i]);
		vsize += csizes[i];
		
		free(cfile);
		free(filenames[i]);
	}
	free(filenames);

	/* Read elements from each clusterset */
	elements = malloc(sizeof(char*) * vsize);
	if (elements == NULL) {
		perror("gen_elements_list");
		if (fp != NULL)
			fclose(fp);
		return NULL;
	}

	k = 0;
	for (i = 0; i < nfiles; i++) {
		for (j = 0; j < csizes[i]; j++) {
			elements[k] = strdup(clustersets[i][j].name);
			k++;
		}
		free_clusterset(clustersets[i], csizes[i]);
	}

	/* Sort elements */
	qsort(elements, vsize, sizeof(char*), cmpstringp);

	/* Remove duplicated entries and write to file (if necessary) */
	i = 0;
	esize = vsize;
	while (i < vsize) {
		if (fp != NULL)
			fprintf(fp, "%s\n", elements[i]);
		for (j = i+1; j < vsize; j++) {
			if (strcmp(elements[i], elements[j]) != 0) {
				break;
			} else {
				free(elements[j]);
				elements[j] = NULL;
				esize--;
			}
		}
		i = j;
	}

	/* Generate return vector */
	enames = malloc(sizeof(char*) * esize);
	if (enames == NULL) {
		perror("gen_elements_list");
		if (fp != NULL)
			fclose(fp);
		free(csizes);
	}

	j = 0;
	for (i = 0; i < vsize; i++) {
		if (elements[i] != NULL) {
			enames[j] = strdup(elements[i]);
			free(elements[i]);
			j++;
		}
	}
	free(elements);

	if (fp != NULL)
		fclose(fp);
	*size = esize;
	free(csizes);
	return enames;
}

