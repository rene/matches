/**
 * Copyright (C) 2014 Renê de Souza Pinto. All rights reserved.
 *
 * Author: Renê S. Pinto
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include "cmatches.h"

#define FAKE_CLUSTER 9999999

#define DEFAULT_INDEX INDEX_COMP
#define SHOW_CLUSTERS 1
#define GENERATE_LIST 1
#define SHOW_NE       0x01
#define SHOW_NP       0x02

/* Program arguments */

/** input file name */
const char *inpdir = NULL;
/** list file name */
const char *listfile = NULL;
/** New list file name */
const char *newlist = NULL;
/** output file base name */
const char *outfile = NULL;
/** Congruency index(es) to be calculated */
char cindex = 0;
/** Show clustersets */
char agroup = 0;
/** Show Np or Ne */
char show_n = 0;
/** be verbose */
char verbose = 0;


/* Prototypes */
void show_help(const char *prgname);
char **get_enames(const char *filename, unsigned long *size);
cmat_t *initialize_cmatrix(const char *dirname);
int calculate_total_congruency(cmat_t *mat, char **names, unsigned long nsize, const char *dirname, char ind, char flags);
char search_el(char *name, elem_t *clusterset, unsigned long csize);
double calculate_congruency1(char *file1, char *file2, char **names, unsigned long nsize, char flags);
double calculate_congruency2(char *file1, char *file2, char **names, unsigned long nsize, char flags);

/* Program standard output */
FILE *fpout;

/**
 * \brief Main
 */
int main(int argc, char *argv[])
{
	int c, q;
	int longindex;
	char ind;
	const char optstring[] = "hvi:l:o:L:cpgPE";
	static const struct option longOpts[] = {
		{ "help",   no_argument, NULL, 'h' },
		{ "input",  required_argument, NULL, 'i' },
		{ "list",   required_argument, NULL, 'l' },
		{ "output", required_argument, NULL, 'o' },
		{ "complete", no_argument, NULL, 'c' },
		{ "pair",     no_argument, NULL, 'p' },
		{ "group",    no_argument, NULL, 'g' },
		{ "np",       no_argument, NULL, 'P' },
		{ "ne",       no_argument, NULL, 'E' },
		{ "createlist", required_argument, NULL, 'L' },
		{ "verbose" , no_argument, NULL, 'v' },
		{ NULL,       no_argument, NULL, 0 }
	};
	cmat_t *mat;
	char **enames;
	unsigned long ecnt, i, j, n;
	double c_mean, sd, sumsqr, dev, va;

	/* Parse arguments */
	while((c = getopt_long(argc, argv, optstring, longOpts, &longindex)) != -1) {
		switch(c) {
			case 'h':
				show_help(argv[0]);
				exit(EXIT_SUCCESS);
				break;

			case 'i':
				inpdir = optarg;
				break;

			case 'l':
				listfile = optarg;
				break;

			case 'o':
				outfile = optarg;
				break;

			case 'p':
				cindex |= INDEX_P2P;
				break;

			case 'c':
				cindex |= INDEX_COMP;
				break;

			case 'g':
				agroup = SHOW_CLUSTERS;
				break;

			case 'P':
				show_n |= SHOW_NP;
				break;

			case 'E':
				show_n |= SHOW_NE;
				break;

			case 'L':
				newlist = optarg;
				break;

			case 'v':
				verbose = 1;
				break;

			default:
				break;
		}
	}

	/* Validate arguments */
	if (inpdir == NULL) {
		fprintf(stderr, "Input directory should be provided.\n");
		show_help(argv[0]);
		exit(EXIT_FAILURE);
	}
	if (listfile != NULL && newlist != NULL) {
		fprintf(stderr, "Both -l and -L cannot be used at the same time.\n");
		show_help(argv[0]);
		exit(EXIT_FAILURE);
	}
	if (cindex == 0) {
		cindex = DEFAULT_INDEX;
	} else {
		if (agroup == SHOW_CLUSTERS) {
			fprintf(stderr, "-g cannot be used with -c and/or -p at the same time.\n");
			show_help(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if ((show_n & SHOW_NP) && (show_n & SHOW_NE)) {
		fprintf(stderr, "Both -P and -E cannot be used at the same time.\n");
		show_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	
	/* Check output */
	if (outfile == NULL) {
		fpout = stdout;
	} else {
		if ((fpout = fopen(outfile, "w")) == NULL) {
			fprintf(stderr, "Could not open/create output file. Using standard output.\n");
			fpout = stdout;
		}
	}

	if (agroup == SHOW_CLUSTERS) {
		/* Show clusters of each clusterset */
		show_clustersets(inpdir, fpout);
	} else {
		/* Initialize total congruency matrix */
		mat = initialize_cmatrix(inpdir);
		if (mat == NULL) {
			fprintf(stderr, "Could not read cluster set files.\n");
			return EXIT_FAILURE;
		}

		/* Get elements names */
		if (listfile != NULL) {
			enames = get_enames(listfile, &ecnt);
		} else if (newlist != NULL) {
			print_info("Generating list file: %s\n", newlist);
			enames = gen_elements_list(inpdir, newlist, &ecnt);
		} else {
			print_info("Generating clusters elements list\n");
			enames = gen_elements_list(inpdir, NULL, &ecnt);
		}
		if (enames == NULL) {
			fprintf(stderr, "Could not get elements names.\n");
			destroy_matrix(mat);
			return EXIT_FAILURE;
		}

		q = 0;
		while(q < 2) {
			zero_matrix(mat);

			if (q == 0) {
				if ((cindex & INDEX_P2P)) {
					ind = INDEX_P2P;
					fprintf(fpout, "============= pair-to-pair congruency (h) =============\n");
				} else {
					q++;
					continue;
				}
			}
			
			if (q == 1) {
				if ((cindex & INDEX_COMP)) {
					ind = INDEX_COMP;
					fprintf(fpout, "============== complete congruency index ==============\n");
				} else {
					break;
				}
			}

			/* Calculate congruences */
			calculate_total_congruency(mat, enames, ecnt, inpdir, ind, show_n);

			/* Calculate total mean and standard deviation */
			c_mean = n = 0;
			for (i = 0; i < mat->size; i++) {
				for (j = i+1; j < mat->size; j++) {
					c_mean += mat->matrix[i][j];
					n++;
				}
			}
			c_mean = c_mean / (double)n;

			sumsqr = 0;
			for (i = 0; i < mat->size; i++) {
				for (j = i+1; j < mat->size; j++) {
					dev     = mat->matrix[i][j] - c_mean;
					sumsqr += (dev * dev);
				}
			}
			if (n == 1) {
				sd = 0;
			} else {
				va = sumsqr / (double)(n-1);
				sd = sqrt(va);
			}

			/* Print results */
			print_matrix(mat, fpout);
			fprintf(fpout, "---------------------------------------\n");
			fprintf(fpout, "Total mean         = %f\n", c_mean);
			fprintf(fpout, "Standard deviation = %f\n", sd);
			fprintf(fpout, "---------------------------------------\n\n");

			q++;
		}

		destroy_matrix(mat);
	}

	if (fpout != stdout) {
		fclose(fpout);
	}

	return EXIT_SUCCESS;
}

/**
 * \brief Show program help
 * \param [in] prgname Program's name
 */
void show_help(const char *prgname)
{
	printf("Use: %s [options]\n", prgname);
	printf("Options:\n");
	printf("    -h | --help        Show this help and exit\n");
	printf("    -i | --input       Input directory\n");
	printf("    -l | --list        Input list file\n");
	printf("    -c | --complete    Calculate complete congruency\n");
	printf("    -p | --pair        Calculate pair-to-pair congruency\n");
	printf("    -g | --group       Just show clusterset clusters (groups)\n");
	printf("    -P | --np          Show congruency matrix with Np\n");
	printf("    -E | --ne          Show congruency matrix with Ne\n");
	printf("    -L | --createlist  Create elements list from clustersets\n");
	printf("    -o | --output      Write results to output file\n");
	printf("    -v | --verbose     Be verbose\n");
}


/**
 * \brief Initialize total congruency matrix
 * \param [in] dirname Path to cluster set files
 * \return cmat_t*
 */
cmat_t *initialize_cmatrix(const char *dirname)
{
	DIR *dp;
	struct dirent *ep;
	unsigned long nfiles, i, j;
	char **filenames;
	cmat_t *mat = NULL;


	/* Count the number of interested files into directory */
	nfiles = 0;
	dp = opendir(dirname);
	if (dp == NULL) {
		perror("initialize_cmatrix");
		return NULL;
	}
	while ((ep = readdir(dp))) {
		if (ep->d_type == DT_REG || ep->d_type == DT_LNK) {
			nfiles++;
		}
	}
	closedir(dp);

	/* Initialize the matrix */
	mat = create_matrix(nfiles);

	/* Fill columns with file names */
	filenames = malloc(sizeof(char*) * nfiles);
	if (filenames == NULL) {
		perror("initialize_cmatrix");
		destroy_matrix(mat);
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
	qsort(filenames, i, sizeof(char*), cmpstringp);
	mat->col_names = filenames;

	for (i = 0; i < mat->size; i++) {
		for (j = 0; j < mat->size; j++) {
			mat->matrix[i][j] = 0;
		}
	}

	return mat;
}


/**
 * \brief Initialize elements congruency matrix from list file
 * \param [in] filename List file name
 * \param [out] size Vector size
 * \return char** Vector with elements names
 */
char **get_enames(const char *filename, unsigned long *size)
{
	FILE *fp;
	char *file_contents, *str;
	unsigned long i, fsize, msize;
	char **names;

	if ((fp = fopen(filename, "r")) == NULL) {
		perror("initialize_matrix()");
		return NULL;
	}
	
	/* Read file contents */
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	file_contents = malloc(sizeof(char) * fsize);
	if (file_contents == NULL) {
		fclose(fp);
		perror("initialize_matrix()");
		return NULL;
	} else {
		if(fread(file_contents, sizeof(char), fsize, fp) < fsize) {
			fclose(fp);
			free(file_contents);
			perror("initialize_matrix()");
			return NULL;
		} else {
			fclose(fp);
		}
	}

	/* Initialize matrix */
	msize = 0;
	for (i = 0; i < fsize; i++) {
		if (file_contents[i] == '\n') {
			msize++;
		}
	}
	names = malloc(sizeof(char*) * msize);
	if (names == NULL) {
		perror("initialize_matrix()");
		free(file_contents);
		return NULL;
	}

	/* Fill columns names */
	msize = 0;
	str   = file_contents;
	for (i = 0; i < fsize; i++) {
		if (file_contents[i] == '\n') {
			file_contents[i] = '\0';
			names[msize] = str;
			str = &file_contents[i+1];
			msize++;
		}
	}

	*size = msize;
	return names;
}


/**
 * \brief Calculate total congruency
 * \param [out] mat Total congruency matrix
 * \param [out] names Elements names
 * \param [in] dirname Path to cluster set files
 * \param [in] indx Which index should be calculated
 * \param [in] flags Flags to show Np or Ne
 * \return int
 */

int calculate_total_congruency(cmat_t *mat, char **names, unsigned long nsize, const char *dirname, char ind, char flags)
{
	unsigned long i, j;
	char *file1, *file2;
    double (*index)(char*, char *, char **, unsigned long, char);

	if (mat == NULL || names == NULL) {
		return -1;
	}
	
	switch(ind) {
		case INDEX_P2P:
			index = calculate_congruency1;
			break;

		case INDEX_COMP:
			index = calculate_congruency2;
			break;

		default:
			index = calculate_congruency1;
	}
	
	
	for (i = 0; i < mat->size; i++) {
		mat->matrix[i][i] = 1.0;
		for (j = (i+1); j < mat->size; j++) {
		//for (j = 0; j < mat->size; j++) {
			asprintf(&file1, "%s/%s", dirname, mat->col_names[i]);
			asprintf(&file2, "%s/%s", dirname, mat->col_names[j]);
			
			mat->matrix[i][j] = index(file1, file2, names, nsize, flags);
			mat->matrix[j][i] = mat->matrix[i][j];
			
			free(file1);
			free(file2);
		}
	}

	return 0;
}


/**
 * \brief Search for an element on the cluster set
 * \param [in] name 
 * \param [in] clusterset
 * \param [in] csize Clusterset size
 * return char 1 if element was found, 0 otherwise
 */
char search_el(char *name, elem_t *clusterset, unsigned long csize)
{
	unsigned long i;
	for (i = 0; i < csize; i++) {
		if (strcmp(name, clusterset[i].name) == 0) {
			return 1;
		}
	}
	return 0;
}


/**
 *  Calculate pair-to-pair congruency (h) between two cluster sets
 *  \param [in] file1 Cluster set file 1
 *  \param [in] file2 Cluster set file 2
 *  \param [in] flags Flags to show Np or Ne
 *  \return double Congruency
 */
double calculate_congruency1(char *file1, char *file2, char **names, unsigned long nsize, char flags)
{
	unsigned long i, j, k, l;
	unsigned long csize1, csize2, csizeA, csizeB;
	unsigned long common_el;
	mpz_t *T;
	mpz_t maxNp, Ne, Np[2];
	mpf_t NeNp, div;
	elem_t *cset1, *cset2;
	elem_t *csetA, *csetB;
	char *fileA, *fileB, *e1, *e2;
	int x;
	double h;

	cset1 = read_clusterset(file1, &csize1);
	cset2 = read_clusterset(file2, &csize2);
	if (cset1 == NULL || cset2 == NULL) {
		perror("calculate_congruency1()");
		return -1;
	}

	mpz_init(Ne);
	mpz_init(maxNp);
	mpz_init(Np[0]);
	mpz_init(Np[1]);

	/* Here we first analyzes clusterset1 X clusterset2, and then,
	   on the second iteration we make clusterset2 X clusterset1 analyzes */
	csetA  = cset1;
	csizeA = csize1;
	fileA  = file1;
	csetB  = cset2;
	csizeB = csize2;
	fileB  = file2;
	for (x = 0; x < 2; x++) {
		print_info("\n=========================================\n");
		print_info("Clustersets: %s X %s\n", fileA, fileB);

		/* analyzes first cluster set */
		i = k = 0;
		mpz_set_ui(Np[x], 0);
		while(i <= csizeA) {
			while((csetA[k].cluster == csetA[i].cluster) && i < csizeA) i++;

			/* Check elements of the cluster which is common to the elements of other cluster set */
			print_info("\n================\n");
			print_info("Cluster found:\n");
		
			common_el = 0;
			for (j = k; j < i; j++) {
				if (search_el(csetA[j].name, csetB, csizeB) == 1) {
					common_el++;
				}
				print_info("    %10s | %ld\n", csetA[j].name, csetA[j].cluster);
			}
			T = single_combination(common_el, 2);
			mpz_add(Np[x], Np[x], *T);
			k = i;
			i++;

			print_info("----------------\n");
			print_info("C. Elements = %ld\n", common_el);
			gmp_print_info("         Nk = %Zd\n", *T);
			print_info("----------------\n");

			mpz_clear(*T);
			free(T);
		}
		gmp_print_info("T[%d] = %Zd\n", x, Np[x]);
	
		/* switch clustersets */
		csetA  = cset2;
		csizeA = csize2;
		fileA  = file2;
		csetB  = cset1;
		csizeB = csize1;
		fileB  = file1;
	}

	if (mpz_cmp(Np[0], Np[1]) > 0) {
		mpz_set(maxNp, Np[0]);
	} else {
		mpz_set(maxNp, Np[1]);
	}

	if ((flags & SHOW_NP)) {
		return mpz_get_d(maxNp);
	}

	/* Calculate common clusters, i.e., present in both clustersets */
	mpz_set_ui(Ne, 0);
	for (i = 0; i < csize1; i++) {
		e1 = cset1[i].name;
		for (j = i+1; j < csize1; j++) {
			/* We test all possible pair combination in clusterset 1 */
			e2 = cset1[j].name;

			/* are they on the same cluster ? */
			if (cset1[i].cluster == cset1[j].cluster) {
				/* Yes, lets try to find same pair on clusterset 2 */
				for (k = 0; k < csize2; k++) {
					for (l = k+1; l < csize2; l++) {
						if (cset2[k].cluster == cset2[l].cluster) {
							if (strcmp(e1, cset2[k].name) == 0 &&
									strcmp(e2, cset2[l].name) == 0) {
								mpz_add_ui(Ne, Ne, 1);
							}
						}
					}
				}
			}
		}
	}

	if ((flags & SHOW_NE)) {
		return mpz_get_d(Ne);
	}

	/* Finnaly, calculate congruency */
	if (mpz_cmp_ui(maxNp, 0) != 0) {
		mpf_init(NeNp);
		mpf_init(div);

		mpf_set_z(NeNp, Ne);
		mpf_set_z(div, maxNp);

		/* NeNp = NeNp / div */
		mpf_div(NeNp, NeNp, div);

		h = mpf_get_d(NeNp);

		mpf_clear(NeNp);
		mpf_clear(div);
	} else {
		h = 0;
	}

	print_info("============= pair-to-pair congruency (h) =============\n");
	gmp_print_info("               Ne = %Zd\n", Ne);
	gmp_print_info("max{T[1], T[2]} = %Zd\n", maxNp);
	print_info("                h = %f\n",  h);
	print_info("\n\n");

	mpz_clear(Ne);
	mpz_clear(maxNp);
	mpz_clear(Np[0]);
	mpz_clear(Np[1]);

	free_clusterset(cset1, csize1);
	free_clusterset(cset2, csize2);

	return h;
}


/**
 *  \brief Calculate complete congruency index between two cluster sets
 *  \param [in] file1 Cluster set file 1
 *  \param [in] file2 Cluster set file 2
 *  \param [in] flags Flags to show Np or Ne
 *  \return double Congruency
 */
double calculate_congruency2(char *file1, char *file2, char **names, unsigned long nsize, char flags)
{
	unsigned long i, j, k, l, p, q;
	unsigned long csize1, csize2, csizeA, csizeB, csizeC;
	unsigned long common_el, num_el;
	mpz_t A, *c, Np[2], Ne, maxNp;
	mpf_t NeNp, div;
	elem_t *cset1, *cset2;
	elem_t *csetA, *csetB;
	elem_t *caux;
	char *fileA, *fileB, **elements;
	int x;
	double h;
	char is_common;

	cset1 = read_clusterset(file1, &csize1);
	cset2 = read_clusterset(file2, &csize2);
	if (cset1 == NULL || cset2 == NULL) {
		perror("calculate_congruency2()");
		return -1;
	} 
	
	mpz_init(A);
	mpz_init(Np[0]);
	mpz_init(Np[1]);
	mpz_init(Ne);
	mpz_init(maxNp);

	/* Here we first analyzes clusterset1 X clusterset2, and then,
	   on the second iteration we make clusterset2 X clusterset1 analyzes */
	csetA  = cset1;
	csizeA = csize1;
	fileA  = file1;
	csetB  = cset2;
	csizeB = csize2;
	fileB  = file2;
	for (x = 0; x < 2; x++) {
		print_info("\n=========================================\n");
		print_info("Clustersets: %s X %s\n", fileA, fileB);

		/* We copy all elements from clusterset2 to count common elements without repetition */
		elements = malloc(sizeof(char*) * csizeB);
		if (elements == NULL) {
			perror("calculate_congruency2()");
			return -1;
		}
		for (i = 0; i < csizeB; i++) {
			elements[i] = strdup(csetB[i].name);
		}

		/* analyzes first cluster set */
		i = k = 0;
		mpz_set_ui(Np[x], 0);
		while(i <= csizeA) {
			while((csetA[k].cluster == csetA[i].cluster) && i < csizeA) i++;

			/* Check elements of the cluster which is common to the elements of other cluster set */
			print_info("\n================\n");
			print_info("Cluster found:\n");
		
			common_el = 0;
			for (j = k; j < i; j++) {
				for (l = 0; l < csizeB; l++) {
					if (elements[l] != NULL && csetA[j].name != NULL) {
						if (strcmp(csetA[j].name, elements[l]) == 0) {
							common_el++;
							free(elements[l]);
							elements[l] = NULL;
							break;
						}
					}
				}
				print_info("    %10s | %ld\n", csetA[j].name, csetA[j].cluster);
			}
			num_el = (i - k);

			/* Calculate A */
			mpz_set_ui(A, 0);
			for (l = 1; l <= common_el; l++) {
				c = single_combination(common_el, l);
				mpz_add(A, A, *c);

				gmp_print_info("(%ld/%ld)=%Zd ", common_el, l, *c);

				mpz_clear(*c);
				free(c);
			}
			print_info("\n");
			mpz_add(Np[x], Np[x], A);
			k = i;
			i++;

			print_info("----------------\n");
			print_info("C. Elements = %ld\n", common_el);
			gmp_print_info("          A = %Zd\n", A);
			print_info("----------------\n");
		}
		for (l = 0; l < csizeB; l++) {
			if (elements[l] != NULL) {
				free(elements[l]);
			}
		}
		free(elements);

		gmp_print_info("Np = %Zd\n", Np[x]);
	
		/* switch clustersets */
		csetA  = cset2;
		csizeA = csize2;
		fileA  = file2;
		csetB  = cset1;
		csizeB = csize1;
		fileB  = file1;
	}

	if (mpz_cmp(Np[0], Np[1]) > 0) {
		mpz_set(maxNp, Np[0]);
	} else {
		mpz_set(maxNp, Np[1]);
	}

	if ((flags & SHOW_NP)) {
		return mpz_get_d(maxNp);
	}


	/* Ne calculation:
	 *	- First we copy both clustersets
	 *	- Each clusterset is analysed, removing non common elements
	 *	- Only common elements will remain on each clusterset
	 *  - We count the congruency for each cluster
	 */
	print_info("\n-------------- Ne ---------------\n");

	csizeA = csize1; 
	csetA  = dup_clusterset(cset1, csize1);
	csizeB = csize2;
	csetB  = dup_clusterset(cset2, csize2);

	for (x = 0; x < 2; x++) {
		/* We copy all elements from clusterset2 to count common elements without repetition */
		elements = malloc(sizeof(char*) * csizeB);
		if (elements == NULL) {
			perror("calculate_congruency2()");
			return -1;
		}
		for (i = 0; i < csizeB; i++) {
			elements[i] = strdup(csetB[i].name);
		}

		i = k = 0;
		while(i <= csizeA) {
			while((csetA[k].cluster == csetA[i].cluster) && i < csizeA) i++;
		
			for (p = k; p < i; p++) {
				is_common = 0;
				for (l = 0; l < csizeB; l++) {
					if (elements[l] != NULL) {
						if (strcmp(csetA[p].name, elements[l]) == 0) {
							free(elements[l]);
							elements[l] = NULL;
							is_common = 1;
							break;
						}
					}
				}
				if (!is_common) {
					/* Mark element to be removed */
					free(csetA[p].name);
					csetA[p].name = NULL;
					csetA[p].cluster = FAKE_CLUSTER;
				}

			}
			k = i;
			i++;
		}
		qsort(csetA, csizeA, sizeof(elem_t), elem_cmp);
		k = csizeA;
		for (i = (k-1); i > 0; i--) {
			if (csetA[i].name == NULL && csetA[i].cluster == FAKE_CLUSTER) {
				csizeA--;
			}
		}
		if (csizeA < k) {
			caux = realloc(csetA, sizeof(elem_t) * csizeA); 
			if (caux == NULL) {
				perror("calculate_congruency2()");
			} else {
				csetA = caux;
			}
		}
		
		for (i = 0; i < csizeB; i++) {
			if (elements[i] != NULL) {
				free(elements[i]);
			}
		}
		free(elements);

		caux   = csetA;
		csizeC = csizeA;
		csetA  = csetB;
		csizeA = csizeB;
		csetB  = caux;
		csizeB = csizeC;
	}

	/* Finally, we just count congruences */
	i = k = 0;
	mpz_set_ui(Ne, 0);
	while(i <= csizeA) {
		while((csetA[k].cluster == csetA[i].cluster) && i < csizeA) i++;

		print_info("C1 cluster: ");
		for (p = k; p < i; p++) {
			print_info("%s ", cset1[p].name);
		}
		print_info("\n------\n");

		j = l = 0;
		mpz_set_ui(A, 0);
		while(j <= csizeB) {
			while((csetB[l].cluster == csetB[j].cluster) && j < csizeB) j++;

			/* Count common elements */
			num_el = 0;
			for (p = l; p < j; p++) {
				for (q = k; q < i; q++) {
					if (csetA[q].name != NULL) {
						if (strcmp(csetB[p].name, csetA[q].name) == 0) {
							num_el++;
							free(csetA[q].name);
							csetA[q].name = NULL;
							break;
						}
					}
				}

				print_info("C2: %s | %ld\n", csetB[p].name, csetB[p].cluster);
			}
			if (num_el == 1) {
				if((i-k) > 1 && (j-l) > 1) {
					num_el = 0;
				} else if ((i-k) != (j-l)) {
					num_el = 0;
				}
			}

			print_info("Common: %ld\n", num_el);
			for (q = 1; q <= num_el; q++) {
				c  = single_combination(num_el, q);
				mpz_add(A, A, *c);
			
				gmp_print_info("(%ld/%ld)=%Zd ", num_el, q, *c);
				
				mpz_clear(*c);
				free(c);
			} print_info("\n\n");

			l = j;
			j++;
		}
	
		mpz_add(Ne, Ne, A);

		k = i;
		i++;
	}

	if ((flags & SHOW_NE)) {
		return mpz_get_d(Ne);
	}

	/* Finally, calculate congruency */
	if (mpz_cmp_ui(maxNp, 0) != 0) {
		mpf_init(NeNp);
		mpf_init(div);

		mpf_set_z(NeNp, Ne);
		mpf_set_z(div, maxNp);

		/* NeNp = NeNp / div */
		mpf_div(NeNp, NeNp, div);

		h = mpf_get_d(NeNp);

		mpf_clear(NeNp);
		mpf_clear(div);
	} else {
		h = 0;
	}

	print_info("============= complete congruency (h) =============\n");
	gmp_print_info("               Ne = %Zd\n", Ne);
	gmp_print_info("max{Np[1], Np[2]} = %Zd\n", maxNp);
	print_info("               h2 = %f\n",  h);
	print_info("\n\n");

	mpz_clear(A);
	mpz_clear(Np[0]);
	mpz_clear(Np[1]);
	mpz_clear(Ne);
	mpz_clear(maxNp);

	free_clusterset(csetA, csizeA);
	free_clusterset(csetB, csizeB);
	free_clusterset(cset1, csize1);
	free_clusterset(cset2, csize2);

	return h;
}

