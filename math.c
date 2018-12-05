/**
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
#include "cmatches.h"
#include <stdlib.h>
#include <gmp.h>

/**
 * \brief Factorial
 * \param [in] n Number
 * \return mpz_t
 */
mpz_t *factorial (unsigned long int n)
{
	unsigned long int i;
	mpz_t *f;

	f = malloc(sizeof(mpz_t*));
	if (!f) return NULL;
	
	mpz_init(*f);
	mpz_set_ui(*f, 1);

	for (i = n; i >	0; i--) {
		mpz_mul_ui(*f, *f, i);
	}

	return f;
}


/**
 * \brief Calculate single combination 
 * \param [in] n
 * \param [in] r
 * \return mpz_t
 * \note See http://pt.wikipedia.org/wiki/Arranjo_%28matem%C3%A1tica%29#Combina.C3.A7.C3.A3o_simples 
 */
mpz_t *single_combination (unsigned int n, unsigned int r)
{
	mpz_t *comb, *nf, *rf, *nrf;
	mpz_t a;
	
	comb = malloc(sizeof(mpz_t*));
	if (!comb) return NULL;
	
	mpz_init(*comb);

	if (n < r)
		return comb;
		
	nf  = factorial(n);
	rf  = factorial(r);
	nrf = factorial(n-r);

	mpz_init(a);
	mpz_mul(a, *rf, *nrf);
	mpz_cdiv_q(*comb, *nf, a);

	mpz_clear(a);

	mpz_clear(*nf);
	mpz_clear(*rf);
	mpz_clear(*nrf);
	free(nf);
	free(rf);
	free(nrf);

	return comb;
}

