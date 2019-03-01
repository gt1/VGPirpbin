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
#if ! defined(VGPIRPBIN_HEADERSTATSLINE_H)
#define VGPIRPBIN_HEADERSTATSLINE_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_BitLevelDecoder.h"
#include "VGPirpbin_BitLevelEncoder.h"
#include <stdint.h>

typedef struct _HeaderStatsLine
{
	char type;
	char subtype;
	uint64_t num;
} HeaderStatsLine;

int HeaderStatsLine_encode(BitLevelEncoder * BLE, HeaderStatsLine const * HLE);
int HeaderStatsLine_decode(BitLevelDecoder * BLD, HeaderStatsLine * HLE);
int HeaderStatsLine_push(HeaderStatsLine ** PSH, uint64_t * HSLo, uint64_t * HSLn, HeaderStatsLine NHSL);
#endif
