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
#if ! defined(VGPIRPBIN_IRPBINDECODER_H)
#define VGPIRPBIN_IRPBINDECODER_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_QualityHuffman.h"
#include "VGPirpbin_DecodeResult.h"
#include "VGPirpbin_HeaderStatsLine.h"
#include "VGPirpbin_ProvenanceStep.h"

typedef struct _IRPBINDecoder
{
	FILE * in;
	QualityHuffman * QH;
	BitLevelDecoder * BLD;
	DecodeResult * DF;
	DecodeResult * DR;
	uint64_t reverseQualityTableSize;
	uint64_t * reverseQualityTable;
	uint64_t HSLo;
	HeaderStatsLine * HSL;
	ProvenanceStep * PS;
	HuffmanCode * symCode;
	HuffmanCode * lengthsCode;
	uint64_t nr;
	uint64_t indexmod;
	uint64_t indexpos;
	char * groupname;
	uint64_t groupsize;
} IRPBINDecoder;

int IRPBINDecoder_decodeSequenceAndQuality(
	BitLevelDecoder * BLD,
	QualityHuffman * QH,
	HuffmanCode * symCode,
	HuffmanCode * lengthsCode,
	uint64_t * reverseQualityTable,
	DecodeResult * D
);
IRPBINDecoder * IRPBINDecoder_deallocate(IRPBINDecoder * I);
IRPBINDecoder * IRPBINDecoder_allocate();
IRPBINDecoder * IRPBINDecoder_allocateFromFile(char const * fn, char const * binfiletype);
void IRPBINDecoder_addStep(IRPBINDecoder * I, ProvenanceStep ** insPS);
int IRPBINDecoder_printHeader(IRPBINDecoder const * I, FILE * out);
int IRPBINDecoder_decodePair(IRPBINDecoder * I);
int IRPBINDecoder_printPair(IRPBINDecoder const * I, FILE * out);
#endif
