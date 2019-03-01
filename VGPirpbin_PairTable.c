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
#include "VGPirpbin_PairTable.h"
#include <stdlib.h>
#include <assert.h>

PairTable * PairTable_allocate(uint64_t const n)
{
	PairTable * P = NULL;
	uint64_t i;

	if ( !(P = (PairTable *)malloc(sizeof(PairTable))) )
		return NULL;

	P->A = NULL;
	P->n = n;

	P->A = (Pair *)malloc(n * sizeof(Pair));

	if ( ! P->A )
	{
		free(P);
		return NULL;
	}

	for ( i = 0; i < n; ++i )
	{
		P->A[i].key = P->A[i].value = 0;
	}

	return P;
}

int PairTable_encode(PairTable const * P, BitLevelEncoder * BLE)
{
	size_t i;

	if ( BitLevelEncoder_encodeGamma(BLE,P->n) < 0 )
		return -1;

	for ( i = 0; i < P->n; ++i )
	{
		if ( BitLevelEncoder_encodeGamma(BLE,P->A[i].key) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeGamma(BLE,P->A[i].value) < 0 )
			return -1;
	}

	return 0;
}

int PairTable_encodeHuffmanDif(PairTable const * P, BitLevelEncoder * BLE)
{
	size_t i;
	uint64_t prevkey = 0;
	uint64_t prevval = 0;

	if ( BitLevelEncoder_encodeGamma(BLE,P->n) < 0 )
		return -1;

	for ( i = 0; i < P->n; ++i )
	{
		fprintf(stderr,"PairTable[%d]=(%d,%d)\n",(int)i,(int)P->A[i].key,(int)P->A[i].value);
	}

	for ( i = 0; i < P->n; ++i )
	{
		uint64_t const vdif = P->A[i].value - prevval;
		uint64_t kdif;

		if ( vdif )
			prevkey = 0;

		assert ( P->A[i].key   >= prevkey );
		assert ( P->A[i].value >= prevval );

		kdif = P->A[i].key   - prevkey;

		if ( BitLevelEncoder_encodeGamma(BLE,vdif) < 0 )
			return -1;
		if ( BitLevelEncoder_encodeGamma(BLE,kdif) < 0 )
			return -1;

		prevkey = P->A[i].key;
		prevval = P->A[i].value;
	}

	return 0;
}

PairTable * PairTable_decode(BitLevelDecoder * BLD)
{
	size_t i;
	PairTable * P = NULL;
	uint64_t n;

	if ( BitLevelDecoder_decodeGamma(BLD,&n) < 0 )
		return NULL;

	if ( ! (P = PairTable_allocate(n)) )
		return NULL;

	for ( i = 0; i < P->n; ++i )
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&P->A[i].key) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
		if ( BitLevelDecoder_decodeGamma(BLD,&P->A[i].value) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}
	}

	return P;
}

PairTable * PairTable_decodeHuffmanDif(BitLevelDecoder * BLD)
{
	size_t i;
	PairTable * P = NULL;
	uint64_t n;
	uint64_t prevval = 0;
	uint64_t prevkey = 0;
	uint64_t v;

	if ( BitLevelDecoder_decodeGamma(BLD,&n) < 0 )
		return NULL;

	if ( ! (P = PairTable_allocate(n)) )
		return NULL;

	for ( i = 0; i < P->n; ++i )
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}

		prevval += v;
		P->A[i].value = prevval;

		if ( v )
			prevkey = 0;

		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			PairTable_deallocate(P);
			return NULL;
		}

		prevkey += v;
		P->A[i].key = prevkey;
	}

	return P;
}

PairTable * PairTable_getPairTable(Table const * T)
{
	size_t i;
	size_t n = 0;
	PairTable * PT = NULL;

	for ( i = 0; i < T->n; ++i )
		if ( T->A[i] )
			++n;

	PT = (PairTable *)malloc(sizeof(PairTable));
	if ( ! PT )
		return NULL;

	PT->A = (Pair *)malloc(n * sizeof(Pair));
	PT->n = n;
	if ( ! PT->A )
	{
		free(PT);
		return NULL;
	}

	n = 0;
	for ( i = 0; i < T->n; ++i )
		if ( T->A[i] )
		{
			PT->A[n].key = i;
			PT->A[n].value = T->A[i];
			n += 1;
		}

	assert ( n == PT->n );

	qsort(
		PT->A,
		PT->n,
		sizeof(Pair),
		Pair_comp
	);

	return PT;
}

void PairTable_deallocate(PairTable * PT)
{
	if ( PT )
	{
		free(PT->A);
		free(PT);
	}
}
