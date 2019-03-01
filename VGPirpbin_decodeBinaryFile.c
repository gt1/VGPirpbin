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
#include "VGPirpbin_decodeBinaryFile.h"

int VGP_IRPBIN_decodeBinaryFile(char const * fn, ProvenanceStep ** insPS, char const * binfiletype)
{
	int returncode = 0;
	uint64_t iii;

	IRPBINDecoder * I = NULL;

	if ( ! (I = IRPBINDecoder_allocateFromFile(fn,binfiletype)) )
	{
		returncode = -1;
		goto cleanup;
	}

	IRPBINDecoder_addStep(I,insPS);

	if ( IRPBINDecoder_printHeader(I,stdout) < 0 )
	{
		returncode = -1;
		goto cleanup;
	}

	for ( iii = 0; iii < I->nr; ++iii )
	{
		if ( IRPBINDecoder_decodePair(I) < 0 )
		{
			fprintf(stderr,"[E] failed to decode pair\n");
			goto cleanup;
		}

		if ( IRPBINDecoder_printPair(I, stdout) < 0 )
		{
			fprintf(stderr,"[E] failed to print pair\n");
			goto cleanup;
		}
	}

	cleanup:
	IRPBINDecoder_deallocate(I);

	return returncode;
}
