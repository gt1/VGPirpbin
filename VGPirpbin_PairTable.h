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
#if ! defined(VGPIRPBIN_PAIRTABLE_H)
#define VGPIRPBIN_PAIRTABLE_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_Table.h"
#include "VGPirpbin_Pair.h"
#include "VGPirpbin_BitLevelEncoder.h"
#include "VGPirpbin_BitLevelDecoder.h"

typedef struct _PairTable
{
	Pair * A;
	uint64_t n;
} PairTable;

PairTable * PairTable_allocate(uint64_t const n);
void PairTable_deallocate(PairTable * PT);
PairTable * PairTable_getPairTable(Table const * T);
int PairTable_encode(PairTable const * P, BitLevelEncoder * BLE);
int PairTable_encodeHuffmanDif(PairTable const * P, BitLevelEncoder * BLE);
PairTable * PairTable_decode(BitLevelDecoder * BLD);
PairTable * PairTable_decodeHuffmanDif(BitLevelDecoder * BLD);
#endif
