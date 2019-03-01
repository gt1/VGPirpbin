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
#if ! defined(VGPIRPBIN_GETQUALITYTABLE_H)
#define VGPIRPBIN_GETQUALITYTABLE_H

#include "VGPirpbin_pre.h"
#include "VGPirpbin_Table.h"

int getQualityTable(FILE * in, int64_t const maxlines, Table ** rqualTable, Table ** rsymTable, Table ** rlengthsTable);
int getQualityTableFromFile(char const * fn, int64_t const maxlines, Table ** rtable, Table ** rsymtable, Table ** rlengthsTable);
#endif
