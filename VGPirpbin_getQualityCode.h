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
#if ! defined(VGPIRPBIN_GETQUALITYCODE_H)
#define VGPIRPBIN_GETQUALITYCODE_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_QualityHuffman.h"
#include <stddef.h>

QualityHuffman * getQualityCode(FILE * in, Table const * qualTable, int64_t const maxlines);
QualityHuffman * getQualityCodeFromFile(char const * fn, Table const * qualityTable, int64_t const maxlines);
#endif
