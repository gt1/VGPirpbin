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
#if ! defined(VGPIRPBIN_IRPBINDECODER_INPUT_H)
#define VGPIRPBIN_IRPBINDECODER_INPUT_H

#include "VGPirpbin_pre.h"
#include <stdio.h>
#include "VGPirpbin_BitLevelDecoder.h"

typedef struct _IRPBINDecoder_Input
{
	FILE * in;
	BitLevelDecoder * BLD;
} IRPBINDecoder_Input;

IRPBINDecoder_Input * IRPBINDecoder_Input_allocate(char const * fn);
IRPBINDecoder_Input * IRPBINDecoder_Input_deallocate(IRPBINDecoder_Input * I);
#endif
