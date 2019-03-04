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
#include "VGPirpbin_IRPBinDecoderContext.h"
#include <stdlib.h>
#include <string.h>

IRPBinDecoderContext * IRPBinDecoderContext_allocate(char const * fn)
{
	IRPBinDecoderContext * I = NULL;

	I = (IRPBinDecoderContext*)malloc(sizeof(IRPBinDecoderContext));

	if ( ! I )
		return IRPBinDecoderContext_deallocate(I);

	memset(I,0,sizeof(IRPBinDecoderContext));

	if ( !(I->DF = DecodeResult_allocate()) )
		return IRPBinDecoderContext_deallocate(I);

	if ( !(I->DR = DecodeResult_allocate()) )
		return IRPBinDecoderContext_deallocate(I);

	if ( ! (I->IN = IRPBINDecoder_Input_allocate(fn)) )
		return IRPBinDecoderContext_deallocate(I);

	return I;
}

IRPBinDecoderContext * IRPBinDecoderContext_deallocate(IRPBinDecoderContext * I)
{
	if ( I )
	{
		IRPBINDecoder_Input_deallocate(I->IN);
		DecodeResult_deallocate(I->DF);
		DecodeResult_deallocate(I->DR);
		free(I->groupname);
		free(I);
	}

	return NULL;
}

int IRPBinDecoderContext_printPair(IRPBinDecoderContext const * I, FILE * out)
{
	if ( fprintf(out,"P\n") < 0 )
		return -1;

	if ( fprintf(out,"S %lu ",(unsigned long)I->DF->S_l) < 0 )
		return -1;
	if ( fwrite(I->DF->S,I->DF->S_l,1,out) != 1 )
		return -1;
	if ( fprintf(out,"\n") < 0 )
		return -1;

	if ( fprintf(out,"Q %lu ",(unsigned long)I->DF->Q_l) < 0 )
		return -1;
	if ( fwrite(I->DF->Q,I->DF->Q_l,1,out) != 1 )
		return -1;
	if ( fprintf(out,"\n") < 0 )
		return -1;

	if ( fprintf(out,"S %lu ",(unsigned long)I->DR->S_l) < 0 )
		return -1;
	if ( fwrite(I->DR->S,I->DR->S_l,1,out) != 1 )
		return -1;
	if ( fprintf(out,"\n") < 0 )
		return -1;

	if ( fprintf(out,"Q %lu ",(unsigned long)I->DR->Q_l) < 0 )
		return -1;
	if ( fwrite(I->DR->Q,I->DR->Q_l,1,out) != 1 )
		return -1;
	if ( fprintf(out,"\n") < 0 )
		return -1;

	return 0;
}
