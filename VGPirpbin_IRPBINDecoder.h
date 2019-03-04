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
#include "VGPirpbin_IRPBinDecoderContext.h"

typedef struct _IRPBINDecoder
{
	char * fn;
	QualityHuffman * QH;
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
	uint64_t datapos;
} IRPBINDecoder;

/**
 * allocate decoder for binary IRP files
 **/
IRPBINDecoder * IRPBINDecoder_allocateFromFile(char const * fn, char const * binfiletype);
/**
 * print header of binary IRP file as text
 *
 * return codes:
 *  -1: failure
 *   0: success
 *
 **/
int IRPBINDecoder_printHeader(IRPBINDecoder const * I, FILE * out);
/**
 * decode a pair from the current position and store the information in context
 *
 * return codes:
 *  * -1: an error occurred
 *  *  0: end of file reached
 *  *  1: succesfully decoded a read pair
 *  *  2: found a read group record and stored it in context
 *
 **/
int IRPBINDecoder_decodePair(IRPBINDecoder * I, IRPBinDecoderContext * context);
/**
 * set input pointer to position i
 *
 * return codes:
 *  * -1: seek failed
 *  *  0: seek successful
 *
 **/
int IRPBINDecoder_seek(IRPBINDecoder * I, IRPBinDecoderContext * context, uint64_t i);
/**
 * get a context for decoding records and set the input pointer for that context to record 0
 * After use the context should be freed using IRPBinDecoderContext_deallocate
 *
 * return values:
 *  * NULL: failed to construct context
 *  * any other value: pointer to decode context
 **/
IRPBinDecoderContext * IRPBINDecoder_getContext(IRPBINDecoder * I);

/**
 * deallocate a decoder object
 *
 * return values:
 *   NULL: always
 **/
IRPBINDecoder * IRPBINDecoder_deallocate(IRPBINDecoder * I);

/**
 * add provenance step line to the header
 **/
void IRPBINDecoder_addStep(IRPBINDecoder * I, ProvenanceStep ** insPS);
#endif
