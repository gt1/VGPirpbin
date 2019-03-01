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
#include "VGPirpbin_getNumber.h"

int64_t getNumber(char const ** a, char const ** e)
{
	int64_t n = 0;
	unsigned int ni = 0;

	while ( *a != *e && isdigit(**a) )
	{
		int64_t const dig = *a[0] - '0';
		(*a) += 1;

		n *= 10;
		n += dig;
		ni += 1;
	}

	if ( ! ni )
		return -1;
	else
		return n;
}
