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
#include "VGPirpbin_HeaderStatsLine.h"

#include <stdlib.h>
#include <string.h>

int HeaderStatsLine_encode(BitLevelEncoder * BLE, HeaderStatsLine const * HLE)
{
	if ( BitLevelEncoder_encode(BLE,HLE->type,8) < 0 )
		return -1;
	if ( BitLevelEncoder_encode(BLE,HLE->subtype,8) < 0 )
		return -1;
	if ( BitLevelEncoder_encodeGamma(BLE,HLE->num) < 0 )
		return -1;

	return 0;
}

int HeaderStatsLine_decode(BitLevelDecoder * BLD, HeaderStatsLine * HLE)
{
	uint64_t v;

	if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		return -1;

	HLE->type = v;

	if ( BitLevelDecoder_decode(BLD,&v,8) < 0 )
		return -1;

	HLE->subtype = v;

	if ( BitLevelDecoder_decodeGamma(BLD,&v) < 0 )
		return -1;

	HLE->num = v;

	return 0;
}

int HeaderStatsLine_push(HeaderStatsLine ** PSH, uint64_t * HSLo, uint64_t * HSLn, HeaderStatsLine NHSL)
{
	if ( *HSLo == *HSLn )
	{
		uint64_t const newsize = *HSLn ? 2*(*HSLn) : 1;

		HeaderStatsLine * NSH = (HeaderStatsLine *)malloc(sizeof(HeaderStatsLine)*newsize);
		if ( ! NSH )
			return -1;

		memcpy(
			NSH,
			*PSH,
			sizeof(HeaderStatsLine) * (*HSLn)
		);

		free(*PSH);
		*PSH = NSH;
		*HSLn = newsize;
	}

	(*PSH)[(*HSLo)++] = NHSL;

	return 0;
}
