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
#if ! defined(VGPIRPBIN_HUFFMANCODE_H)
#define VGPIRPBIN_HUFFMANCODE_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_PairTable.h"
#include "VGPirpbin_CodeTable.h"
#include "VGPirpbin_HuffmanDecodeQueueEntry.h"

typedef struct _HuffmanCode
{
	PairTable * PT;
	CodeTable * CT;
	CodeTable * CTsorted;
	CodeTable * CTsortedSparse;
	HuffmanDecodeQueueEntry * DQ;
} HuffmanCode;

int HuffmanCode_encodeSymbol(BitLevelEncoder * BLE, HuffmanCode * HC, uint64_t const sym);
int HuffmanCode_encode(BitLevelEncoder * BLE, HuffmanCode const * HC);
HuffmanCode * HuffmanCode_decode(BitLevelDecoder * BLD);
int64_t HuffmanCode_decodeSymbol(HuffmanCode const * H, BitLevelDecoder * BLD);
HuffmanCode * HuffmanCode_computeFromLengths(PairTable ** firstPT);

HuffmanCode * HuffmanCode_allocate(PairTable * PT, CodeTable * CT, CodeTable * CTsorted, CodeTable * CTsortedSparse);
void HuffmanCode_deallocate(HuffmanCode * HC);
int HuffmanCode_computeLengths(PairTable * PT);
#endif
