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
#include "VGPirpbin_pre.h"
#include "VGPirpbin_CString.h"
#include "VGPirpbin_expect.h"
#include "VGPirpbin_getNumber.h"
#include <stdlib.h>
#include <string.h>

CString CString_getString(char const ** a, char const ** e)
{
	int64_t const n = getNumber(a,e);
	CString C;
	C.a = NULL;
	C.e = NULL;

	if ( n < 0 )
		return C;

	if ( ! expect(a,e,' ') )
		return C;

	if ( *e - *a < n )
		return C;

	C.a = *a;
	C.e = *a + n;

	*a += n;

	return C;
}

char * CString_tostring(CString const * a)
{
	size_t const l = a->e - a->a;
	char * c = (char *)malloc(sizeof(char)*(l+1));
	if ( ! c )
		return NULL;
	c[l] = 0;
	memcpy(c,a->a,l);
	return c;
}
