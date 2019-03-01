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
#if ! defined(VGPIRPBIN_BITLEVELENCODER_H)
#define VGPIRPBIN_BITLEVELENCODER_H

#include "VGPirpbin_pre.h"
#include <stdint.h>

typedef struct _BitLevelEncoder
{
	FILE * out;
	uint8_t c;
	unsigned int f;
	uint64_t w;
} BitLevelEncoder;

uint64_t BitLevelEncoder_getOffset(BitLevelEncoder const * A);
BitLevelEncoder * BitLevelEncoder_allocate(FILE * out);
int BitLevelEncoder_write(BitLevelEncoder * enc);
int BitLevelEncoder_flush(BitLevelEncoder * enc);
int BitLevelEncoder_encode(BitLevelEncoder * enc, uint64_t codeword, unsigned int codelength);
int BitLevelEncoder_encodeGamma(BitLevelEncoder * enc, uint64_t v);
int BitLevelEncoder_deallocate(BitLevelEncoder * enc);
int BitLevelEncoder_encodeString(BitLevelEncoder * BLE, char const * c);
#endif