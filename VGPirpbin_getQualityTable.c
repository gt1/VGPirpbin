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
#include "VGPirpbin_getQualityTable.h"
#include "VGPirpbin_LineBuffer.h"
#include "VGPirpbin_CString.h"
#include "VGPirpbin_expect.h"
#include "VGPirpbin_getNumber.h"
#include <stddef.h>

int getQualityTable(FILE * in, int64_t const maxlines, Table ** rqualTable, Table ** rsymTable, Table ** rlengthsTable)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	CString CS;
	Table * qualTable = NULL;
	Table * symTable = NULL;
	Table * lengthsTable = NULL;
	size_t i;
	size_t rank = 0;
	size_t nr = 0;
	uint64_t fs = 0;
	int64_t ql = 0;
	double dfs;

	/* seek to end of file */
	FSEEK(in,0,SEEK_END);
	fs = FTELL(in);
	FSEEK(in,0,SEEK_SET);
	dfs = fs;

	LB = LineBuffer_allocate(in,1);

	if ( ! LB )
	{
		fprintf(stderr,"[E] unable to allocate LineBuffer\n");
		goto cleanup;
	}

	if ( ! (qualTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate qual table\n");
		goto cleanup;
	}

	if ( ! (symTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate sym table\n");
		goto cleanup;
	}

	if ( ! (lengthsTable = Table_allocate(1)) )
	{
		fprintf(stderr,"[E] unable to allocate lengths table\n");
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
		if ( e-a && isalpha(a[0]) )
		{
			if ( Table_increment(symTable, a[0]) < 0 )
			{
				fprintf(stderr,"[E] unable to increment value\n");
				goto cleanup;
			}
		}

		if ( e-a && a[0] == 'S' )
		{
			char const * c = a;
			int64_t l;

			c += 1;

			if ( c != e && *c == ' ' )
				++c;
			else
			{
				fprintf(stderr,"[E] malformed S line\n");
				goto cleanup;
			}

			if ( (l = getNumber(&c,&e)) < 0 )
			{
				fprintf(stderr,"[E] malformed S line\n");
				goto cleanup;
			}
			else
			{
				if ( l < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
				{
					if ( Table_increment(lengthsTable, l) < 0 )
					{
						fprintf(stderr,"[E] unable to increment value\n");
						goto cleanup;
					}
				}
				else
				{
					if ( Table_increment(lengthsTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] unable to increment value\n");
						goto cleanup;
					}
				}
			}
		}

		if ( e-a && a[0] == 'Q' )
		{
			CString QS;
			char const * c;
			int64_t l;

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

			l = QS.e - QS.a;

			if ( l < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
			{
				if ( Table_increment(lengthsTable, l) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}
			else
			{
				if ( Table_increment(lengthsTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}

			for ( c = QS.a; c < QS.e; ++c )
				if ( *c < VGPIRPBIN_PHREDSHIFT || *c > 93 )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					goto cleanup;
				}

			for ( c = QS.a; c < QS.e; ++c )
			{
				int64_t const lv = *c - VGPIRPBIN_PHREDSHIFT;

				if ( Table_increment(qualTable, lv) < 0 )
				{
					fprintf(stderr,"[E] unable to increment value\n");
					goto cleanup;
				}
			}

			nr += 1;

			if ( nr % (4*1024*1024) == 0 )
				fprintf(stderr,"[V] getQualityTable %lu %f\n", (unsigned long)nr, (double)FTELL(in)/dfs);

			ql += 1;
		}
	}

	if ( maxlines != INT64_MAX )
	{
		if ( Table_increment(symTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}
		if ( Table_increment(lengthsTable, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
		{
			fprintf(stderr,"[E] unable to increment value\n");
			goto cleanup;
		}
	}

	LineBuffer_deallocate(LB);

	for ( i = 0; i < qualTable->n; ++i )
		if ( qualTable->A[i] )
			qualTable->A[i] = rank++;
		else
			qualTable->A[i] = UINT64_MAX;

	fprintf(stderr,"[V] getQualityTable %lu %f\n", (unsigned long)nr, 1.0);

	*rqualTable = qualTable;
	*rsymTable = symTable;
	*rlengthsTable = lengthsTable;

	return 0;

	cleanup:
	LineBuffer_deallocate(LB);
	Table_deallocate(qualTable);
	Table_deallocate(symTable);

	return -1;
}

int getQualityTableFromFile(char const * fn, int64_t const maxlines, Table ** rtable, Table ** rsymtable, Table ** rlengthsTable)
{
	FILE * in = fopen(fn,"r");

	if ( ! in )
		return -1;

	if ( getQualityTable(in,maxlines,rtable,rsymtable,rlengthsTable) < 0 )
	{
		fclose(in);
		return -1;
	}

	fclose(in);

	return 0;
}
