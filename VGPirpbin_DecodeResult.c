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
#include "VGPirpbin_DecodeResult.h"
#include <stdlib.h>
#include <string.h>

DecodeResult * DecodeResult_allocate()
{
	DecodeResult * D = NULL;

	D = (DecodeResult *)malloc(sizeof(DecodeResult));

	if ( ! D )
		return NULL;

	memset(D,0,sizeof(DecodeResult));

	return D;
}

void DecodeResult_deallocate(DecodeResult * D)
{
	if ( D )
	{
		free(D->S);
		free(D->Q);
		free(D);
	}
}
