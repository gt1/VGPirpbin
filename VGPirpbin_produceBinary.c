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
#include "VGPirpbin_produceBinary.h"
#include "VGPirpbin_LineBuffer.h"
#include "VGPirpbin_HeaderStatsLine.h"
#include "VGPirpbin_expect.h"
#include "VGPirpbin_getNumber.h"
#include "VGPirpbin_mconcat.h"
#include "VGPirpbin_getQualityTable.h"
#include "VGPirpbin_getQualityCode.h"
#include <stddef.h>

int VGP_IRPBIN_produceBinary(FILE * out, FILE * in, QualityHuffman const * QH, Table * qualTable, char const * tmpfn, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode, char const * binfiletype)
{
	LineBuffer * LB = NULL;
	char const * a;
	char const * e;
	char const * ft = "seq";
	int returnvalue = 0;
	CString CS;
	BitLevelEncoder * BLE = NULL;
	size_t nr = 0;
	size_t numindex = 0;
	uint64_t fs;
	double dfs;
	size_t * reverseQualTable = NULL;
	size_t ii;
	size_t reverseQualTableSize = 0;
	uint64_t maxqualvalue = 0;
	uint64_t const indexmod = 1024;
	FILE * tmpfile = NULL;
	uint64_t indexposition = 0;
	uint64_t i;
	uint64_t numenc = 0;
	uint64_t numbits = 0;
	HuffmanCode const * firstHuf = NULL;
	HuffmanCode const * difHuf = NULL;
	CodeTable const * firstSparse = NULL;
	CodeTable const * difSparse = NULL;
	char * subtype = NULL;
	ProvenanceStep * PS = NULL;
	ProvenanceStep * PSL = NULL;
	HeaderStatsLine * HSL = NULL;
	uint64_t HSLo = 0;
	uint64_t HSLn = 0;
	char * filetype = NULL;
	int64_t fileversion = -1;
	int64_t filesubversion = -1;
	char const * cc;

	tmpfile = fopen(tmpfn,"wb");

	if ( ! tmpfile )
	{
		fprintf(stderr,"[E] unable to open tmp file\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( ii = 0; ii < qualTable->n; ++ii )
		if ( qualTable->A[ii] != UINT64_MAX )
		{
			/* ii -> qualTable->A[ii] */
			if ( qualTable->A[ii] > maxqualvalue )
				maxqualvalue = qualTable->A[ii];
		}

	reverseQualTableSize = maxqualvalue+1;

	reverseQualTable = (size_t *)malloc(sizeof(size_t)*reverseQualTableSize);

	for ( ii = 0; ii < qualTable->n; ++ii )
		if ( qualTable->A[ii] != UINT64_MAX )
			reverseQualTable[qualTable->A[ii]] = ii;

	#if 0
	for ( ii = 0; ii < reverseQualTableSize; ++ii )
		fprintf(stderr,"reverseQualTable[%d]=%d\n", (int)ii, (int)reverseQualTable[ii]);
	#endif

	FSEEK(in,0,SEEK_END);
	fs = FTELL(in);
	FSEEK(in,0,SEEK_SET);
	dfs = fs;

	/* allocate line buffer */
	if ( ! (LB = LineBuffer_allocate(in,1)) )
	{
		fprintf(stderr,"[E] unable to instantiate line buffer\n");
		returnvalue = -1;
		goto cleanup;
	}

	/* read line */
	if ( LineBuffer_getline(LB,&a,&e) <= 0 )
	{
		fprintf(stderr,"[E] unable to get first line of file\n");
		returnvalue = -1;
		goto cleanup;
	}
	if ( ! expect(&a,&e,'1') )
	{
		fprintf(stderr,"[E] first line of file does not start with a 1\n");
		returnvalue = -1;
		goto cleanup;
	}
	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		returnvalue = -1;
		goto cleanup;
	}

	CS = CString_getString(&a,&e);

	if ( ! CS.a )
	{
		fprintf(stderr,"[E] unable to read file type\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( (CS.e - CS.a) != (ptrdiff_t)(strlen(ft)) )
	{
		fprintf(stderr,"[E] file type has the wrong length\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( ! (filetype = CString_tostring(&CS)) )
	{
		fprintf(stderr,"[E] unable to copy file type\n");
		returnvalue = -1;
		goto cleanup;
	}

	/* check file type */
	if ( strcmp(filetype,ft) != 0 )
	{
		fprintf(stderr,"[E] unexpected file type\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after 1\n");
		returnvalue = -1;
		goto cleanup;
	}

	fileversion = getNumber(&a,&e);
	if ( fileversion < 0 )
	{
		fprintf(stderr,"[E] failed to get version number after file type\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( ! expect(&a,&e,' ') )
	{
		fprintf(stderr,"[E] first line of file does not have a space after version\n");
		returnvalue = -1;
		goto cleanup;
	}

	filesubversion = getNumber(&a,&e);
	if ( filesubversion < 0 )
	{
		fprintf(stderr,"[E] failed to get version number after file type\n");
		returnvalue = -1;
		goto cleanup;
	}

	firstHuf = QH->firstHuf;
	difHuf = QH->difHuf;
	firstSparse = firstHuf->CTsortedSparse;
	difSparse = difHuf->CTsortedSparse;

	while ( LineBuffer_getline(LB,&a,&e) )
	{
		if ( (e-a) && isalpha(a[0]) )
		{
			LineBuffer_putback(LB,a);
			break;
		}
		else
		{
			if (
				e-a &&
				(
					a[0] == '#'
					||
					a[0] == '+'
					||
					a[0] == '@'
				)
			)
			{
				char type, subtype;
				int64_t num;
				HeaderStatsLine LHSL;

				if ( e-a < 5 || a[1] != ' ' || a[3] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(a,e-a,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				type = a[0];
				subtype = a[2];

				a += 4;

				num = getNumber(&a,&e);

				LHSL.type = type;
				LHSL.subtype = subtype;
				LHSL.num = num;

				if ( HeaderStatsLine_push(&HSL, &HSLo, &HSLn, LHSL) < 0 )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(a,e-a,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				fprintf(stderr,"[V] header line type %c subtype %c num %ld\n",type,subtype,num);
			}
			else if (
				e-a
				&&
				a[0] == '2'
			)
			{
				char const * linestart = a;
				CString subtypeCS;

				if ( e-a < 2 || a[1] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				a += 2;

				subtypeCS = CString_getString(&a,&e);

				if ( ! subtypeCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				subtype = (char *)malloc(sizeof(char) * ((subtypeCS.e - subtypeCS.a) + 1));
				if ( ! subtype )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				subtype[subtypeCS.e - subtypeCS.a] = 0;
				memcpy(subtype,subtypeCS.a,subtypeCS.e - subtypeCS.a);

				fprintf(stderr,"[V] header line found subtype %s\n",subtype);
			}
			else if (
				e-a
				&&
				a[0] == '!'
			)
			{
				char const * linestart = a;
				CString progCS;
				CString progverCS;
				CString comlinCS;
				CString dateCS;
				ProvenanceStep * LPS = NULL;

				fprintf(stderr,"[V] header provenance line: ");
				fwrite(linestart,e-linestart,1,stderr);
				fprintf(stderr,"\n");

				if ( e-a < 2 || a[1] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				a += 2;

				progCS = CString_getString(&a,&e);

				if ( ! progCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				a += 1;

				progverCS = CString_getString(&a,&e);

				if ( ! progverCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				a += 1;

				comlinCS = CString_getString(&a,&e);

				if ( ! comlinCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				if ( e-a < 1 || a[0] != ' ' )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				a += 1;

				dateCS = CString_getString(&a,&e);

				if ( ! dateCS.a )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				LPS = ProvenanceStep_allocate(&progCS,&progverCS,&comlinCS,&dateCS);

				if ( ! LPS )
				{
					fprintf(stderr,"[E] cannot parse header line ");
					fwrite(linestart,e-linestart,1,stderr);
					fprintf(stderr,"\n");
					returnvalue = -1;
					goto cleanup;
				}

				ProvenanceStep_add(&PS,&PSL,LPS);
			}
			else
			{
				char const * linestart = a;

				fprintf(stderr,"[V] header line: ");
				fwrite(linestart,e-linestart,1,stderr);
				fprintf(stderr,"\n");
			}
		}
	}

	ProvenanceStep_add(&PS,&PSL,*insPS);
	*insPS = NULL;

	for ( i = 0; i < HSLo; ++i )
		if ( HSL[i].type == '#' && HSL[i].subtype == '!' )
			HSL[i].num += 1;

	for ( i = 0; i < HSLo; ++i )
	{
		fprintf(stderr,"%c %c %lu\n", HSL[i].type, HSL[i].subtype, HSL[i].num);
	}

	ProvenanceStep_print(stderr, PS);

	if ( ! (BLE = BitLevelEncoder_allocate(out)) )
	{
		fprintf(stderr,"[E] unable to instantiate BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( cc = binfiletype; *cc; ++cc )
		if ( BitLevelEncoder_encode(BLE,*cc,8) < 0 )
		{
			fprintf(stderr,"[E] unable to add version line in BitLevelEncoder\n");
			returnvalue = -1;
			goto cleanup;
		}

	if ( BitLevelEncoder_encode(BLE,0,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add padding in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( BitLevelEncoder_encode(BLE,HSLo,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add number of header objects\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( i = 0; i < HSLo; ++i )
		if ( HeaderStatsLine_encode(BLE,&(HSL[i])) < 0 )
		{
			fprintf(stderr,"[E] unable to add header objects\n");
			returnvalue = -1;
			goto cleanup;
		}

	if ( ProvenanceStep_encode(BLE, PS) < 0 )
	{
		fprintf(stderr,"[E] unable to encode provenance\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( BitLevelEncoder_encode(BLE,indexmod,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add padding in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( QualityHuffman_encode(BLE,QH) < 0 )
	{
		fprintf(stderr,"[E] unable to write Huffman code tables to output file\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( BitLevelEncoder_encodeGamma(BLE,reverseQualTableSize) < 0 )
	{
		fprintf(stderr,"[E] unable to write size of reverse quality table to output file\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( ii = 0; ii < reverseQualTableSize; ++ii )
		if ( BitLevelEncoder_encodeGamma(BLE,reverseQualTable[ii]) < 0 )
		{
			fprintf(stderr,"[E] unable to write reverse quality table to output file\n");
			returnvalue = -1;
			goto cleanup;
		}

	if ( HuffmanCode_encode(BLE, symCode) < 0 )
	{
		fprintf(stderr,"[E] unable to write sym huffman tables to output file\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( HuffmanCode_encode(BLE, lengthsCode) < 0 )
	{
		fprintf(stderr,"[E] unable to write lengths huffman tables to output file\n");
		returnvalue = -1;
		goto cleanup;
	}

	while ( LineBuffer_getline(LB,&a,&e) )
	{
		if ( e-a )
		{
			char const linetype = a[0];

			if ( linetype == 'P' )
			{
				#if 0
				if ( BitLevelEncoder_flush(BLE) < 0 )
				{
					fprintf(stderr,"[E] BitLevelEncoder_flush failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				#endif

				uint64_t const offset = BitLevelEncoder_getOffset(BLE);

				if ( nr % indexmod == 0 )
				{
					if ( fwrite(
						&offset,
						sizeof(offset),
						1,
						tmpfile
						) != 1
					)
					{
						fprintf(stderr,"[E] unable to write to tmp file\n");
						returnvalue = -1;
						goto cleanup;
					}

					numindex += 1;
				}

				nr += 1;

				if ( nr % (4*1024*1024) == 0 )
					fprintf(stderr,"[V] VGP_IRPBIN_produceBinary number of pairs %lu input processed %f bits per symbol %f\n",
						(unsigned long)nr, (double)FTELL(in)/dfs,
						BitLevelEncoder_getOffset(BLE) / (double)numenc
					);

				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'P' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encode(P) failed\n");
					returnvalue = -1;
					goto cleanup;
				}
			}
			else if ( linetype == 'Q' )
			{
				CString QS;
				char const * c;
				int64_t first;
				int64_t prev;
				CodeTableEntry * firstcode;
				int needfirstescape;
				int64_t l;

				a += 1;

				if ( ! expect(&a,&e,' ') )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					returnvalue = -1;
					goto cleanup;
				}

				QS = CString_getString(&a,&e);

				if ( ! QS.a )
				{
					fprintf(stderr,"[E] malformed Q line\n");
					returnvalue = -1;
					goto cleanup;
				}

				for ( c = QS.a; c < QS.e; ++c )
					if ( *c < VGPIRPBIN_PHREDSHIFT || *c > 93 )
					{
						fprintf(stderr,"[E] malformed Q line\n");
						returnvalue = -1;
						goto cleanup;
					}

				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'Q' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q) failed\n");
					returnvalue = -1;
					goto cleanup;
				}

				l = QS.e - QS.a;

				if ( l < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, l) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}
				else
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
					if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(Q_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}

				first = *(QS.a) - VGPIRPBIN_PHREDSHIFT;

				assert ( first < (int64_t)qualTable->n );
				assert ( qualTable->A[first] != UINT64_MAX );

				/* fprintf(stderr,"mapping %d to %d\n", first, qualTable->A[first]); */

				first = qualTable->A[first];

				needfirstescape = (firstSparse->A[first].codelength == UINT64_MAX);

				firstcode = needfirstescape ?
					&(firstSparse->A[VGPIRPBIN_HUFFMAN_ESCAPE_CODE])
					:
					&(firstSparse->A[first]);
				assert ( firstcode->codelength != UINT64_MAX );

				if ( BitLevelEncoder_encode(BLE,firstcode->code,firstcode->codelength) < 0 )
				{
					fprintf(stderr,"[E] BitLevelEncoder_encode(first) failed\n");
					returnvalue = -1;
					goto cleanup;
				}
				if ( needfirstescape )
				{
					if ( BitLevelEncoder_encodeGamma(BLE,first) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(first) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}
				numenc += 1;
				numbits += firstcode->codelength;

				prev = first;

				/* handle first */

				for ( c = QS.a+1; c < QS.e; ++c )
				{
					int64_t lv = *c - VGPIRPBIN_PHREDSHIFT;
					CodeTableEntry * difcode;
					int64_t dif;
					int64_t v;
					int needescape;

					assert ( lv < (int64_t)qualTable->n );
					assert ( qualTable->A[lv] != UINT64_MAX );

					/* fprintf(stderr,"mapping %d to %d\n", lv, qualTable->A[lv]); */

					lv = qualTable->A[lv];

					dif = lv - prev;

					/* lv = dif + prev */


					if ( dif >= 0 )
					{
						v = (dif+1)*2;
					}
					else
					{
						v = (-dif * 2) | 1;
					}

					/* fprintf(stderr,"odifsym=%d odif=%d\n",(int)v,(int)dif); */

					needescape = (difSparse->A[v].codelength == UINT64_MAX);

					difcode = needescape ? &(difSparse->A[VGPIRPBIN_HUFFMAN_ESCAPE_CODE]) : &(difSparse->A[v]);
					assert ( difcode->codelength != UINT64_MAX );

					if ( BitLevelEncoder_encode(BLE,difcode->code,difcode->codelength) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encode(dif) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
					if ( needescape )
					{
						if ( BitLevelEncoder_encodeGamma(BLE,v) < 0 )
						{
							fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(dif) failed\n");
							returnvalue = -1;
							goto cleanup;
						}
					}

					numenc += 1;
					numbits += difcode->codelength;

					/* CodeTableEntry_print(stderr,difcode); */

					/* handle v */

					prev = lv;
				}

				#if 0
				nr += 1;

				if ( nr % (4*1024*1024) == 0 )
					fprintf(stderr,"[V] VGP_IRPBIN_produceBinary %lu %f\n", (unsigned long)nr, (double)FTELL(in)/dfs);
				#endif

			}
			else if ( linetype == 'S' )
			{
				CString QS;
				char const * c;
				int64_t l;

				a += 1;

				if ( ! expect(&a,&e,' ') )
				{
					fprintf(stderr,"[E] malformed S line\n");
					returnvalue = -1;
					goto cleanup;
				}

				QS = CString_getString(&a,&e);

				if ( ! QS.a )
				{
					fprintf(stderr,"[E] malformed S line\n");
					returnvalue = -1;
					goto cleanup;
				}

				if ( HuffmanCode_encodeSymbol(BLE, symCode, 'S' ) < 0 )
				{
					fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S) failed\n");
					returnvalue = -1;
					goto cleanup;
				}

				l = QS.e - QS.a;

				if ( l < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, l) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}
				else
				{
					if ( HuffmanCode_encodeSymbol(BLE, lengthsCode, VGPIRPBIN_HUFFMAN_ESCAPE_CODE) < 0 )
					{
						fprintf(stderr,"[E] HuffmanCode_encodeSymbol(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
					if ( BitLevelEncoder_encodeGamma(BLE,l) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encodeGamma(S_length) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}


				for ( c = QS.a; c < QS.e; ++c )
				{
					unsigned int sym = 0;

					switch ( *c )
					{
						case 'a':
						case 'A':
							sym = 0;
							break;
						case 'c':
						case 'C':
							sym = 1;
							break;
						case 'g':
						case 'G':
							sym = 2;
							break;
						case 't':
						case 'T':
							sym = 3;
							break;
					}

					assert ( sym < 4 );

					if ( BitLevelEncoder_encode(BLE,sym,2) < 0 )
					{
						fprintf(stderr,"[E] BitLevelEncoder_encode(S data) failed\n");
						returnvalue = -1;
						goto cleanup;
					}
				}

				numbits += 2*(QS.e-QS.a);
			}
		}
	}

	if ( BitLevelEncoder_flush(BLE) < 0 )
	{
		fprintf(stderr,"[E] BitLevelEncoder_flush() failed\n");
		returnvalue = -1;
		goto cleanup;
	}

	if ( fclose(tmpfile) != 0 )
	{
		fprintf(stderr,"[E] unable correctly close tmp file\n");
		returnvalue = -1;
		goto cleanup;
	}

	indexposition = BitLevelEncoder_getOffset(BLE);

	tmpfile = fopen(tmpfn,"rb");

	if ( ! tmpfile )
	{
		fprintf(stderr,"[E] unable re open tmp file\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( i = 0; i < numindex; ++i )
	{
		uint64_t v;
		if ( fread(&v,sizeof(uint64_t),1,tmpfile) != 1 )
		{
			fprintf(stderr,"[E] unable read tmp file\n");
			returnvalue = -1;
			goto cleanup;
		}

		if ( BitLevelEncoder_encode(BLE,v,64) < 0 )
		{
			fprintf(stderr,"[E] unable write index dara\n");
			returnvalue = -1;
			goto cleanup;
		}
	}

	if ( BitLevelEncoder_flush(BLE) < 0 )
	{
		fprintf(stderr,"[E] BitLevelEncoder_flush() failed\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( i = 0; i < sizeof(uint64_t); ++i )
	{
		unsigned char const u = indexposition >> ((sizeof(uint64_t) - i - 1)*8) & 0xFF;
		if ( fwrite(&u,1,1,out) != 1 )
		{
			fprintf(stderr,"[E] fwrite index failed\n");
			returnvalue = -1;
			goto cleanup;
		}
	}

	FSEEK(out,0,SEEK_SET);

	for ( cc = binfiletype; *cc; ++cc )
		if ( BitLevelEncoder_encode(BLE,*cc,8) < 0 )
		{
			fprintf(stderr,"[E] unable to add version line in BitLevelEncoder\n");
			returnvalue = -1;
			goto cleanup;
		}

	if ( BitLevelEncoder_encode(BLE,nr,64) < 0 )
	{
		fprintf(stderr,"[E] unable to add number of reads in BitLevelEncoder\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] VGP_IRPBIN_produceBinary number of sequences %lu input processed %f bits per symbol %f\n",
		(unsigned long)nr, (double)FTELL(in)/dfs,
		BitLevelEncoder_getOffset(BLE) / (double)numenc
	);
	/* fprintf(stderr,"numenc %lu bits %lu bits per symbol %f\n", (unsigned long)numenc, (unsigned long)numbits, (double)numbits/numenc); */

	remove(tmpfn);

	cleanup:
	ProvenanceStep_deallocate(PS);
	if ( tmpfile )
	{
		fclose(tmpfile);
		tmpfile = NULL;
	}
	LineBuffer_deallocate(LB);
	BitLevelEncoder_deallocate(BLE);
	free(reverseQualTable);
	free(subtype);
	free(HSL);
	free(filetype);

	return returnvalue;
}

int VGP_IRPBIN_produceBinaryFromFile(char const * outfn, char const * fn, QualityHuffman const * QH, Table * qualTable, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode, char const * binfiletype)
{
	char * tmpfn = mconcat(outfn,".tmp");
	FILE * in = NULL;
	FILE * out = NULL;
	int r;

	if ( ! tmpfn )
		return -1;

	in = fopen(fn,"r");

	if ( ! in )
	{
		free(tmpfn);
		return -1;
	}

	out = fopen(outfn,"wb");

	if ( ! out )
	{
		free(tmpfn);
		fclose(in);
		return -1;
	}

	r = VGP_IRPBIN_produceBinary(out,in,QH,qualTable,tmpfn,insPS,symCode,lengthsCode,binfiletype);

	free(tmpfn);
	fclose(in);
	fclose(out);

	return r;
}

int VGP_IRPBIN_produceBinaryFile(char const * outfn, char const * infn, int64_t const maxstatlines, ProvenanceStep ** insPS, char const * binfiletype)
{
	Table * QT = NULL;
	Table * symTable = NULL;
	Table * lengthsTable = NULL;
	PairTable * symPT = NULL;
	PairTable * lengthsPT = NULL;
	QualityHuffman * QH = NULL;
	HuffmanCode * symCode = NULL;
	HuffmanCode * lengthsCode = NULL;
	int r;
	size_t i;
	int returnvalue = 0;

	if ( getQualityTableFromFile(infn,maxstatlines,&QT,&symTable,&lengthsTable) < 0 )
	{
		fprintf(stderr,"[E] failed to compute quality table\n");
		returnvalue = -1;
		goto cleanup;
	}

	for ( i = 0; i < QT->n; ++i )
		if ( QT->A[i] != UINT64_MAX )
			fprintf(stderr,"QT[%d]=%d\n", (int)i, (int)QT->A[i]);

	symPT = PairTable_getPairTable(symTable);

	if ( ! symPT )
	{
		fprintf(stderr,"[E] failed to compute sym pair table\n");
		returnvalue = -1;
		goto cleanup;
	}

	Table_deallocate(symTable);
	symTable = NULL;

	if ( HuffmanCode_computeLengths(symPT) < 0 )
	{
		fprintf(stderr,"[E] failed to compute code for symbol information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] symbol table\n");
	for ( i = 0; i < symPT->n; ++i )
	{
		uint64_t const key = symPT->A[i].key;
		uint64_t const value = symPT->A[i].value;

		if ( key < VGPIRPBIN_HUFFMAN_ESCAPE_CODE && isprint(key) )
			fprintf(stderr,"%c %d\n",(char)key,(int)value);
		else
			fprintf(stderr,"%d %d\n",(int)key,(int)value);
	}


	/* lenghts */
	lengthsPT = PairTable_getPairTable(lengthsTable);

	if ( ! lengthsPT )
	{
		fprintf(stderr,"[E] failed to compute lenghts pair table\n");
		returnvalue = -1;
		goto cleanup;
	}

	Table_deallocate(lengthsTable);
	lengthsTable = NULL;

	if ( HuffmanCode_computeLengths(lengthsPT) < 0 )
	{
		fprintf(stderr,"[E] failed to compute code for lenghtsbol information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] lengths table\n");
	for ( i = 0; i < lengthsPT->n; ++i )
	{
		uint64_t const key = lengthsPT->A[i].key;
		uint64_t const value = lengthsPT->A[i].value;

		if ( key < VGPIRPBIN_HUFFMAN_ESCAPE_CODE && isprint(key) )
			fprintf(stderr,"%c %d\n",(char)key,(int)value);
		else
			fprintf(stderr,"%d %d\n",(int)key,(int)value);
	}

	if ( !(symCode = HuffmanCode_computeFromLengths(&symPT)) )
	{
		fprintf(stderr,"[E] unable to construct huffman code for symbols");
		returnvalue = -1;
		goto cleanup;
	}

	if ( !(lengthsCode = HuffmanCode_computeFromLengths(&lengthsPT)) )
	{
		fprintf(stderr,"[E] unable to construct huffman code for lenghts");
		returnvalue = -1;
		goto cleanup;
	}
	/* HuffmanCode * HuffmanCode_computeFromLengths(PairTable ** firstPT); */

	QH = getQualityCodeFromFile(infn,QT,maxstatlines);

	if ( ! QH )
	{
		fprintf(stderr,"[E] failed to compute code for quality information\n");
		returnvalue = -1;
		goto cleanup;
	}

	fprintf(stderr,"[V] first huffman table\n");
	CodeTable_print(stderr,QH->firstHuf->CT);
	fprintf(stderr,"[V] dif huffman table\n");
	CodeTable_print(stderr,QH->difHuf->CT);

	r = VGP_IRPBIN_produceBinaryFromFile(outfn,infn, QH, QT, insPS, symCode, lengthsCode, binfiletype);

	if ( r < 0 )
	{
		Table_deallocate(QT);
		fprintf(stderr,"[E] VGP_IRPBIN_produceBinaryFromFile failed\n");
		return -1;
	}

	cleanup:
	HuffmanCode_deallocate(symCode);
	HuffmanCode_deallocate(lengthsCode);
	Table_deallocate(QT);
	Table_deallocate(symTable);
	Table_deallocate(lengthsTable);
	PairTable_deallocate(symPT);
	PairTable_deallocate(lengthsPT);
	QualityHuffman_deallocate(QH);
	return returnvalue;
}
