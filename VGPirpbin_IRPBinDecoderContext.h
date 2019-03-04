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
#if ! defined(VGPIRPBIN_IRPBINDECODERCONTEXT_H)
#define VGPIRPBIN_IRPBINDECODERCONTEXT_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_DecodeResult.h"
#include <stdint.h>

typedef struct _IRPBinDecoderContext
{
        char * groupname;
        uint64_t groupsize;
        DecodeResult * DF;
        DecodeResult * DR;
} IRPBinDecoderContext;

IRPBinDecoderContext * IRPBinDecoderContext_allocate();
IRPBinDecoderContext * IRPBinDecoderContext_deallocate(IRPBinDecoderContext * I);
int IRPBinDecoderContext_printPair(IRPBinDecoderContext const * I, FILE * out);
#endif
