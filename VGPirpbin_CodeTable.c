/**
    Copyright (C) 2009-2019 German Tischler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
#include "VGPirpbin_pre.h"
#include "VGPirpbin_CodeTable.h"
#include <stdlib.h>

CodeTable * CodeTable_allocate(size_t n)
{
	CodeTable * T = (CodeTable *)malloc(sizeof(CodeTable));

	if ( ! T )
		return NULL;

	T->n = n;
	T->A = (CodeTableEntry *)malloc(sizeof(CodeTableEntry)*n);

	if ( ! T->A )
	{
		free(T);
		return NULL;
	}

	return T;
}

CodeTable * CodeTable_sortBySymbol(CodeTable const * C)
{
	CodeTable * O = CodeTable_allocate(C->n);
	size_t i;
	if ( ! O )
		return NULL;

	for ( i = 0; i < C->n; ++i )
		O->A[i] = C->A[i];

	qsort(O->A,O->n,sizeof(CodeTableEntry),CodeTableEntry_symbolCompare);

	return O;
}

CodeTable * CodeTable_createSparse(CodeTable const * C)
{
	CodeTable * O = NULL;
	size_t i;

	if ( C->n == 0 )
		return CodeTable_allocate(0);

	O = CodeTable_allocate(C->A[C->n-1].symbol+1);
	if ( ! O )
		return NULL;

	for ( i = 0; i < O->n; ++i )
	{
		CodeTableEntry CTE;
		CTE.symbol = UINT64_MAX;
		CTE.codelength = UINT64_MAX;
		CTE.code = UINT64_MAX;
		O->A[i] = CTE;
	}

	for ( i = 0; i < C->n; ++i )
		O->A[C->A[i].symbol] = C->A[i];

	return O;
}

void CodeTable_deallocate(CodeTable * T)
{
	if ( T )
	{
		free(T->A);
		free(T);
	}
}

void CodeTable_print(FILE * out, CodeTable * C)
{
	size_t i;

	for ( i = 0; i < C->n; ++i )
	{
		fprintf(out,"%08d ",(int)i);

		if ( C->A[i].codelength != UINT64_MAX )
		{
			CodeTableEntry_print(
				out,
				&(C->A[i])
			);
		}
		else
		{
			fprintf(out,"\n");
		}
	}
}

CodeTable * CodeTable_Create(PairTable const * PT)
{
	size_t low = 0;
	size_t i;
	uint64_t nextcode = 0;
	CodeTable * CT = CodeTable_allocate(PT->n);

	for ( i = 0; i < PT->n; ++i )
		if ( PT->A[i].value > 64 )
		{
			fprintf(stderr,"[E] code length %lu exceeds 64 bits\n", (unsigned long)PT->A[i].value);
			CodeTable_deallocate(CT);
			return NULL;
		}

	while ( low < PT->n )
	{
		size_t const len = PT->A[low].value;
		size_t high = low;

		while ( high < PT->n && PT->A[high].value == PT->A[low].value )
		{
			#if 0
			size_t i;

			fprintf(stderr,"sym %05d len %05d ", (int)PT->A[high].key, (int)PT->A[high].value);

			for ( i = 0; i < len; ++i )
				fprintf(stderr,"%d", (int)(nextcode >> (len-i-1))&1);
			fprintf(stderr,"\n");
			#endif

			CodeTableEntry CTE;
			CTE.symbol = PT->A[high].key;
			CTE.codelength = len;
			CTE.code = nextcode;

			CT->A[high] = CTE;

			nextcode += 1;
			high++;
		}

		low = high;

		if ( low < PT->n )
		{
			size_t const nlen = PT->A[low].value;
			size_t const lendif = nlen - len;
			nextcode <<= lendif;
		}
	}

	return CT;
}
