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
#include "VGPirpbin_getQualityCode.h"
#include "VGPirpbin_LineBuffer.h"
#include "VGPirpbin_CString.h"
#include "VGPirpbin_expect.h"

QualityHuffman * getQualityCode(FILE * in, Table const * qualTable, int64_t const maxlines)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	CString CS;
	Table * firstTable = NULL;
	Table * difTable = NULL;
	PairTable * firstPT = NULL;
	PairTable * difPT = NULL;
	HuffmanCode * firstHC = NULL;
	HuffmanCode * difHC = NULL;
	size_t nr = 0;
	uint64_t fs;
	double dfs;
	QualityHuffman * QH = NULL;
	int64_t ql = 0;

	FSEEK(in,0,SEEK_END);
	fs = FTELL(in);
	FSEEK(in,0,SEEK_SET);
	dfs = fs;

	LB = LineBuffer_allocate(in,1);

	if ( ! LB )
	{
		fprintf(stderr,"[E] failed to instantiate line buffer\n");
		goto cleanup;
	}

	if ( ! (firstTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate firstTable\n");
		goto cleanup;
	}

	if ( ! (difTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate firstTable\n");
		goto cleanup;
	}


	if ( LineBuffer_getline(LB,&a,&e) <= 0 )
	{
		fprintf(stderr,"[E] unable to get first line of file\n");
		goto cleanup;
	}
	if ( ! expect(&a,&e,'1') )
	{
		fprintf(stderr,"[E] first line of file does not start with a 1\n");
		goto cleanup;
	}
	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		goto cleanup;
	}

	CS = CString_getString(&a,&e);

	if ( ! CS.a )
	{
		fprintf(stderr,"[E] unable to read file type\n");
		goto cleanup;
	}

	if ( (CS.e - CS.a) != (ptrdiff_t)(strlen(ft)) )
	{
		fprintf(stderr,"[E] file type has the wrong length\n");
		goto cleanup;
	}

	if ( strncmp(CS.a,ft,CS.e-CS.a) != 0 )
	{
		fprintf(stderr,"[E] unexpected file type\n");
		goto cleanup;
	}

	while ( LineBuffer_getline(LB,&a,&e) && ql < maxlines )
	{
		if ( e-a && a[0] == 'Q' )
		{
			CString QS;
			char const * c;
			int64_t first;
			int64_t prev;

			a += 1;

			if ( ! expect(&a,&e,' ') )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;
			}

			QS = CString_getString(&a,&e);

			if ( ! QS.a )
			{
				fprintf(stderr,"[E] malformed Q line\n");
				goto cleanup;
			}

			for ( c = QS.a; c < QS.e; ++c )
				if ( *c < VGPIRPBIN_PHREDSHIFT || *c > 93 )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					goto cleanup;
				}

			first = *(QS.a) - VGPIRPBIN_PHREDSHIFT;

			assert ( first < (int64_t)qualTable->n );
			assert ( qualTable->A[first] != UINT64_MAX );

			first = qualTable->A[first];

			prev = first;

			if ( Table_increment(firstTable, first) < 0 )
			{
				fprintf(stderr,"[E] unable to increment value\n");
				goto cleanup;
			}

			for ( c = QS.a+1; c < QS.e; ++c )
			{
				int64_t lv = *c - VGPIRPBIN_PHREDSHIFT;
				int64_t dif;
				int64_t v;

				assert ( lv < (int64_t)qualTable->n );
				assert ( qualTable->A[lv] != UINT64_MAX );

				lv = qualTable->A[lv];

				dif = lv - prev;

				if ( dif >= 0 )
				{
					v = (dif+1)*2;
				}
				else
				{
					v = (-dif * 2) | 1;
				}

				if ( Table_increment(difTable, v) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}

				prev = lv;
			}

			#if 0
			fwrite(QS.a,1,QS.e-QS.a,stdout);
			fputc('\n',stdout);
			#endif

			nr += 1;

			if ( nr % (4*1024*1024) == 0 )
				fprintf(stderr,"[V] getQualityCode %lu %f\n", (unsigned long)nr, (double)FTELL(in)/dfs);

			ql += 1;
		}
	}

	fprintf(stderr,"[V] getQualityCode %lu %f\n", (unsigned long)nr, (double)1.0);

	/* if we (possibly) did not read all the Q lines, then add an escape code in the Huffman tables */
	if ( maxlines != INT64_MAX )
	{
		if ( Table_increment(firstTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}
		if ( Table_increment(difTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}
	}

	if ( ! (firstPT = PairTable_getPairTable(firstTable)) )
	{
		fprintf(stderr,"[E] unable to produce firstPT\n");
		goto cleanup;
	}
	if ( ! (difPT = PairTable_getPairTable(difTable)) )
	{
		fprintf(stderr,"[E] unable to produce difPT\n");
		goto cleanup;
	}

	if ( HuffmanCode_computeLengths(firstPT) != 0 )
	{
		fprintf(stderr,"[E] unable to produce code lengths from firstPT\n");
		goto cleanup;
	}

	if ( ! (firstHC = HuffmanCode_computeFromLengths(&firstPT)) )
	{
		fprintf(stderr,"[E] failed HuffmanCode_computeFromLengths\n");
		goto cleanup;
	}

	if ( HuffmanCode_computeLengths(difPT) != 0 )
	{
		fprintf(stderr,"[E] unable to produce code lengths from difPT\n");
		goto cleanup;
	}

	if ( ! (difHC = HuffmanCode_computeFromLengths(&difPT)) )
	{
		fprintf(stderr,"[E] failed HuffmanCode_computeFromLengths\n");
		goto cleanup;
	}

	QH = QualityHuffman_allocate(firstHC,difHC);
	if ( ! QH )
	{
		fprintf(stderr,"[E] unable to produce huffman struct\n");
		goto cleanup;
	}

	firstHC = NULL;
	difHC = NULL;

	Table_deallocate(firstTable);
	Table_deallocate(difTable);
	LineBuffer_deallocate(LB);

	return QH;

	cleanup:
	LineBuffer_deallocate(LB);
	Table_deallocate(firstTable);
	Table_deallocate(difTable);
	PairTable_deallocate(firstPT);
	PairTable_deallocate(difPT);
	HuffmanCode_deallocate(firstHC);
	HuffmanCode_deallocate(difHC);

	return NULL;
}

QualityHuffman * getQualityCodeFromFile(char const * fn, Table const * qualityTable, int64_t const maxlines)
{
	FILE * in = fopen(fn,"r");
	QualityHuffman * QH = NULL;

	if ( ! in )
		return NULL;

	QH = getQualityCode(in,qualityTable,maxlines);

	fclose(in);

	return QH;
}
