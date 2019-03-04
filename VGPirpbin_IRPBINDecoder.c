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
#include "VGPirpbin_IRPBINDecoder.h"
#include "VGPirpbin_IRPBINDecoderInput.h"
#include "VGPirpbin_mstrdup.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

static int IRPBINDecoder_decodeSequenceAndQuality(
	BitLevelDecoder * BLD,
	QualityHuffman * QH,
	HuffmanCode * symCode,
	HuffmanCode * lengthsCode,
	uint64_t * reverseQualityTable,
	DecodeResult * D
)
{
	/* uint64_t v; */
	uint64_t seqlen;
	uint64_t qlen;
	uint64_t i;
	int returncode = 0;
	int64_t firstqual;
	int64_t prevqual;
	int64_t firstqualsym;
	int64_t llv;

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'S' )
	{
		fprintf(stderr,"[E] failed to find S marker after P marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after S marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( llv < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		seqlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&seqlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read sequence length after P marker\n");
			returncode = -1;
			goto cleanup;
		}
	}

	while ( seqlen > D->S_o )
	{
		size_t nlen = D->S_o ? 2 * D->S_o : 1;
		free(D->S);

		D->S = NULL;
		D->S_o = 0;
		D->S = (char *)malloc(nlen);

		if ( ! D->S )
		{
			fprintf(stderr,"[E] failed to allocate read buffer\n");
			returncode = -1;
			goto cleanup;
		}

		D->S_o = nlen;
	}

	/* fprintf(stderr,"seqlen %lu\n", (unsigned long)seqlen); */

	for ( i = 0; i < seqlen; ++i )
	{
		uint64_t sym;
		if ( BitLevelDecoder_decode(BLD,&sym,2) < 0 )
		{
			fprintf(stderr,"[E] failed to decode symbol\n");
			returncode = -1;
			goto cleanup;
		}

		switch ( sym )
		{
			case 0: sym = 'A'; break;
			case 1: sym = 'C'; break;
			case 2: sym = 'G'; break;
			case 3: sym = 'T'; break;
		}

		/* fprintf(stderr,"%c", (char)sym); */
		D->S[i] = sym;
	}

	D->S_l = seqlen;

	/* fprintf(stderr,"\n"); */

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'Q' )
	{
		fprintf(stderr,"[E] failed to find Q marker after sequence\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after Q marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( llv < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		qlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&qlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read q length after P marker\n");
			returncode = -1;
			goto cleanup;
		}
	}


	while ( qlen > D->Q_o )
	{
		size_t nlen = D->Q_o ? 2 * D->Q_o : 1;
		free(D->Q);

		D->Q = NULL;
		D->Q_o = 0;
		D->Q = (char *)malloc(nlen);

		if ( ! D->Q )
		{
			fprintf(stderr,"[E] failed to allocate read buffer\n");
			returncode = -1;
			goto cleanup;
		}

		D->Q_o = nlen;
	}

	/* fprintf(stderr,"qlen %lu\n", (unsigned long)qlen); */

	firstqualsym = HuffmanCode_decodeSymbol(QH->firstHuf, BLD);

	if ( firstqualsym < 0 )
	{
		fprintf(stderr,"[E] failed to read first quality value\n");
		returncode = -1;
		goto cleanup;
	}

	if ( firstqualsym == VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		uint64_t v;

		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			fprintf(stderr,"[E] failed to read first quality value\n");
			returncode = -1;
			goto cleanup;
		}

		firstqualsym = v;
	}

	firstqual = reverseQualityTable[firstqualsym];
	prevqual = firstqualsym;

	/* fprintf(stderr,"firstqualsym %d firstqual %d\n", (int)firstqualsym, (int)firstqual); */
	/* fprintf(stderr,"%c",(char)(firstqual + VGPIRPBIN_PHREDSHIFT)); */
	D->Q[0] = firstqual + VGPIRPBIN_PHREDSHIFT;

	for ( i = 1; i < qlen; ++i )
	{
		int64_t difsym;
		int64_t dif;
		int64_t nextqual;
		int64_t nextqualvalue;

		difsym = HuffmanCode_decodeSymbol(QH->difHuf, BLD);

		if ( difsym < 0 )
		{
			fprintf(stderr,"[E] failed to read quality dif value\n");
			returncode = -1;
			goto cleanup;
		}

		if ( difsym == VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
		{
			uint64_t v;

			if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
			{
				fprintf(stderr,"[E] failed to read quality dif value\n");
				returncode = -1;
				goto cleanup;
			}

			difsym = v;
		}

		if ( (difsym & 1) )
			dif = -(difsym ^ 1)/2;
		else
			dif =  (difsym/2) - 1;

		nextqual      = prevqual + dif;
		nextqualvalue = reverseQualityTable[nextqual];

		/* fprintf(stderr,"nextqualsym %d nextqual %d sym %c\n", (int)nextqual, (int)nextqualvalue, (char)(nextqualvalue+VGPIRPBIN_PHREDSHIFT)); */
		/* fprintf(stderr,"%c",(int)(nextqualvalue+VGPIRPBIN_PHREDSHIFT)); */
		D->Q[i] = nextqualvalue + VGPIRPBIN_PHREDSHIFT;

		prevqual = nextqual;
	}

	/* fprintf(stderr,"\n"); */

	D->Q_l = qlen;

	cleanup:

	return returncode;
}

#if 0
static int IRPBINDecoder_skipSequenceAndQuality(
	BitLevelDecoder * BLD,
	QualityHuffman * QH,
	HuffmanCode * symCode,
	HuffmanCode * lengthsCode
)
{
	/* uint64_t v; */
	uint64_t seqlen;
	uint64_t qlen;
	uint64_t i;
	int returncode = 0;
	/* int64_t firstqual; */
	int64_t prevqual;
	int64_t firstqualsym;
	int64_t llv;

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'S' )
	{
		fprintf(stderr,"[E] failed to find S marker after P marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after S marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( llv < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		seqlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&seqlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read sequence length after P marker\n");
			returncode = -1;
			goto cleanup;
		}
	}

	/* fprintf(stderr,"seqlen %lu\n", (unsigned long)seqlen); */

	for ( i = 0; i < seqlen; ++i )
	{
		uint64_t sym;
		if ( BitLevelDecoder_decode(BLD,&sym,2) < 0 )
		{
			fprintf(stderr,"[E] failed to decode symbol\n");
			returncode = -1;
			goto cleanup;
		}

		switch ( sym )
		{
			case 0: sym = 'A'; break;
			case 1: sym = 'C'; break;
			case 2: sym = 'G'; break;
			case 3: sym = 'T'; break;
		}

		/* fprintf(stderr,"%c", (char)sym); */
	}

	/* fprintf(stderr,"\n"); */

	if ( (llv = HuffmanCode_decodeSymbol(symCode, BLD)) < 0 || llv != 'Q' )
	{
		fprintf(stderr,"[E] failed to find Q marker after sequence\n");
		returncode = -1;
		goto cleanup;
	}

	if ( (llv = HuffmanCode_decodeSymbol(lengthsCode, BLD)) < 0 )
	{
		fprintf(stderr,"[E] failed to decode length of sequence after Q marker\n");
		returncode = -1;
		goto cleanup;
	}

	if ( llv < VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		qlen = llv;
	}
	else
	{
		if ( BitLevelDecoder_decodeGamma(BLD,&qlen) < 0 )
		{
			fprintf(stderr,"[E] failed to read q length after P marker\n");
			returncode = -1;
			goto cleanup;
		}
	}

	/* fprintf(stderr,"qlen %lu\n", (unsigned long)qlen); */

	firstqualsym = HuffmanCode_decodeSymbol(QH->firstHuf, BLD);

	if ( firstqualsym < 0 )
	{
		fprintf(stderr,"[E] failed to read first quality value\n");
		returncode = -1;
		goto cleanup;
	}

	if ( firstqualsym == VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
	{
		uint64_t v;

		if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		{
			fprintf(stderr,"[E] failed to read first quality value\n");
			returncode = -1;
			goto cleanup;
		}

		firstqualsym = v;
	}

	/* firstqual = reverseQualityTable[firstqualsym]; */
	prevqual = firstqualsym;

	/* fprintf(stderr,"firstqualsym %d firstqual %d\n", (int)firstqualsym, (int)firstqual); */
	/* fprintf(stderr,"%c",(char)(firstqual + VGPIRPBIN_PHREDSHIFT)); */
	/* D->Q[0] = firstqual + VGPIRPBIN_PHREDSHIFT; */

	for ( i = 1; i < qlen; ++i )
	{
		int64_t difsym;
		int64_t dif;
		int64_t nextqual;
		/* int64_t nextqualvalue; */

		difsym = HuffmanCode_decodeSymbol(QH->difHuf, BLD);

		if ( difsym < 0 )
		{
			fprintf(stderr,"[E] failed to read quality dif value\n");
			returncode = -1;
			goto cleanup;
		}

		if ( difsym == VGPIRPBIN_HUFFMAN_ESCAPE_CODE )
		{
			uint64_t v;

			if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
			{
				fprintf(stderr,"[E] failed to read quality dif value\n");
				returncode = -1;
				goto cleanup;
			}

			difsym = v;
		}

		if ( (difsym & 1) )
			dif = -(difsym ^ 1)/2;
		else
			dif =  (difsym/2) - 1;

		nextqual      = prevqual + dif;
		/* nextqualvalue = reverseQualityTable[nextqual]; */

		/* fprintf(stderr,"nextqualsym %d nextqual %d sym %c\n", (int)nextqual, (int)nextqualvalue, (char)(nextqualvalue+VGPIRPBIN_PHREDSHIFT)); */
		/* fprintf(stderr,"%c",(int)(nextqualvalue+VGPIRPBIN_PHREDSHIFT)); */
		/* D->Q[i] = nextqualvalue + VGPIRPBIN_PHREDSHIFT; */

		prevqual = nextqual;
	}

	/* fprintf(stderr,"\n"); */

	/* D->Q_l = qlen; */

	cleanup:

	return returncode;
}
#endif

IRPBINDecoder * IRPBINDecoder_deallocate(IRPBINDecoder * I)
{
	if ( I )
	{
		#if 0
		if ( I->in )
			fclose(I->in);
		BitLevelDecoder_deallocate(I->BLD);
		#endif

		QualityHuffman_deallocate(I->QH);
		#if 0
		DecodeResult_deallocate(I->DF);
		DecodeResult_deallocate(I->DR);
		#endif
		free(I->reverseQualityTable);
		free(I->HSL);
		ProvenanceStep_deallocate(I->PS);
		HuffmanCode_deallocate(I->symCode);
		HuffmanCode_deallocate(I->lengthsCode);
		free(I->fn);
		free(I);
	}

	return NULL;
}


IRPBINDecoder * IRPBINDecoder_allocate()
{
	IRPBINDecoder * I = NULL;

	I = (IRPBINDecoder *)malloc(sizeof(IRPBINDecoder));

	if ( ! I )
		return IRPBINDecoder_deallocate(I);

	memset(I,0,sizeof(IRPBINDecoder));

	#if 0
	if ( !(I->DF = DecodeResult_allocate()) )
		return IRPBINDecoder_deallocate(I);

	if ( !(I->DR = DecodeResult_allocate()) )
		return IRPBINDecoder_deallocate(I);
	#endif

	return I;
}


IRPBINDecoder * IRPBINDecoder_allocateFromFile(char const * fn, char const * binfiletype)
{
	IRPBINDecoder * I = NULL;
	uint64_t iii = 0;
	uint64_t ii = 0;
	char const * cc = NULL;
	IRPBINDecoder_Input * IN = NULL;

	if ( !(I = IRPBINDecoder_allocate()) )
	{
		fprintf(stderr,"[E] failed to allocate memory\n");
		goto cleanup;
	}

	if ( ! (I->fn = mstrdup(fn)) )
	{
		fprintf(stderr,"[E] failed to allocate memory\n");
		goto cleanup;
	}

	if ( ! (IN = IRPBINDecoder_Input_allocate(I->fn)) )
	{
		fprintf(stderr,"[E] failed to open file %s\n",fn);
		goto cleanup;
	}

	#if 0
	if ( ! (I->in = fopen(fn,"rb")) )
	{
		fprintf(stderr,"[E] failed to open file %s\n",fn);
		goto cleanup;
	}
	#endif

	/* read position of index */
	FSEEK(IN->in,-(int)sizeof(uint64_t),SEEK_END);

	I->indexpos = 0;
	for ( iii = 0; iii < sizeof(uint64_t); ++iii )
	{
		unsigned char c;
		if ( fread(&c,1,1,IN->in) != 1 )
		{
			fprintf(stderr,"[E] failed to read position of index %s\n",fn);
			goto cleanup;
		}

		I->indexpos <<= 8;
		I->indexpos |= c;
	}

	#if 0
	fprintf(stderr,"index position at %lu\n", (unsigned long)indexpos);
	#endif

	FSEEK(IN->in,0,SEEK_SET);

	#if 0
	if ( ! (I->BLD = BitLevelDecoder_allocate(I->in)) )
	{
		fprintf(stderr,"[E] failed to instantiate bit level decoder for %s\n",fn);
		goto cleanup;
	}
	#endif

	/* read file type */
	for ( cc = binfiletype; *cc; ++cc )
	{
		uint64_t v;
		int c;

		if ( BitLevelDecoder_decode(IN->BLD,&v,8) < 0 )
		{
			fprintf(stderr,"[E] failed to read number of reads in file\n");
			goto cleanup;
		}

		c = v;

		if ( c != *cc )
		{
			fprintf(stderr,"[E] file type mismatch\n");
			goto cleanup;
		}
	}

	/* read number of pairs in file */
	if ( BitLevelDecoder_decode(IN->BLD,&(I->nr),64) < 0 )
	{
		fprintf(stderr,"[E] failed to read number of reads in file\n");
		goto cleanup;
	}

	/* read size of HeaderStatsLine table */
	if ( BitLevelDecoder_decode(IN->BLD,&(I->HSLo),64) < 0 )
	{
		fprintf(stderr,"[E] failed to read number of header objects in file\n");
		goto cleanup;
	}

	/* allocate HeaderStatsLine table */
	if ( ! (I->HSL = (HeaderStatsLine *)malloc(I->HSLo * sizeof(HeaderStatsLine))) )
	{
		fprintf(stderr,"[E] unable to read header objects\n");
		goto cleanup;
	}

	/* read HeaderStatsLine table */
	for ( ii = 0; ii < I->HSLo; ++ii )
		if ( HeaderStatsLine_decode(IN->BLD,&(I->HSL[ii])) < 0 )
		{
			fprintf(stderr,"[E] unable to read header objects\n");
			goto cleanup;
		}

	/* decode provenance lines */
	if ( !(I->PS = ProvenanceStep_decode(IN->BLD)) )
	{
		fprintf(stderr,"[E] unable to decode provenance\n");
		goto cleanup;
	}


	/* read index mod */
	if ( BitLevelDecoder_decode(IN->BLD,&(I->indexmod),64) < 0 )
	{
		fprintf(stderr,"[E] failed to indexmod from file\n");
		goto cleanup;
	}

	/* read huffman tables  */
	if ( ! (I->QH = QualityHuffman_decode(IN->BLD)) )
	{
		fprintf(stderr,"[E] failed to decode Huffman tables in %s\n",fn);
		goto cleanup;
	}

	#if 0
	fprintf(stderr,"[V] decoding\n");

	fprintf(stderr,"[V] first huffman table\n");
	CodeTable_print(stderr,QH->firstHuf->CT);
	fprintf(stderr,"[V] dif huffman table\n");
	CodeTable_print(stderr,QH->difHuf->CT);
	#endif

	/* read size of reverse quality table */
	if ( BitLevelDecoder_decodeGamma(IN->BLD,&(I->reverseQualityTableSize)) < 0 )
	{
		fprintf(stderr,"[E] unable to decode size of reverse quality table from %s\n",fn);
		goto cleanup;
	}

	/* allocate memory for reverse quality table */
	if ( ! (I->reverseQualityTable = (uint64_t *)malloc(sizeof(uint64_t)*I->reverseQualityTableSize)) )
	{
		fprintf(stderr,"[E] unable to allocate memory for reverse quality table from %s\n",fn);
		goto cleanup;
	}

	/* decode reverse quality table */
	for ( ii = 0; ii < I->reverseQualityTableSize; ++ii )
		if ( BitLevelDecoder_decodeGamma(IN->BLD,&(I->reverseQualityTable[ii])) < 0 )
		{
			fprintf(stderr,"[E] unable to decode reverse quality table from %s\n",fn);
			goto cleanup;
		}

	if ( ! (I->symCode = HuffmanCode_decode(IN->BLD)) )
	{
		fprintf(stderr,"[E] unable to decode sym huffman table from %s\n",fn);
		goto cleanup;
	}

	if ( ! (I->lengthsCode = HuffmanCode_decode(IN->BLD)) )
	{
		fprintf(stderr,"[E] unable to decode sym huffman table from %s\n",fn);
		goto cleanup;
	}

	fprintf(stderr,"[V] sym huffman table\n");
	CodeTable_print(stderr,I->symCode->CT);
	fprintf(stderr,"[V] lengths huffman table\n");
	CodeTable_print(stderr,I->lengthsCode->CT);

	if ( BitLevelDecoder_tell(IN->BLD,&(I->datapos)) < 0 )
	{
		fprintf(stderr,"[E] unable to get position after header in %s\n",fn);
		goto cleanup;
	}

	/* fprintf(stderr,"[V] offset %lu\n", (unsigned long)(I->datapos)); */

	IRPBINDecoder_Input_deallocate(IN);

	return I;

	cleanup:
	IRPBINDecoder_Input_deallocate(IN);
	IRPBINDecoder_deallocate(I);

	return NULL;
}

void IRPBINDecoder_addStep(IRPBINDecoder * I, ProvenanceStep ** insPS)
{
	uint64_t ii;

	assert ( I->PS );

	{
		ProvenanceStep * PP = I->PS;
		assert ( PP );

		while ( PP->next )
			PP = PP->next;

		PP->next = *insPS;
		*insPS = NULL;
	}

	for ( ii = 0; ii < I->HSLo; ++ii )
		if ( I->HSL[ii].type == '#' && I->HSL[ii].subtype == '!' )
			I->HSL[ii].num += 1;
}

int IRPBINDecoder_printHeader(IRPBINDecoder const * I, FILE * out)
{
	int returncode = 0;
	uint64_t ii = 0;

	if ( (returncode == 0) && (fprintf(out,"1 3 seq 1 0\n") < 0) )
		returncode = -1;
	if ( (returncode == 0) && (fprintf(out,"2 3 irp\n") < 0) )
		returncode = -1;

	for ( ii = 0; (returncode == 0) && (ii < I->HSLo); ++ii )
		if ( (returncode == 0) && fprintf(out,"%c %c %lu\n", I->HSL[ii].type, I->HSL[ii].subtype, (unsigned long)(I->HSL[ii].num)) < 0 )
			returncode = -1;

	if ( (returncode == 0) && (ProvenanceStep_print(out, I->PS) < 0) )
		returncode = -1;

	return returncode;
}

int IRPBINDecoder_decodePair(IRPBINDecoder * I, IRPBinDecoderContext * context)
{
	int64_t llv;
	if ( (llv = HuffmanCode_decodeSymbol(I->symCode, context->IN->BLD)) < 0 )
	{
		fprintf(stderr,"[E] unable to read marker\n");
		return -1;
	}

	switch ( llv )
	{
		case 'P':
		{
			break;
		}
		case 'g':
		{
			uint64_t groupsize;
			char * groupname = NULL;

			if ( BitLevelDecoder_decodeGamma(context->IN->BLD,&groupsize) < 0 )
			{
				fprintf(stderr,"[E] failed to read group size after g marker\n");
				return -1;
			}

			if ( ! (groupname = BitLevelDecoder_decodeString(context->IN->BLD)) )
			{
				fprintf(stderr,"[E] failed to read group name after g marker\n");
				return -1;
			}

			/* fprintf(stderr,"[V] found group %s of size %lu\n", groupname, (unsigned long)groupsize); */

			context->groupname = groupname;
			context->groupsize = groupsize;

			/* free(groupname); */

			return 1;
		}
		default:
		{
			fprintf(stderr,"[E] unknown marker %c\n", (char)llv);
			return -1;
			break;
		}
	}

	if ( IRPBINDecoder_decodeSequenceAndQuality(context->IN->BLD,I->QH,I->symCode,I->lengthsCode,I->reverseQualityTable,context->DF) < 0 )
	{
		fprintf(stderr,"[E] unable to read forward data\n");
		return -1;
	}

	if ( IRPBINDecoder_decodeSequenceAndQuality(context->IN->BLD,I->QH,I->symCode,I->lengthsCode,I->reverseQualityTable,context->DR) < 0 )
	{
		fprintf(stderr,"[E] unable to read reverse data\n");
		return -1;
	}

	return 0;
}

#if 0
int IRPBINDecoder_skipPair(IRPBINDecoder * I)
{
	int64_t llv;
	if ( (llv = HuffmanCode_decodeSymbol(I->symCode, I->BLD)) < 0 )
	{
		fprintf(stderr,"[E] unable to read marker\n");
		return -1;
	}

	switch ( llv )
	{
		case 'P':
		{
			break;
		}
		case 'g':
		{
			uint64_t groupsize;
			char * groupname = NULL;

			if ( BitLevelDecoder_decodeGamma(I->BLD,&groupsize) < 0 )
			{
				fprintf(stderr,"[E] failed to read group size after g marker\n");
				return -1;
			}

			if ( ! (groupname = BitLevelDecoder_decodeString(I->BLD)) )
			{
				fprintf(stderr,"[E] failed to read group name after g marker\n");
				return -1;
			}

			/* fprintf(stderr,"[V] found group %s of size %lu\n", groupname, (unsigned long)groupsize); */

			free(groupname);

			/* free(groupname); */

			return 1;
		}
		default:
		{
			fprintf(stderr,"[E] unknown marker %c\n", (char)llv);
			return -1;
			break;
		}
	}

	if ( IRPBINDecoder_skipSequenceAndQuality(I->BLD,I->QH,I->symCode,I->lengthsCode) < 0 )
	{
		fprintf(stderr,"[E] unable to read forward data\n");
		return -1;
	}

	if ( IRPBINDecoder_skipSequenceAndQuality(I->BLD,I->QH,I->symCode,I->lengthsCode) < 0 )
	{
		fprintf(stderr,"[E] unable to read reverse data\n");
		return -1;
	}

	return 0;
}
#endif

int IRPBINDecoder_seek(IRPBINDecoder * I, IRPBinDecoderContext * context, uint64_t i)
{
	uint64_t blockid;
	uint64_t blockmod;
	uint64_t filepos;

	if ( i > I->nr )
		i = I->nr;

	blockid = i / I->indexmod;
	blockmod = i - (blockid * I->indexmod);

	if ( BitLevelDecoder_seek(context->IN->BLD,I->indexpos + blockid * CHAR_BIT * sizeof(uint64_t)) < 0 )
		return -1;

	if ( BitLevelDecoder_decode(context->IN->BLD,&filepos,64) < 0 )
		return -1;

	/* fprintf(stderr,"file position is %lu\n", (unsigned long)filepos); */

	if ( BitLevelDecoder_seek(context->IN->BLD,filepos) < 0 )
		return -1;

	while ( blockmod )
	{
		int const r = IRPBINDecoder_decodePair(I,context);

		if ( r < 0 )
			return -1;
		else if ( r == 0 )
			blockmod -= 1;
	}

	return 0;
}

IRPBinDecoderContext * IRPBINDecoder_getContext(IRPBINDecoder * I)
{
	IRPBinDecoderContext * context = NULL;

	if (!(context = IRPBinDecoderContext_allocate(I->fn)))
		goto cleanup;

	if ( BitLevelDecoder_seek(context->IN->BLD,I->datapos) < 0 )
		goto cleanup;

	return context;

	cleanup:
	return IRPBinDecoderContext_deallocate(context);
}
