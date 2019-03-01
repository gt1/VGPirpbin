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
#include "VGPirpbin_mstrdup.h"

char * mstrdup(char const * c)
{
	size_t const l = strlen(c);
	char * s = (char *)malloc((l+1)*sizeof(char));

	if ( ! s )
		return NULL;

	memcpy(s,c,l);
	s[l] = 0;

	return s;
}
