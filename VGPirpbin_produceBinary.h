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
#if ! defined(VGPIRPBIN_IRPBIN_PRODUCEBINARY_H)
#define VGPIRPBIN_IRPBIN_PRODUCEBINARY_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_QualityHuffman.h"
#include "VGPirpbin_ProvenanceStep.h"

int VGP_IRPBIN_produceBinary(FILE * out, FILE * in, QualityHuffman const * QH, Table * qualTable, char const * tmpfn, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode, char const * binfiletype);
int VGP_IRPBIN_produceBinaryFromFile(char const * outfn, char const * fn, QualityHuffman const * QH, Table * qualTable, ProvenanceStep ** insPS, HuffmanCode * symCode, HuffmanCode * lengthsCode, char const * binfiletype);
int VGP_IRPBIN_produceBinaryFile(char const * outfn, char const * infn, int64_t const maxstatlines, ProvenanceStep ** insPS, char const * binfiletype);
#endif
