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
#include "VGPirpbin_HuffmanCode.h"
#include <stdlib.h>
#include "VGPirpbin_HuffmanInnerTable.h"

int HuffmanCode_encodeSymbol(BitLevelEncoder * BLE, HuffmanCode * HC, uint64_t const sym)
{
	CodeTable const * CT = HC->CTsortedSparse;
	CodeTableEntry const * CTE = NULL;

	if ( sym >= CT->n )
		return -1;

	CTE = &(CT->A[sym]);

	if ( BitLevelEncoder_encode(BLE,CTE->code,CTE->codelength) < 0 )
		return -1;

	return 0;
}

int HuffmanCode_encode(BitLevelEncoder * BLE, HuffmanCode const * HC)
{
	int const r = PairTable_encodeHuffmanDif(HC->PT,BLE);

	return r;
}

HuffmanCode * HuffmanCode_decode(BitLevelDecoder * BLD)
{
	PairTable * PT = NULL;
	HuffmanCode * HC = NULL;

	if ( ! (PT = PairTable_decodeHuffmanDif(BLD)) )
		return NULL;

	if ( ! (HC = HuffmanCode_computeFromLengths(&PT)) )
		return NULL;

	return HC;
}

HuffmanCode * HuffmanCode_computeFromLengths(PairTable ** firstPT)
{
	CodeTable * firstCT = NULL;
	CodeTable * firstCTSorted = NULL;
	CodeTable * firstCTSortedSparse = NULL;
	HuffmanCode * firstHC = NULL;
	HuffmanDecodeQueueEntry * HDQE = NULL;
	HuffmanDecodeQueueEntry * H_a = NULL;
	HuffmanDecodeQueueEntry * H_e = NULL;
	unsigned int depth = 0;

	if ( ! (HDQE = HuffmanDecodeQueueEntry_allocate( 2 * (*firstPT)->n)) )
	{
		fprintf(stderr,"[E] unable to allocate HuffmanDecodeQueueEntry\n");
		goto cleanup;
	}

	if ( ! (firstCT = CodeTable_Create(*firstPT)) )
	{
		fprintf(stderr,"[E] unable to produce code table from firstPT\n");
		goto cleanup;
	}
	if ( ! (firstCTSorted = CodeTable_sortBySymbol(firstCT)) )
	{
		fprintf(stderr,"[E] unable to produce sorted code table from firstPT\n");
		goto cleanup;
	}
	if ( ! (firstCTSortedSparse = CodeTable_createSparse(firstCTSorted)) )
	{
		fprintf(stderr,"[E] unable to produce sorted sparse code table from firstPT\n");
		goto cleanup;
	}

	firstHC = HuffmanCode_allocate(*firstPT,firstCT,firstCTSorted,firstCTSortedSparse);
	if ( ! firstHC )
	{
		fprintf(stderr,"[E] unable to produce first huffman code\n");
		goto cleanup;
	}

	*firstPT = NULL;

	H_a = HDQE;
	H_e = HDQE;

	{
		HuffmanDecodeQueueEntry HD;
		HD.from = 0;
		HD.to = firstHC->CT->n;
		*(H_e++) = HD;
	}

	while ( H_a != H_e )
	{
		HuffmanDecodeQueueEntry * H_z = H_e;

		while ( H_a != H_e )
		{
			HuffmanDecodeQueueEntry HD = *(H_a);
			uint64_t low = HD.from;
			uint64_t const end = HD.to;
			HuffmanDecodeQueueEntry NHDA[2];
			uint64_t nbits = 0;
			uint64_t i;

			#if 0
			fprintf(stderr,"---- interval ----\n");
			for ( i = low; i < end; ++i )
			{
				CodeTableEntry_print(stderr,&(firstHC->CT->A[i]));
			}
			#endif

			while ( low != end )
			{
				unsigned int const bit = ((firstHC->CT->A[low].code >> (firstHC->CT->A[low].codelength - 1 - depth)) & 1);
				uint64_t high = low + 1;
				HuffmanDecodeQueueEntry NHD;

				while ( high != end && ((firstHC->CT->A[high].code >> (firstHC->CT->A[high].codelength - 1 - depth)) & 1) == bit )
					++high;

				NHD.from = low;
				NHD.to = high;
				NHDA[bit] = NHD;

				nbits += 1;
				low = high;
			}

			if ( nbits > 1 )
			{
				for ( i = 0; i < nbits; ++i )
				{
					if ( i == 0 )
						H_a->left = H_z - HDQE;
					else
						H_a->right = H_z - HDQE;

					*(H_z++) = NHDA[i];
				}
			}

			++H_a;
		}

		H_a = H_e;
		H_e = H_z;

		++depth;
	}

	firstHC->DQ = HDQE;
	HDQE = NULL;

	return firstHC;

	cleanup:
	PairTable_deallocate(*firstPT);
	HuffmanDecodeQueueEntry_deallocate(HDQE);
	*firstPT = NULL;
	return NULL;
}

HuffmanCode * HuffmanCode_allocate(
	PairTable * PT,
	CodeTable * CT,
	CodeTable * CTsorted,
	CodeTable * CTsortedSparse
)
{
	HuffmanCode * HC = (HuffmanCode *)malloc(sizeof(HuffmanCode));
	if ( ! HC )
		return NULL;

	HC->PT = PT;
	HC->CT = CT;
	HC->CTsorted = CTsorted;
	HC->CTsortedSparse = CTsortedSparse;
	HC->DQ = NULL;

	return HC;
}

int64_t HuffmanCode_decodeSymbol(HuffmanCode const * H, BitLevelDecoder * BLD)
{
	uint64_t nodeid = 0;

	while ( H->DQ[nodeid].to - H->DQ[nodeid].from > 1 )
	{
		int v;

		v = BitLevelDecoder_getBit(BLD);

		if ( v < 0 )
		{
			return -1;
		}
		else if ( v == 0 )
		{
			nodeid = H->DQ[nodeid].left;
		}
		else
		{
			nodeid = H->DQ[nodeid].right;
		}
	}

	return H->CT->A[ H->DQ[nodeid].from ].symbol;
}

void HuffmanCode_deallocate(
	HuffmanCode * HC
)
{
	if ( HC )
	{
		PairTable_deallocate(HC->PT);
		CodeTable_deallocate(HC->CT);
		CodeTable_deallocate(HC->CTsorted);
		CodeTable_deallocate(HC->CTsortedSparse);
		HuffmanDecodeQueueEntry_deallocate(HC->DQ);
		free(HC);
	}
}

int HuffmanCode_computeLengths(PairTable * PT)
{
	size_t const numinner = PT->n ? (PT->n - 1) : 0;
	HuffmanInnerTable * HUFIN = NULL;
	size_t * decQ = NULL;
	size_t lq_a = 0;
	size_t lq_e = PT->n;
	size_t in_a = 0;
	size_t in_e = 0;
	size_t * dec_a = NULL;
	size_t * dec_e = NULL;
	size_t depth = 0;
	size_t i;

	if ( ! PT->n )
	{
		return 0;
	}
	else if ( PT->n == 1 )
	{
		PT->A[0].value = 0;
		return 0;
	}

	if ( ! (HUFIN = HuffmanInnerTable_allocate(numinner)) )
		goto error;

	if ( ! (decQ = (size_t *)malloc(sizeof(size_t)*(2*PT->n-1))) )
		goto error;

	while ( (lq_e-lq_a) + (in_e-in_a) > 1 )
	{
		size_t c_0, c_1;
		size_t freq_0, freq_1;
		HuffmanInnerNode HIN;

		if ( lq_e - lq_a && in_e - in_a )
		{
			if ( PT->A[lq_a].value <= HUFIN->A[in_a].freq )
			{
				freq_0 = PT->A[lq_a].value;
				c_0 = lq_a++;
			}
			else
			{
				freq_0 = HUFIN->A[in_a].freq;
				c_0 = in_a++ + PT->n;
			}
		}
		else
		{
			if ( lq_e - lq_a )
			{
				freq_0 = PT->A[lq_a].value;
				c_0 = lq_a++;
			}
			else
			{
				freq_0 = HUFIN->A[in_a].freq;
				c_0 = in_a++ + PT->n;
			}
		}
		if ( lq_e - lq_a && in_e - in_a )
		{
			if ( PT->A[lq_a].value <= HUFIN->A[in_a].freq )
			{
				freq_1 = PT->A[lq_a].value;
				c_1 = lq_a++;
			}
			else
			{
				freq_1 = HUFIN->A[in_a].freq;
				c_1 = in_a++ + PT->n;
			}
		}
		else
		{
			if ( lq_e - lq_a )
			{
				freq_1 = PT->A[lq_a].value;
				c_1 = lq_a++;
			}
			else
			{
				freq_1 = HUFIN->A[in_a].freq;
				c_1 = in_a++ + PT->n;
			}
		}

		HIN.child_a = c_0;
		HIN.child_b = c_1;
		HIN.freq = freq_0+freq_1;
		HUFIN->A[in_e++] = HIN;
	}

	dec_a = decQ;
	dec_e = decQ;
	*(dec_e++) = (in_e-1) + PT->n;

	for (; dec_a != dec_e; depth++ )
	{
		size_t * dec_z = dec_e;

		if ( depth > 8*sizeof(uint64_t) )
		{
			fprintf(stderr,"[E] huffman tree is deeper than 64 bits");
			goto error;
		}

		while ( dec_a != dec_e )
		{
			size_t const id = *(dec_a++);

			if ( id < PT->n )
			{
				PT->A[id].value = depth;
				/* fprintf(stderr,"depth %d symbol %d\n",(int)depth,(int)PT->A[id].key); */
			}
			else
			{
				*dec_z++ = HUFIN->A[id-PT->n].child_a;
				*dec_z++ = HUFIN->A[id-PT->n].child_b;
			}
		}

		dec_a = dec_e;
		dec_e = dec_z;
	}

	for ( i = 0; i < PT->n/2; ++i )
		Pair_swap(
			&PT->A[i],
			&PT->A[PT->n-i-1]
		);

	qsort(
		PT->A,
		PT->n,
		sizeof(Pair),
		Pair_comp
	);

	free(decQ);
	HuffmanInnerTable_deallocate(HUFIN);

	return 0;

	error:
	HuffmanInnerTable_deallocate(HUFIN);
	free(decQ);

	return -1;
}
