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
#if ! defined(VGPIRPBIN_CODETABLEENTRY_H)
#define VGPIRPBIN_CODETABLEENTRY_H

#include "VGPirpbin_pre.h"
#include <stdint.h>
#include <stdio.h>

typedef struct _CodeTableEntry
{
	uint64_t symbol;
	uint64_t codelength;
	uint64_t code;
} CodeTableEntry;

void CodeTableEntry_print(FILE * out, CodeTableEntry const * C);
int CodeTableEntry_symbolCompare(void const * A, void const * B);
#endif
