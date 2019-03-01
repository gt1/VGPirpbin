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
#if ! defined(VGPIRPBIN_BITLEVELDECODER_H)
#define VGPIRPBIN_BITLEVELDECODER_H

#include "VGPirpbin_pre.h"
#include <stdint.h>
#include <stdio.h>

typedef struct _BitLevelDecoder
{
	FILE * in;
	uint8_t c;
	unsigned int f;
} BitLevelDecoder;

BitLevelDecoder * BitLevelDecoder_allocate(FILE * in);
int BitLevelDecoder_load(BitLevelDecoder * BLV);
int BitLevelDecoder_getBit(BitLevelDecoder * BLV);
int BitLevelDecoder_decodeGamma(BitLevelDecoder * BLV, uint64_t * v);
void BitLevelDecoder_deallocate(BitLevelDecoder * BLV);
int BitLevelDecoder_decode(BitLevelDecoder * BLV, uint64_t * v, unsigned int l);
char * BitLevelDecoder_decodeString(BitLevelDecoder * BLD);
int BitLevelDecoder_seek(BitLevelDecoder * BLD, uint64_t const p);
#endif
