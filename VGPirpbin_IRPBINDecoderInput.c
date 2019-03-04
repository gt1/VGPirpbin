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
#include "VGPirpbin_IRPBINDecoderInput.h"
#include <stdlib.h>
#include <string.h>

IRPBINDecoder_Input * IRPBINDecoder_Input_allocate(char const * fn)
{
	IRPBINDecoder_Input * I = NULL;

	if ( ! (I = (IRPBINDecoder_Input *)malloc(sizeof(IRPBINDecoder_Input))) )
		return IRPBINDecoder_Input_deallocate(I);

	memset(I,0,sizeof(IRPBINDecoder_Input));

	if ( ! (I->in = fopen(fn,"rb")) )
		return IRPBINDecoder_Input_deallocate(I);

	if ( ! (I->BLD = BitLevelDecoder_allocate(I->in)) )
		return IRPBINDecoder_Input_deallocate(I);

	return I;
}

IRPBINDecoder_Input * IRPBINDecoder_Input_deallocate(IRPBINDecoder_Input * I)
{
	if ( I )
	{
		BitLevelDecoder_deallocate(I->BLD);
		if ( I->in )
			fclose(I->in);
		free(I);
	}

	return NULL;
}
