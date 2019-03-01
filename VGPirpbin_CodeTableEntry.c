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
#include "VGPirpbin_CodeTableEntry.h"

int CodeTableEntry_symbolCompare(void const * A, void const * B)
{
	CodeTableEntry * CA = (CodeTableEntry *)A;
	CodeTableEntry * CB = (CodeTableEntry *)B;

	if ( CA->symbol < CB->symbol )
		return -1;
	else if ( CB->symbol < CA->symbol )
		return 1;
	else
		return 0;
}
void CodeTableEntry_print(FILE * out, CodeTableEntry const * C)
{
	size_t i;

	if ( C->codelength != UINT64_MAX )
	{
		fprintf(out,"sym %05d len %05d ", (int)C->symbol, (int)C->codelength);

		for ( i = 0; i < C->codelength; ++i )
			fprintf(out,"%d", (int)(C->code >> (C->codelength-i-1))&1);
		fprintf(out,"\n");
	}
}
